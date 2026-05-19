// Copyright RevenantOps. All Rights Reserved.

#include "WeaponSniper.h"

// Toutes les stats viennent de DT_WeaponStats via ApplyWeaponDataRow().
AWeaponSniper::AWeaponSniper()
{
    WeaponName     = FText::FromString(TEXT("Sniper"));
    WeaponCategory = EWeaponCategory::Sniper;
}
