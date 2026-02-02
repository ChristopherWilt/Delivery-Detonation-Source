// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/ExplosivePackage.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "Sound/SoundBase.h"
#include "TimerManager.h"

AExplosivePackage::AExplosivePackage()
{
    RadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForce"));
    RadialForce->SetupAttachment(GetRootComponent());

    // Default radial force settings (we’ll update values at explode-time too)
    RadialForce->Radius = ExplosionRadius;
    RadialForce->ImpulseStrength = ExplosionImpulseStrength;
    RadialForce->bImpulseVelChange = true;
    RadialForce->bAutoActivate = false;

    // Push physics bodies; characters won’t move from this (damage handles them)
    RadialForce->AddCollisionChannelToAffect(ECC_PhysicsBody);
    RadialForce->AddCollisionChannelToAffect(ECC_WorldDynamic);
}

void AExplosivePackage::BeginPlay()
{
    Super::BeginPlay();
}

void AExplosivePackage::StartFuse()
{
    if (FuseSeconds <= 0.f) return;

    FuseEndTime = GetWorld()->GetTimeSeconds() + FuseSeconds;

    // Start beeping right away
    DoBeep();

    // Explosion timer
    GetWorldTimerManager().SetTimer(FuseTimerHandle, this, &AExplosivePackage::ExplodeNow, FuseSeconds, false);
}

void AExplosivePackage::DoBeep()
{
    if (bDelivered || bFailed) return;

    if (Sfx_Beep)
    {
        Multicast_BeepFX();
    }

    // Ramp beep speed near the end
    const float Now = GetWorld()->GetTimeSeconds();
    const float TimeLeft = FMath::Max(0.f, FuseEndTime - Now);

    float Interval = BeepStartInterval;

    if (TimeLeft <= BeepRampStartAt)
    {
        // Map TimeLeft: rampStart..0 => startInterval..minInterval
        const float Alpha = 1.f - (TimeLeft / FMath::Max(0.01f, BeepRampStartAt));
        Interval = FMath::Lerp(BeepStartInterval, BeepMinInterval, Alpha);
    }

    GetWorldTimerManager().SetTimer(BeepTimerHandle, this, &AExplosivePackage::DoBeep, Interval, false);
}

void AExplosivePackage::Multicast_BeepFX_Implementation()
{
    if (Sfx_Beep)
    {
        UGameplayStatics::PlaySoundAtLocation(
            this, Sfx_Beep, GetActorLocation(),
            FRotator::ZeroRotator,
            1.0f, 1.0f, 0.0f,
            BeepAttenuation
        );
    }
}

void AExplosivePackage::ExplodeNow()
{
    if (bDelivered || bFailed) return;

    DamageOuterRadius = FMath::Max(DamageOuterRadius, DamageInnerRadius);
    ExplosionRadius = FMath::Max(ExplosionRadius, DamageOuterRadius);

    // Trigger the standard “failed” path, but our OnFailFX will do explosion stuff
    Fail("Exploded");
}

void AExplosivePackage::OnFailFX(FName Reason)
{
    if (HasAuthority())
    {
        Multicast_ExplodeFX();
    }
}

void AExplosivePackage::Multicast_ExplodeFX_Implementation()
{
    // Stop timers so no extra beeps after explode
    GetWorldTimerManager().ClearTimer(BeepTimerHandle);
    GetWorldTimerManager().ClearTimer(FuseTimerHandle);

    const FVector Loc = GetActorLocation();

    // SFX + VFX
    if (Sfx_Explosion)
        UGameplayStatics::PlaySoundAtLocation(
            this,
            Sfx_Explosion,
            Loc,
            FRotator::ZeroRotator,
            1.0f,  // VolumeMultiplier
            1.0f,  // PitchMultiplier
            0.0f,  // StartTime
            ExplosionAttenuation
        );

    if (Vfx_Explosion)
    {
        UNiagaraComponent* FX =
            UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                GetWorld(),
                Vfx_Explosion,
                Loc
            );

        if (FX)
        {
            FX->SetAutoDestroy(true);

            // TWeakObjectPtr to prevent accessing a destroyed object
            TWeakObjectPtr<UNiagaraComponent> WeakFX = FX;

            // match explosion length
            constexpr float ExplosionFXDuration = 1.3f;

            FTimerHandle KillFXTimer;
            GetWorldTimerManager().SetTimer(
                KillFXTimer,
                [WeakFX]()
                {
                    // Check if the component still exists before accessing it
                    if (UNiagaraComponent* Comp = WeakFX.Get())
                    {
                        Comp->Deactivate();
                    }
                },
                ExplosionFXDuration,
                false
            );
        }
    }

    // Radial physics impulse
    if (HasAuthority() && RadialForce)
    {
        RadialForce->Radius = ExplosionRadius;
        RadialForce->ImpulseStrength = ExplosionImpulseStrength;
        RadialForce->FireImpulse();
    }

    // Radial damage to actors
    if (HasAuthority() && Damage > 0.f)
    {
        // Resolve damage type class safely
        UClass* ResolvedDamageType = DamageTypeClass ? DamageTypeClass.Get() : UDamageType::StaticClass();

        // Optional ignore list (add actors here later)
        TArray<AActor*> IgnoreActors;
        IgnoreActors.Add(this);

        UGameplayStatics::ApplyRadialDamageWithFalloff(
            this,                   // WorldContextObject
            Damage,                 // BaseDamage
            DamageMin,              // MinimumDamage
            Loc,                    // Origin
            DamageInnerRadius,      // DamageInnerRadius
            DamageOuterRadius,      // DamageOuterRadius
            DamageFalloff,          // DamageFalloff
            ResolvedDamageType,     // DamageTypeClass (UClass*)
            IgnoreActors,           // IgnoreActors
            this,                   // DamageCauser
            GetInstigatorController(),
            DamagePreventionChannel // DamagePreventionChannel
        );
    }

    // Optional: if we still want default “OnFail” FX from BasePackage, call Super after
    // Super::OnFailFX(Reason);
}

void AExplosivePackage::StartFuseFromPickup()
{
    if (bDelivered || bFailed) return;

    if (!GetWorldTimerManager().IsTimerActive(FuseTimerHandle))
    {
        StartFuse();
    }
}

float AExplosivePackage::GetRemainingFuseTime() const
{
    return FMath::Max(0.f, FuseEndTime - GetWorld()->GetTimeSeconds());
}