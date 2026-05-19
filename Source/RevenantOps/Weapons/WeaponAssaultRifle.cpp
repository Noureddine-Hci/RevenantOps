// Copyright RevenantOps. All Rights Reserved.

#include "WeaponAssaultRifle.h"

// Toutes les stats viennent de DT_WeaponStats via ApplyWeaponDataRow().
AWeaponAssaultRifle::AWeaponAssaultRifle()
{
    WeaponName     = FText::FromString(TEXT("Assault Rifle"));
    WeaponCategory = EWeaponCategory::AssaultRifle;
}
