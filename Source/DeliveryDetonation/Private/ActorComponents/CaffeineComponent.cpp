#include "ActorComponents/CaffeineComponent.h"
#include "Net/UnrealNetwork.h"

UCaffeineComponent::UCaffeineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UCaffeineComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UCaffeineComponent, CurrentCaffeine);
}

void UCaffeineComponent::BeginPlay()
{
	Super::BeginPlay();
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		CurrentCaffeine = 0.f;
	}
}

void UCaffeineComponent::OnRep_CurrentCaffeine()
{
	OnCaffeineChanged.Broadcast(CurrentCaffeine, MaxCaffeine);
}

void UCaffeineComponent::AddCaffeine(float Amount)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	CurrentCaffeine = FMath::Clamp(CurrentCaffeine + Amount, 0.f, MaxCaffeine);
	OnRep_CurrentCaffeine();
}

void UCaffeineComponent::Consume(float DeltaTime)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	if (CurrentCaffeine <= 0.f) return;

	CurrentCaffeine = FMath::Clamp(CurrentCaffeine - (DrainRate * DeltaTime), 0.f, MaxCaffeine);
	OnRep_CurrentCaffeine();
}

void UCaffeineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	// Only consume on server
	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Consume(DeltaTime);
	}
}