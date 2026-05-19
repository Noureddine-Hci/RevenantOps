// Copyright RevenantOps. All Rights Reserved.

#include "WeaponPistol.h"

// Toutes les stats viennent de DT_WeaponStats via ApplyWeaponDataRow().
// Cette classe existe uniquement pour servir de parent aux BP_Pistol*.
AWeaponPistol::AWeaponPistol()
{
    WeaponName     = FText::FromString(TEXT("Pistol"));
    WeaponCategory = EWeaponCategory::Pistol;
}
