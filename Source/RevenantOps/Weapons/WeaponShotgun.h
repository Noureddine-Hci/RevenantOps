// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "WeaponShotgun.generated.h"

/**
 *  Shotgun weapon — fires 8 pellets in a wide spread cone.
 *  High close-range damage, nearly useless at distance.
 *  Semi-auto with slow pump-action reload.
 */
UCLASS(Blueprintable)
class AWeaponShotgun : public AWeaponBase {
  GENERATED_BODY()

public:
  AWeaponShotgun();
};
