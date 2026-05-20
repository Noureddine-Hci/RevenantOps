// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "Gameplay/InventoryItem.h"
#include "Gameplay/AmmoTypes.h"
#include "ItemDefinition.generated.h"

class AWeaponBase;

/**
 *  Source de vérité pour un item inventaire.
 *  Créer une instance DA_Item_* par type d'objet dans le Content Browser.
 *  Exemples : DA_Item_Ammo_Pistol, DA_Item_Health_Small, DA_Item_TimeBonus_30s
 *
 *  Règle : ce fichier N'inclut PAS RevenantOpsCharacter.h.
 *  MakeInventoryItem() retourne une struct valeur — aucun couplage au personnage.
 */
UCLASS(Blueprintable, BlueprintType)
class REVENANTOPS_API UItemDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ── Identité ──────────────────────────────────────────────────────────────

    /** Type de l'item — détermine la logique d'utilisation côté Character */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Identity")
    EInventoryItemType ItemType = EInventoryItemType::Empty;

    /** Nom affiché dans l'inventaire et les popups */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Identity")
    FText DisplayName;

    /** Description courte affichée dans le tooltip du slot */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Identity")
    FText Description;

    /** Icône affichée dans le slot inventaire */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Identity")
    TObjectPtr<UTexture2D> ItemIcon = nullptr;

    /**
     *  Mesh 3D affiché sur l'actor pickup dans le monde.
     *  Laisser vide = pickup invisible (juste la collision).
     *  Ex : boîte de munitions Kenney, spray médical, etc.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual")
    TObjectPtr<UStaticMesh> PickupMesh = nullptr;

    /** Scale appliqué au mesh 3D du pickup (ajuster selon la taille du mesh) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Visual")
    FVector PickupMeshScale = FVector(1.f);

    // ── Weapon ────────────────────────────────────────────────────────────────

    /** Classe BP de l'arme. Valide uniquement si ItemType == Weapon. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Weapon")
    TSubclassOf<AWeaponBase> WeaponClass;

    // ── Ammo ──────────────────────────────────────────────────────────────────

    /** Type de munitions. Valide si ItemType == Ammo. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Ammo")
    EAmmoType AmmoType = EAmmoType::None;

    // ── Health ────────────────────────────────────────────────────────────────

    /**
     *  HP à restaurer quand l'item est utilisé depuis l'inventaire (valeur absolue).
     *  Si 0, la logique côté Character peut calculer depuis un % de HP max.
     */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Health",
              meta = (ClampMin = 0.f))
    float HealAmount = 0.f;

    // ── TimeBonus ─────────────────────────────────────────────────────────────

    /** Secondes de bonus accordées. Valide si ItemType == TimeBonus. */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|TimeBonus",
              meta = (ClampMin = 0.f))
    float TimeBonusSeconds = 0.f;

    // ── API ───────────────────────────────────────────────────────────────────

    /**
     *  Fabrique un FInventoryItem runtime à partir de cette définition.
     *  @param Qty  Quantité initiale du slot (munitions, charges de soin, etc.)
     */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
    FInventoryItem MakeInventoryItem(int32 Qty = 1) const;

    /** Retourne true si la définition est valide et utilisable */
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Item")
    bool IsValid() const { return ItemType != EInventoryItemType::Empty; }

    // UPrimaryDataAsset — pour l'Asset Manager
    virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};

// ─────────────────────────────────────────────────────────────────────────────

/**
 *  Paire {définition + quantité} — utilisée dans FCharacterInfo::StartingItems
 *  pour configurer l'inventaire de départ sans remplir 9 champs manuellement.
 */
USTRUCT(BlueprintType)
struct REVENANTOPS_API FStartingItem
{
    GENERATED_BODY()

    /** DataAsset qui décrit l'item (DA_Item_Ammo_Pistol, DA_Item_Health_Small, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
    TObjectPtr<UItemDefinition> Definition = nullptr;

    /** Quantité placée dans le slot au début du match */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory",
              meta = (ClampMin = 1))
    int32 Quantity = 1;
};
