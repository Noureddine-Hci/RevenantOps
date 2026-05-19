// Copyright RevenantOps. All Rights Reserved.

#include "WeaponShotgun.h"

// Toutes les stats viennent de DT_WeaponStats via ApplyWeaponDataRow().
AWeaponShotgun::AWeaponShotgun()
{
    WeaponName     = FText::FromString(TEXT("Shotgun"));
    WeaponCategory = EWeaponCategory::Shotgun;
}
