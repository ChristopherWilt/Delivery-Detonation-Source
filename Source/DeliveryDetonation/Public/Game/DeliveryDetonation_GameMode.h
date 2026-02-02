#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Game/DeliveryZone.h"
#include "DeliveryDetonation_GameMode.generated.h"

// Forward Declarations
class ABasePackage;
class APackageSpawnPoint;
class UHUDWidget;
class ADeliveryDetonation_GameState;
class UHealthComponent;
class APlayerCharacter;
class ADeliveryDetonation_PlayerState;

// Rarity Config Struct
USTRUCT(BlueprintType)
struct FPackageTypeEntry
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSubclassOf<ABasePackage> PackageClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0.1"))
    float SpawnWeight = 1.0f;
};

UCLASS()
class DELIVERYDETONATION_API ADeliveryDetonation_GameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ADeliveryDetonation_GameMode();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
    virtual void RestartPlayer(AController* NewPlayer) override;
    virtual void Logout(AController* Exiting) override;

    void Server_SetPlayerReady(APlayerController* PC, bool bReady);

    // --- NEW: FIX FOR BUILD ERROR ---
    UFUNCTION(BlueprintCallable, Category = "UI")
    void RegisterHUD(UHUDWidget* NewHUD);
    // --------------------------------

    // --- GAME RULES & CONFIG ---
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    int32 TargetDeliveryGoal = 5;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    float TimeLimitSeconds = 300.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    float RespawnDelay = 2.0f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Game Rules")
    FName MenuLevelName = "MainMenu";

    // --- UI CLASSES ---
    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> ResultsWidgetClass;

    // --- SPAWNING CONFIG ---
    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    TArray<FPackageTypeEntry> PackageTypes;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Distance")
    float MinDistance = 5000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Distance")
    float MaxPriorityDistance = 30000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning|Distance")
    float FalloffDistance = 70000.0f;

    // --- PUBLIC EVENTS ---
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void RegisterCargo(AActor* CargoActor);

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void NotifyCargoDelivered(AActor* CargoActor, AController* Deliverer);

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void NotifyCargoFailed(AActor* CargoActor);

    // --- DELIVERY SYSTEM ---
    // Called when player picks up a package
    void AssignDeliveryObjective(AController* Player);

    // Called by DeliveryZone to verify
    void TryCompleteDelivery(APlayerCharacter* PlayerChar, ADeliveryZone* Zone);

    void Server_PlayerQuitMatch(APlayerController* PC);

protected:
    // --- RUNTIME STATE ---
    UPROPERTY(BlueprintReadOnly, Category = "Game State")
    float TimeRemainingSeconds;

    bool bGameOver = false;

    UPROPERTY()
    ADeliveryDetonation_GameState* DDGameState;

    UPROPERTY()
    UHUDWidget* HUDWidgetInstance;

    UPROPERTY()
    UUserWidget* ResultsWidgetInstance;

    FTimerHandle ReturnToMenuHandle;

    // --- PER-PLAYER TRACKING ---
    TMap<TWeakObjectPtr<AController>, int32> PlayerScores;
    TMap<TWeakObjectPtr<AController>, FTimerHandle> PlayerRespawnTimers;

    UPROPERTY()
    TArray<AActor*> RegisteredCargo;

    // --- INTERNAL LOGIC ---
    void SpawnPackageForPlayer(AController* Player);

    UFUNCTION()
    void SpawnNextForPlayer(AController* Player);

    APackageSpawnPoint* SelectWeightedSpawnPoint(AController* TargetPlayer);
    TSubclassOf<ABasePackage> SelectWeightedPackageType();

    void TryWin(AController* Winner);
    void TryLoseByTime();

    void EnterPostMatch(AController* Winner);

    void TryStartRestartCountdown();
    void CancelRestartCountdown();
    void RestartCountdownTick();

    bool AreAllRemainingPlayersReady(int32& OutActive, int32& OutReady) const;

    FTimerHandle RestartCountdownHandle;

    UPROPERTY(EditDefaultsOnly, Category = "Match|Post")
    int32 RestartCountdownSeconds = 5;

    void OnWin();
    void OnLose();
    void ShowResults(bool bWon);

    UFUNCTION()
    void ReturnToMenu();

    UFUNCTION()
    void HandlePlayerHealthChanged(float Current, float Max);

    // --- DELIVERY SYSTEM ---
    // Config for Drop Off selection
    UPROPERTY(EditDefaultsOnly, Category = "Delivery|Distance")
    float DropOffMinDistance = 2000.0f; // Minimum 20 meters

    UPROPERTY(EditDefaultsOnly, Category = "Delivery|Distance")
    float DropOffMaxPriorityDistance = 8000.0f; // Preferred ~80 meters

    UPROPERTY(EditDefaultsOnly, Category = "Delivery|Distance")
    float DropOffFalloffDistance = 15000.0f;

    // TRACKING: Which zone is assigned to which player?
    TMap<TWeakObjectPtr<AController>, TWeakObjectPtr<ADeliveryZone>> PlayerObjectives;

    // Helpers
    
    ADeliveryZone* SelectWeightedDeliveryZone(AController* Player);

    void ForcePostMatchForPlayer(APlayerController* PC);

    void TryForcePostMatchForPlayer(APlayerController* PC);
};