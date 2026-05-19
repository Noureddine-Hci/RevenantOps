// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CrosshairTypes.generated.h"

/**
 *  Style de réticule à afficher pour une arme donnée.
 *  Défini ici (header dédié) pour éviter les includes circulaires
 *  entre WeaponBase.h et WeaponTableRow.h.
 */
UENUM(BlueprintType)
enum class ECrosshairStyle : uint8
{
    None      UMETA(DisplayName = "Aucun (mêlée)"),
    Cross     UMETA(DisplayName = "Croix standard"),
    WideCross UMETA(DisplayName = "Croix large (shotgun)"),
    Dot       UMETA(DisplayName = "Point central (sniper)"),
};
