// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "WeaponSMG.generated.h"

/**
 *  SMG — High fire-rate close-quarters weapon.
 *  12 damage, 1000 RPM, 40-round magazine.
 */
UCLASS(Blueprintable)
class AWeaponSMG : public AWeaponBase {
  GENERATED_BODY()

public:
  AWeaponSMG();
};
