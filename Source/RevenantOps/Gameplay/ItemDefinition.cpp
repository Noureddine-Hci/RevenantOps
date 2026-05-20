// Copyright RevenantOps. All Rights Reserved.
#include "Gameplay/ItemDefinition.h"

FInventoryItem UItemDefinition::MakeInventoryItem(int32 Qty) const
{
    FInventoryItem Out;
    Out.Type             = ItemType;
    Out.DisplayName      = DisplayName;
    Out.Description      = Description;
    Out.Quantity         = FMath::Max(1, Qty);
    Out.ItemIcon         = ItemIcon.Get();
    Out.WeaponClass      = WeaponClass;
    Out.HealAmount       = HealAmount;
    Out.AmmoType         = AmmoType;
    Out.TimeBonusSeconds = TimeBonusSeconds;
    Out.DropMesh         = PickupMesh.Get();
    Out.DropMeshScale    = PickupMeshScale;
    return Out;
}

FPrimaryAssetId UItemDefinition::GetPrimaryAssetId() const
{
    return FPrimaryAssetId(TEXT("ItemDefinition"), GetFName());
}
