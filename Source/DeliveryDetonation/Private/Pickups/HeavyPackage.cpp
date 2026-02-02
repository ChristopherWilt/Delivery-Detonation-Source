// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/HeavyPackage.h"

AHeavyPackage::AHeavyPackage()
{
    CarrySpeedMultiplier = 0.65f;  // slower
    CarryJumpMultiplier = 0.75f;  // shorter jump
    SlideFrictionMultiplier = 2.5f;  // more grip while sliding
    PackageWeightInKg = 80.f;   // heavier physics feel
}