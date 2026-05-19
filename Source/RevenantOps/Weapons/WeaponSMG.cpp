// Copyright RevenantOps. All Rights Reserved.

#include "WeaponSMG.h"

// Toutes les stats viennent de DT_WeaponStats via ApplyWeaponDataRow().
AWeaponSMG::AWeaponSMG()
{
    WeaponName     = FText::FromString(TEXT("SMG"));
    WeaponCategory = EWeaponCategory::SMG;
}
