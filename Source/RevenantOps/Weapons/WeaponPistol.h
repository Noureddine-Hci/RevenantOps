// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "WeaponPistol.generated.h"

/**
 *  Pistol — Semi-auto sidearm. Reliable backup weapon.
 *  25 damage, 400 RPM, 12-round magazine.
 */
UCLASS(Blueprintable)
class AWeaponPistol : public AWeaponBase {
  GENERATED_BODY()

public:
  AWeaponPistol();
};
