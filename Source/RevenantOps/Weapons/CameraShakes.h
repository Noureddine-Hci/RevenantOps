// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shakes/DefaultCameraShakeBase.h"
#include "CameraShakes.generated.h"

/**
 *  Subtle camera shake on weapon fire.
 */
UCLASS()
class UCS_WeaponFire : public UDefaultCameraShakeBase {
  GENERATED_BODY()
public:
  UCS_WeaponFire(const FObjectInitializer &ObjInit);
};

/**
 *  Stronger camera shake when taking damage.
 */
UCLASS()
class UCS_TakeDamage : public UDefaultCameraShakeBase {
  GENERATED_BODY()
public:
  UCS_TakeDamage(const FObjectInitializer &ObjInit);
};
