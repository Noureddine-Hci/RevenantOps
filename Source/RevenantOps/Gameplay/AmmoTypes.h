// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "AmmoTypes.generated.h"

/** Type de munitions — doit correspondre entre WeaponBase::WeaponAmmoType et les pickups */
UENUM(BlueprintType)
enum class EAmmoType : uint8 {
  None    UMETA(DisplayName = "Aucun"),
  Pistol  UMETA(DisplayName = "Pistolet"),
  Rifle   UMETA(DisplayName = "Fusil d'assaut"),
  Shotgun UMETA(DisplayName = "Fusil a pompe"),
  SMG     UMETA(DisplayName = "Mitraillette"),
  Sniper  UMETA(DisplayName = "Sniper"),
};
