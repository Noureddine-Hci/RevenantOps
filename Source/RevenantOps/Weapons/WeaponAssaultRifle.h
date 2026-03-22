// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "WeaponAssaultRifle.generated.h"

/**
 *  Assault Rifle — Full-auto versatile primary weapon.
 *  18 damage, 700 RPM, 30-round magazine.
 */
UCLASS(Blueprintable)
class AWeaponAssaultRifle : public AWeaponBase {
  GENERATED_BODY()

public:
  AWeaponAssaultRifle();
};
