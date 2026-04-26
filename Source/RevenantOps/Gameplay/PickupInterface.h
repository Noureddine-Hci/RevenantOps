// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "PickupInterface.generated.h"

class ARevenantOpsCharacter;

UINTERFACE(MinimalAPI, BlueprintType)
class UPickupInterface : public UInterface
{
    GENERATED_BODY()
};

/**
 *  Interface implementee par tous les pickups interactifs (munitions, soins...).
 *  Permet de recuperer l'icone et le nom directement depuis le BP du pickup,
 *  sans avoir a les renseigner manuellement dans chaque entree de loot.
 */
class IPickupInterface
{
    GENERATED_BODY()

public:
    /** Icone du pickup — definie une seule fois dans le BP, utilisee partout */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
    UTexture2D* GetPickupIcon() const;

    /** Nom affiché dans le popup */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
    FText GetPickupDisplayName() const;

    /** Quantite affichee dans le popup (munitions, HP, etc.) */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
    int32 GetPickupDisplayAmount() const;

    /** Appelé quand le joueur appuie sur E */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Pickup")
    void TryPickupInteract(ARevenantOpsCharacter* Player);
};
