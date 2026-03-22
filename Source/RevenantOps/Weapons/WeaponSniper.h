// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "WeaponSniper.generated.h"

/**
 *  Sniper rifle — single high-damage shot with strong zoom scope.
 *  Bolt-action feel with very slow fire rate.
 *  Rewards precision with 3x headshot multiplier.
 */
UCLASS(Blueprintable)
class AWeaponSniper : public AWeaponBase {
  GENERATED_BODY()

public:
  AWeaponSniper();
};
