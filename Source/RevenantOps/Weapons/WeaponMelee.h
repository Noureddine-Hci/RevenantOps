// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "WeaponMelee.generated.h"

/**
 *  Machette — Melee weapon. High risk/reward close-range option.
 *  35 damage, sphere trace sweep, infinite ammo, no reload.
 */
UCLASS(Blueprintable)
class AWeaponMelee : public AWeaponBase {
  GENERATED_BODY()

public:
  AWeaponMelee();

protected:
  /** Override FireShot to use sphere trace instead of hitscan */
  virtual void FireShot() override;

  /** Melee trace distance in cm */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Melee",
            meta = (ClampMin = 50, ClampMax = 300))
  float MeleeRange = 100.f;

  /** Sphere trace radius in cm */
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon|Melee",
            meta = (ClampMin = 10, ClampMax = 150))
  float MeleeRadius = 50.f;
};
