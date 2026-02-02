// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickups/BasePackage.h"
#include "Sound/SoundAttenuation.h"
#include "ExplosivePackage.generated.h"

class URadialForceComponent;
class USoundBase;
class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class DELIVERYDETONATION_API AExplosivePackage : public ABasePackage
{
	GENERATED_BODY()

public:
    AExplosivePackage();

    void StartFuseFromPickup();

    float GetRemainingFuseTime() const;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Explosion")
    float FuseSeconds = 6.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|FX")
    USoundAttenuation* ExplosionAttenuation = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Beep")
    USoundAttenuation* BeepAttenuation = nullptr;

protected:
    virtual void BeginPlay() override;

    // Override the fail hook so “Fail()” becomes “explode”
    virtual void OnFailFX(FName Reason) override;

    // Explosion config

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_ExplodeFX();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_BeepFX();

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Explosion")
    float ExplosionRadius = 450.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Explosion")
    float ExplosionImpulseStrength = 2000.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Explosion")
    float Damage = 60.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Explosion")
    TSubclassOf<UDamageType> DamageTypeClass;

    // Damage falloff config
    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Damage")
    float DamageInnerRadius = 200.0f; // full damage inside this

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Damage")
    float DamageOuterRadius = 450.0f; // damage reaches min by here (often same as ExplosionRadius)

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Damage")
    float DamageMin = 0.0f; // damage at outer radius

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Damage", meta = (ClampMin = "0.1"))
    float DamageFalloff = 1.0f; // higher = drops off faster

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Damage")
    TEnumAsByte<ECollisionChannel> DamagePreventionChannel = ECC_Visibility;

    // Beep behavior
    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Beep")
    USoundBase* Sfx_Beep = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Beep")
    float BeepStartInterval = 0.8f;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Beep")
    float BeepMinInterval = 0.12f;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|Beep")
    float BeepRampStartAt = 2.0f; // last X seconds ramp faster

    // Explosion FX
    UPROPERTY(EditDefaultsOnly, Category = "Bomb|FX")
    USoundBase* Sfx_Explosion = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Bomb|FX")
    UNiagaraSystem* Vfx_Explosion = nullptr;

    // Radial force component (physics push)
    UPROPERTY(VisibleAnywhere, Category = "Bomb|Components")
    URadialForceComponent* RadialForce;

    // Timers
    FTimerHandle FuseTimerHandle;
    FTimerHandle BeepTimerHandle;

    float FuseEndTime = 0.f;

    void StartFuse();
    void DoBeep();
    void ExplodeNow();
	
};
