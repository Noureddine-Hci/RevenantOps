// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "WeaponTableRow.generated.h"

/**
 *  DataTable row struct for weapon balance stats.
 *  Used by WeaponBase::ApplyWeaponDataRow() at BeginPlay.
 *  Create DT_WeaponStats in /Game/Mercenaires/Data/ with this row type.
 */
USTRUCT(BlueprintType)
struct FWeaponTableRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Base damage per hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float Damage = 20.f;

    /** Rounds per minute */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float FireRate = 600.f;

    /** Magazine capacity */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    int32 MaxAmmo = 30;

    /** Maximum hitscan range in cm */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
    float Range = 10000.f;
};
