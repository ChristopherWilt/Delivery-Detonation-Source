#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DeliveryDetonation_GameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimeUpdated, float, NewTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCargoUpdated, int32, Delivered, int32, Total);

UENUM(BlueprintType)
enum class EMatchPhase : uint8
{
    Playing UMETA(DisplayName = "Playing"),
    PostMatch UMETA(DisplayName = "PostMatch")
};

UCLASS()
class DELIVERYDETONATION_API ADeliveryDetonation_GameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ADeliveryDetonation_GameState();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // --- REPLICATED DATA ---
    UPROPERTY(ReplicatedUsing = OnRep_TimeRemaining, BlueprintReadOnly, Category = "Game State")
    float TimeRemaining = 0.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CargoStats, BlueprintReadOnly, Category = "Game State")
    int32 TotalCargo = 0;

    UPROPERTY(ReplicatedUsing = OnRep_CargoStats, BlueprintReadOnly, Category = "Game State")
    int32 DeliveredCargo = 0;

    UPROPERTY(ReplicatedUsing = OnRep_MatchPhase, BlueprintReadOnly, Category = "Match")
    EMatchPhase MatchPhase = EMatchPhase::Playing;

    UPROPERTY(ReplicatedUsing = OnRep_RestartCountdown, BlueprintReadOnly, Category = "Match")
    int32 RestartCountdown = 0;

    // --- DELEGATES FOR UI ---
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnTimeUpdated OnTimeUpdated;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnCargoUpdated OnCargoUpdated;

protected:
    UFUNCTION()
    void OnRep_TimeRemaining();

    UFUNCTION()
    void OnRep_CargoStats();

    UFUNCTION() 
    void OnRep_MatchPhase();

    UFUNCTION() 
    void OnRep_PostMatchState();

    UFUNCTION() 
    void OnRep_RestartCountdown();
};