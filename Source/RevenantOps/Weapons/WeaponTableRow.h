// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "CrosshairTypes.h"
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

    // ========== BALANCE ==========

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

    // ========== CROSSHAIR ==========

    /** Style de réticule affiché quand cette arme est équipée */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
    ECrosshairStyle CrosshairStyle = ECrosshairStyle::Cross;

    /**
     *  Texture de scope affichée plein écran en ADS (sniper uniquement).
     *  Laisser vide pour les armes sans lunette.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair")
    TSoftObjectPtr<UTexture2D> ScopeOverlayTexture;

    /**
     *  Multiplicateur FOV ADS avec scope (ex: 0.4 = très zoomé).
     *  Ignoré si ScopeOverlayTexture est vide.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crosshair",
              meta = (ClampMin = 0.1f, ClampMax = 1.f))
    float ScopeFOVMultiplier = 0.4f;
};
