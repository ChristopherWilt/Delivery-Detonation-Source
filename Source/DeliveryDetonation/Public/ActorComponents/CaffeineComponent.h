#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CaffeineComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCaffeineChanged, float, Current, float, Max);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class DELIVERYDETONATION_API UCaffeineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCaffeineComponent();

	UPROPERTY(BlueprintAssignable)
	FOnCaffeineChanged OnCaffeineChanged;

	float GetCurrent() const { return CurrentCaffeine; }
	float GetMax() const { return MaxCaffeine; }

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caffeine")
	float MaxCaffeine = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Caffeine")
	float DrainRate = 5.f; // per second

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CurrentCaffeine)
	float CurrentCaffeine = 0.f;

	UFUNCTION()
	void OnRep_CurrentCaffeine();

public:
	UFUNCTION(BlueprintCallable)
	void AddCaffeine(float Amount);

	UFUNCTION(BlueprintCallable)
	void Consume(float DeltaTime);
};