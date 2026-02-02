#include "Game/DeliveryDetonation_GameMode.h"
#include "Game/PackageSpawnPoint.h"
#include "Pickups/BasePackage.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Game/DeliveryDetonation_GameState.h"
#include "Game/GamePlayerController.h"
#include "Widgets/HUDWidget.h" 
#include "ActorComponents/HealthComponent.h"
#include "Player/PlayerCharacter.h"
#include "WaypointSystem/WaypointComponent.h"
#include "Player/DeliveryDetonation_PlayerState.h"

ADeliveryDetonation_GameMode::ADeliveryDetonation_GameMode()
{
    PrimaryActorTick.bCanEverTick = true;
    PlayerStateClass = ADeliveryDetonation_PlayerState::StaticClass();
}

void ADeliveryDetonation_GameMode::BeginPlay()
{
    Super::BeginPlay();

    DDGameState = GetGameState<ADeliveryDetonation_GameState>();
    TimeRemainingSeconds = FMath::Max(0.0f, TimeLimitSeconds);

    if (DDGameState)
    {
        DDGameState->TotalCargo = TargetDeliveryGoal;
        DDGameState->DeliveredCargo = 0;
        DDGameState->TimeRemaining = TimeRemainingSeconds;
        DDGameState->MatchPhase = EMatchPhase::Playing;
        DDGameState->RestartCountdown = 0;
        DDGameState->OnCargoUpdated.Broadcast(0, TargetDeliveryGoal);
    }
}

void ADeliveryDetonation_GameMode::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bGameOver) return;

    if (TimeRemainingSeconds > 0.0f)
    {
        TimeRemainingSeconds = FMath::Max(0.0f, TimeRemainingSeconds - DeltaSeconds);

        if (DDGameState)
        {
            DDGameState->TimeRemaining = TimeRemainingSeconds;
            DDGameState->OnTimeUpdated.Broadcast(TimeRemainingSeconds);
        }

        if (TimeRemainingSeconds <= 0.0f)
        {
            TryLoseByTime();
        }
    }
}

void ADeliveryDetonation_GameMode::Logout(AController* Exiting)
{
    Super::Logout(Exiting);

    // If we're in post match and someone leaves, re-check readiness for remaining players
    if (bGameOver)
    {
        TryStartRestartCountdown();
    }
}

void ADeliveryDetonation_GameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    if (!NewPlayer) return;

    PlayerScores.Add(NewPlayer, 0);

    // Always init PS values
    if (ADeliveryDetonation_PlayerState* DDPS = Cast<ADeliveryDetonation_PlayerState>(NewPlayer->PlayerState))
    {
        DDPS->SetWinner(false);
        DDPS->SetReady(false);
    }

    // Hook health (only if pawn exists right now)
    if (APawn* Pawn = NewPlayer->GetPawn())
    {
        if (UHealthComponent* HealthComp = Pawn->FindComponentByClass<UHealthComponent>())
        {
            HealthComp->OnHealthChanged.RemoveDynamic(this, &ADeliveryDetonation_GameMode::HandlePlayerHealthChanged);
            HealthComp->OnHealthChanged.AddDynamic(this, &ADeliveryDetonation_GameMode::HandlePlayerHealthChanged);
        }
    }

    // late join during PostMatch
    const bool bIsPostMatch =
        bGameOver || (DDGameState && DDGameState->MatchPhase == EMatchPhase::PostMatch);

    if (bIsPostMatch)
    {
        // Do NOT spawn packages, do NOT allow roaming.
        ForcePostMatchForPlayer(NewPlayer);

        // Also re-check countdown conditions because Active player count changed
        TryStartRestartCountdown();
        return;
    }

    // Normal match flow
    SpawnPackageForPlayer(NewPlayer);
    UE_LOG(LogTemp, Log, TEXT("[GM] PostLogin: Package loop for %s"), *NewPlayer->GetName());
}

// --- NEW IMPLEMENTATION ---
void ADeliveryDetonation_GameMode::RegisterHUD(UHUDWidget* NewHUD)
{
    HUDWidgetInstance = NewHUD;
}
// --------------------------

void ADeliveryDetonation_GameMode::SpawnPackageForPlayer(AController* Player)
{
    if (!Player) return;
    if (bGameOver) return;

    APackageSpawnPoint* ChosenSpot = SelectWeightedSpawnPoint(Player);

    if (!ChosenSpot)
    {
        FTimerHandle RetryHandle;
        FTimerDelegate RetryDel;
        RetryDel.BindUObject(this, &ADeliveryDetonation_GameMode::SpawnNextForPlayer, Player);
        GetWorldTimerManager().SetTimer(RetryHandle, RetryDel, 1.0f, false);
        return;
    }

    TSubclassOf<ABasePackage> ChosenClass = SelectWeightedPackageType();
    if (!ChosenClass) return;

    FActorSpawnParameters Params;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    ABasePackage* NewPkg = GetWorld()->SpawnActor<ABasePackage>(
        ChosenClass,
        ChosenSpot->GetActorTransform(),
        Params
    );

    if (NewPkg)
    {
        NewPkg->SetOwnerSpawnPoint(ChosenSpot);
        RegisterCargo(NewPkg);
        UE_LOG(LogTemp, Log, TEXT("[GM] Spawned for %s at %s"), *Player->GetName(), *ChosenSpot->GetName());
        if (Player)
        {
            if (APlayerCharacter* PC = Cast<APlayerCharacter>(Player->GetPawn()))
            {
                if (PC->WaypointSystem)
                {
                    PC->WaypointSystem->SetObjectiveLocation(NewPkg->GetActorLocation());
                }
            }
        }
    }
}

void ADeliveryDetonation_GameMode::SpawnNextForPlayer(AController* Player)
{
    SpawnPackageForPlayer(Player);
}

void ADeliveryDetonation_GameMode::NotifyCargoDelivered(AActor* CargoActor, AController* Deliverer)
{
    if (!Deliverer || bGameOver) return;

    int32& ScoreRef = PlayerScores.FindOrAdd(Deliverer);
    ScoreRef++;

    const int32 CurrentScore = ScoreRef;

    if (DDGameState)
    {
        DDGameState->DeliveredCargo++;
        DDGameState->OnCargoUpdated.Broadcast(CurrentScore, TargetDeliveryGoal);
    }

    if (CurrentScore >= TargetDeliveryGoal)
    {
        TryWin(Deliverer);
    }
    else
    {
        FTimerDelegate SpawnDel;
        SpawnDel.BindUObject(this, &ThisClass::SpawnNextForPlayer, Deliverer);
        FTimerHandle& Handle = PlayerRespawnTimers.FindOrAdd(Deliverer);
        GetWorldTimerManager().SetTimer(Handle, SpawnDel, RespawnDelay, false);
    }

    if (CargoActor) RegisteredCargo.Remove(CargoActor);
}

void ADeliveryDetonation_GameMode::NotifyCargoFailed(AActor* CargoActor)
{
    if (bGameOver) return;

    for (auto& Elem : PlayerScores)
    {
        AController* Player = Elem.Key.Get();
        if (Player)
        {
            FTimerHandle RetryHandle;
            FTimerDelegate RetryDel;
            RetryDel.BindUObject(this, &ADeliveryDetonation_GameMode::SpawnNextForPlayer, Player);
            GetWorldTimerManager().SetTimer(RetryHandle, RetryDel, 2.0f, false);
        }
    }
}

void ADeliveryDetonation_GameMode::TryWin(AController* Winner)
{
    if (bGameOver) return;
    EnterPostMatch(Winner);
}

void ADeliveryDetonation_GameMode::TryLoseByTime()
{
    if (bGameOver) return;

    // No winner (time out). Everyone loses.
    EnterPostMatch(nullptr);
}

void ADeliveryDetonation_GameMode::EnterPostMatch(AController* Winner)
{
    if (bGameOver) return;
    bGameOver = true;

    APlayerState* WinnerPS = Winner ? Winner->PlayerState : nullptr;

    // Global match state
    if (DDGameState)
    {
        DDGameState->MatchPhase = EMatchPhase::PostMatch;
        DDGameState->RestartCountdown = 0;
    }

    // Mark per-player state on PlayerState (replicated)
    for (APlayerState* PSBase : GameState->PlayerArray)
    {
        if (ADeliveryDetonation_PlayerState* PS = Cast<ADeliveryDetonation_PlayerState>(PSBase))
        {
            const bool bIsWinner = (WinnerPS && WinnerPS == PSBase);
            PS->SetWinner(bIsWinner);
            PS->SetReady(false);
        }
    }

    // Freeze everyone (server authoritative)
    for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
    {
        if (APlayerController* PC = It->Get())
        {
            if (APawn* Pawn = PC->GetPawn())
            {
                Pawn->DisableInput(PC);

                if (ACharacter* Char = Cast<ACharacter>(Pawn))
                {
                    if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
                    {
                        Move->StopMovementImmediately();
                        Move->DisableMovement();
                    }
                }
            }

            if (AGamePlayerController* DDPC = Cast<AGamePlayerController>(PC))
            {
                DDPC->Client_EnterPostMatchUI();
            }
        }
    }

    CancelRestartCountdown();
}

void ADeliveryDetonation_GameMode::Server_SetPlayerReady(APlayerController* PC, bool bReady)
{
    if (!bGameOver || !PC) return;

    if (ADeliveryDetonation_PlayerState* PS = PC->GetPlayerState<ADeliveryDetonation_PlayerState>())
    {
        PS->SetReady(bReady);
    }

    TryStartRestartCountdown();
}

bool ADeliveryDetonation_GameMode::AreAllRemainingPlayersReady(int32& OutActive, int32& OutReady) const
{
    OutActive = 0;
    OutReady = 0;

    if (!GameState) return false;

    for (APlayerState* PSBase : GameState->PlayerArray)
    {
        if (ADeliveryDetonation_PlayerState* PS = Cast<ADeliveryDetonation_PlayerState>(PSBase))
        {
            OutActive++;
            if (PS->IsReady())
            {
                OutReady++;
            }
        }
    }

    return (OutActive > 0 && OutActive == OutReady);
}

void ADeliveryDetonation_GameMode::Server_PlayerQuitMatch(APlayerController* PC)
{
    if (!PC) return;

    // Kick/close this connection; client should also call LeaveSession/DestroySession locally.
    // This ensures Logout() happens and remaining players can continue.
    PC->ClientTravel(MenuLevelName.ToString(), ETravelType::TRAVEL_Absolute);
    PC->Destroy();
}

void ADeliveryDetonation_GameMode::TryStartRestartCountdown()
{
    if (!bGameOver || !DDGameState) return;

    int32 Active = 0, Ready = 0;
    const bool bAllReady = AreAllRemainingPlayersReady(Active, Ready);

    if (!bAllReady)
    {
        CancelRestartCountdown();
        return;
    }

    if (!GetWorldTimerManager().IsTimerActive(RestartCountdownHandle))
    {
        DDGameState->RestartCountdown = RestartCountdownSeconds;
        GetWorldTimerManager().SetTimer(RestartCountdownHandle, this, &ThisClass::RestartCountdownTick, 1.0f, true);
    }
}

void ADeliveryDetonation_GameMode::CancelRestartCountdown()
{
    if (GetWorldTimerManager().IsTimerActive(RestartCountdownHandle))
    {
        GetWorldTimerManager().ClearTimer(RestartCountdownHandle);
    }
    if (DDGameState)
    {
        DDGameState->RestartCountdown = 0;
    }
}

void ADeliveryDetonation_GameMode::RestartCountdownTick()
{
    if (!DDGameState) return;

    DDGameState->RestartCountdown = FMath::Max(0, DDGameState->RestartCountdown - 1);

    if (DDGameState->RestartCountdown <= 0)
    {
        GetWorldTimerManager().ClearTimer(RestartCountdownHandle);

        // Restart match for remaining players (listen server)
        GetWorld()->ServerTravel(TEXT("?Restart"));
    }
}

void ADeliveryDetonation_GameMode::OnWin()
{
    if (bGameOver) return;
    UE_LOG(LogTemp, Warning, TEXT("[GM] WIN!"));
    ShowResults(true);
}

void ADeliveryDetonation_GameMode::OnLose()
{
    if (bGameOver) return;
    UE_LOG(LogTemp, Warning, TEXT("[GM] LOSE!"));
    ShowResults(false);
}

void ADeliveryDetonation_GameMode::ShowResults(bool bWon)
{
    if (bGameOver) return;
    bGameOver = true;

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (!PC) return;

    if (APawn* Pawn = PC->GetPawn())
    {
        Pawn->DisableInput(PC);
        if (ACharacter* Char = Cast<ACharacter>(Pawn))
        {
            if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
            {
                Move->StopMovementImmediately();
                Move->DisableMovement();
            }
        }
    }

    if (!ResultsWidgetClass) return;

    if (!ResultsWidgetInstance)
    {
        ResultsWidgetInstance = CreateWidget<UUserWidget>(PC, ResultsWidgetClass);
    }

    if (ResultsWidgetInstance)
    {
        ResultsWidgetInstance->AddToViewport(100);
    }

    if (bWon)
    {
        PC->SetPause(false);
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
        GetWorldTimerManager().SetTimer(ReturnToMenuHandle, this, &ADeliveryDetonation_GameMode::ReturnToMenu, 5.0f, false);
    }
    else
    {
        PC->SetPause(true);
        FInputModeUIOnly Mode;
        if (ResultsWidgetInstance) Mode.SetWidgetToFocus(ResultsWidgetInstance->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        PC->SetInputMode(Mode);
        PC->bShowMouseCursor = true;
    }
}

void ADeliveryDetonation_GameMode::ReturnToMenu()
{
    if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
    {
        PC->SetPause(false);
        PC->bShowMouseCursor = false;
        PC->SetInputMode(FInputModeGameOnly());
    }
    UGameplayStatics::OpenLevel(this, MenuLevelName);
}

void ADeliveryDetonation_GameMode::HandlePlayerHealthChanged(float Current, float Max)
{
    if (HUDWidgetInstance)
    {
        HUDWidgetInstance->SetHealth(Current, Max);
    }
}

void ADeliveryDetonation_GameMode::RegisterCargo(AActor* CargoActor)
{
    if (CargoActor) RegisteredCargo.AddUnique(CargoActor);
}

APackageSpawnPoint* ADeliveryDetonation_GameMode::SelectWeightedSpawnPoint(AController* TargetPlayer)
{
    FVector PlayerLoc = FVector::ZeroVector;
    if (TargetPlayer && TargetPlayer->GetPawn())
    {
        PlayerLoc = TargetPlayer->GetPawn()->GetActorLocation();
    }

    TArray<AActor*> FoundPoints;
    UGameplayStatics::GetAllActorsOfClass(this, APackageSpawnPoint::StaticClass(), FoundPoints);

    TMap<APackageSpawnPoint*, float> PointWeights;
    float TotalWeight = 0.0f;

    for (AActor* Actor : FoundPoints)
    {
        APackageSpawnPoint* Pt = Cast<APackageSpawnPoint>(Actor);
        if (!Pt) continue;

        if (Pt->IsOccupied()) continue;

        float Dist = FVector::Dist(Pt->GetActorLocation(), PlayerLoc);
        float Weight = 0.0f;

        // Debug log to see real distances(Uncomment if needed)
        UE_LOG(LogTemp, Log, TEXT("Point: %s | Dist: %f"), *Pt->GetName(), Dist);

            if (Dist < MinDistance)
            {
                // Too close (Penalty)
                Weight = 0.1f;
            }
            else if (Dist >= MinDistance && Dist <= MaxPriorityDistance)
            {
                // Sweet Spot (Massive Bonus)
                Weight = 100.0f; // Was 3.0f - Boosted to guarantee selection if available
            }
            else
            {
                // Falloff Zone
                float OverDistance = Dist - MaxPriorityDistance;

                // HARD CUTOFF: If it's further than FalloffDistance, ignore it completely.
                if (OverDistance > FalloffDistance)
                {
                    Weight = 0.0f;
                }
                else
                {
                    // Linear drop from 100% to 0%
                    float DecayPct = 1.0f - (OverDistance / FalloffDistance);
                    Weight = FMath::Lerp(0.0f, 10.0f, DecayPct); // Scaled relative to the Sweet Spot
                }
            }

        // Only add if it has a valid chance
        if (Weight > 0.0f)
        {
            PointWeights.Add(Pt, Weight);
            TotalWeight += Weight;
        }
    }

    if (TotalWeight <= 0.f) return nullptr;

    float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentSum = 0.0f;

    for (auto& Elem : PointWeights)
    {
        CurrentSum += Elem.Value;
        if (RandomValue <= CurrentSum) return Elem.Key;
    }

    return nullptr;
}

TSubclassOf<ABasePackage> ADeliveryDetonation_GameMode::SelectWeightedPackageType()
{
    if (PackageTypes.Num() == 0) return nullptr;

    float TotalWeight = 0.0f;
    for (const FPackageTypeEntry& Entry : PackageTypes) TotalWeight += Entry.SpawnWeight;

    float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentSum = 0.0f;

    for (const FPackageTypeEntry& Entry : PackageTypes)
    {
        CurrentSum += Entry.SpawnWeight;
        if (RandomValue <= CurrentSum) return Entry.PackageClass;
    }

    return PackageTypes[0].PackageClass;
}

void ADeliveryDetonation_GameMode::AssignDeliveryObjective(AController* Player)
{
    if (!Player) return;

    // --- FIX: CHECK FOR EXISTING OBJECTIVE FIRST ---
    if (PlayerObjectives.Contains(Player))
    {
        // We found an existing assignment!
        TWeakObjectPtr<ADeliveryZone> ExistingZone = PlayerObjectives[Player];

        if (ExistingZone.IsValid())
        {
            // Just refresh the beam (in case UI flickered), but DO NOT change the location.
            if (APlayerCharacter* PC = Cast<APlayerCharacter>(Player->GetPawn()))
            {
                if (PC->WaypointSystem)
                {
                    PC->WaypointSystem->SetObjectiveLocation(ExistingZone->GetActorLocation());
                }
            }

            UE_LOG(LogTemp, Log, TEXT("[GM] Keeping existing objective for %s"), *Player->GetName());
            return; // EXIT EARLY: Don't pick a new random zone!
        }
    }
    // -----------------------------------------------

    // If we get here, it means we had no active objective, so pick a NEW one.
    ADeliveryZone* ChosenZone = SelectWeightedDeliveryZone(Player);

    if (ChosenZone)
    {
        // 1. Store Assignment
        PlayerObjectives.Add(Player, ChosenZone);

        // 2. Update Player's Waypoint (The Beam)
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(Player->GetPawn()))
        {
            if (PC->WaypointSystem)
            {
                PC->WaypointSystem->SetObjectiveLocation(ChosenZone->GetActorLocation());
            }
        }

        UE_LOG(LogTemp, Log, TEXT("[GM] Assigned Delivery Zone %s to %s"), *ChosenZone->GetName(), *Player->GetName());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[GM] No Delivery Zones found! Cannot assign objective."));
    }
}

void ADeliveryDetonation_GameMode::TryCompleteDelivery(APlayerCharacter* PlayerChar, ADeliveryZone* Zone)
{
    if (!PlayerChar || !Zone) return;

    AController* PC = PlayerChar->GetController();

    // 1. Verify this is the CORRECT zone for this player
    if (PlayerObjectives.Contains(PC))
    {
        TWeakObjectPtr<ADeliveryZone> AssignedZone = PlayerObjectives[PC];

        if (AssignedZone.IsValid() && AssignedZone.Get() == Zone)
        {
            // --- SUCCESS ---

            // 2. Clear Objective
            if (PlayerChar->WaypointSystem)
            {
                PlayerChar->WaypointSystem->ClearObjective();
            }
            PlayerObjectives.Remove(PC);

            // 3. Handle the Package
            if (ABasePackage* Pkg = PlayerChar->GetCarriedPackage())
            {
                // This will trigger NotifyCargoDelivered -> Spawns Next Package
                Pkg->Deliver();
            }

            // 4. Clear Player Hands (Remove constraint/mesh)
            PlayerChar->SuccessDelivery();
        }
        else
        {
            // Optional: UI feedback "Wrong Dropoff Location!"
            UE_LOG(LogTemp, Warning, TEXT("Wrong Zone! Go to %s"), *AssignedZone->GetName());
        }
    }
}

ADeliveryZone* ADeliveryDetonation_GameMode::SelectWeightedDeliveryZone(AController* TargetPlayer)
{
    FVector PlayerLoc = FVector::ZeroVector;
    if (TargetPlayer && TargetPlayer->GetPawn())
    {
        PlayerLoc = TargetPlayer->GetPawn()->GetActorLocation();
    }

    TArray<AActor*> FoundZones;
    UGameplayStatics::GetAllActorsOfClass(this, ADeliveryZone::StaticClass(), FoundZones);

    TMap<ADeliveryZone*, float> ZoneWeights;
    float TotalWeight = 0.0f;

    for (AActor* Actor : FoundZones)
    {
        ADeliveryZone* Zone = Cast<ADeliveryZone>(Actor);
        if (!Zone) continue;

        float Dist = FVector::Dist(Zone->GetActorLocation(), PlayerLoc);
        float Weight = 0.0f;

        // WEIGHTED LOGIC (Same as Spawns but using DropOff distances)
        if (Dist < DropOffMinDistance)
        {
            Weight = 0.1f; // Penalty: Too close
        }
        else if (Dist >= DropOffMinDistance && Dist <= DropOffMaxPriorityDistance)
        {
            Weight = 100.0f; // Sweet Spot
        }
        else
        {
            float OverDistance = Dist - DropOffMaxPriorityDistance;
            if (OverDistance > DropOffFalloffDistance)
            {
                Weight = 0.0f; // Too far
            }
            else
            {
                float Decay = 1.0f - (OverDistance / DropOffFalloffDistance);
                Weight = FMath::Lerp(0.0f, 10.0f, Decay);
            }
        }

        if (Weight > 0.f)
        {
            ZoneWeights.Add(Zone, Weight);
            TotalWeight += Weight;
        }
    }

    // Roulette Selection
    if (TotalWeight <= 0.f) return nullptr;
    float RandomValue = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentSum = 0.0f;

    for (auto& Elem : ZoneWeights)
    {
        CurrentSum += Elem.Value;
        if (RandomValue <= CurrentSum) return Elem.Key;
    }

    return nullptr;
}

void ADeliveryDetonation_GameMode::ForcePostMatchForPlayer(APlayerController* PC)
{
    if (!PC) return;

    // If pawn isn't ready yet, retry shortly
    APawn* Pawn = PC->GetPawn();
    if (!Pawn)
    {
        TryForcePostMatchForPlayer(PC);
        return;
    }

    // Server-authoritative freeze
    Pawn->DisableInput(PC);

    if (ACharacter* Char = Cast<ACharacter>(Pawn))
    {
        if (UCharacterMovementComponent* Move = Char->GetCharacterMovement())
        {
            Move->StopMovementImmediately();
            Move->DisableMovement();
        }
    }

    // Tell that client to open the PostMatch UI
    if (AGamePlayerController* DDPC = Cast<AGamePlayerController>(PC))
    {
        DDPC->Client_EnterPostMatchUI();
    }
}

void ADeliveryDetonation_GameMode::TryForcePostMatchForPlayer(APlayerController* PC)
{
    if (!PC) return;

    // Don’t spam timers forever
    FTimerHandle TempHandle;
    FTimerDelegate Del;
    Del.BindLambda([this, PC]()
        {
            // PC might have disconnected
            if (IsValid(PC))
            {
                ForcePostMatchForPlayer(PC);
            }
        });

    GetWorldTimerManager().SetTimer(TempHandle, Del, 0.1f, false);
}

void ADeliveryDetonation_GameMode::RestartPlayer(AController* NewPlayer)
{
    Super::RestartPlayer(NewPlayer);

    // Rebind health events on the newly spawned pawn (server only)
    if (!NewPlayer) return;

    if (APawn* Pawn = NewPlayer->GetPawn())
    {
        if (UHealthComponent* HealthComp = Pawn->FindComponentByClass<UHealthComponent>())
        {
            HealthComp->OnHealthChanged.RemoveDynamic(this, &ADeliveryDetonation_GameMode::HandlePlayerHealthChanged);
            HealthComp->OnHealthChanged.AddDynamic(this, &ADeliveryDetonation_GameMode::HandlePlayerHealthChanged);
        }
    }
}