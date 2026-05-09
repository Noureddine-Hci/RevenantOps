// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Gameplay/PickupBase.h"
#include "Gameplay/PickupInterface.h"
#include "Gameplay/ItemDefinition.h"
#include "HealthPickup.generated.h"

class ARevenantOpsCharacter;

/**
 *  Health pickup — soigne le joueur d'un pourcentage de sa santé max.
 *  Le ramassage est manuel (touche E) avec popup icône.
 *  Crée 3 BPs enfants : BP_HealthPickup_Small (25%), Medium (50%), Full (100%).
 */
UCLASS(Blueprintable)
class AHealthPickup : public APickupBase, public IPickupInterface
{
    GENERATED_BODY()

public:
    AHealthPickup();

protected:
    virtual void BeginPlay() override;

    /**
     *  Pourcentage de la santé max à restaurer (0.25 = 25%, 0.5 = 50%, 1.0 = 100%).
     *  Clamp automatique à [0.01 – 1.0].
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthPickup",
              meta = (ClampMin = 0.01f, ClampMax = 1.0f))
    float HealPercent = 0.25f;

    /** Icône affichée dans le popup au ramassage */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthPickup")
    TObjectPtr<UTexture2D> PickupIcon = nullptr;

    /** Nom affiché dans le popup (ex: "Kit de Soin", "Médicament", "Trousse de Premiers Secours") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthPickup")
    FText PickupDisplayName = FText::FromString("Kit de Soin");

    /**
     *  DataAsset qui décrit ce pickup comme item inventaire (DA_Item_Health_*).
     *  Si assigné → le soin va dans l'inventaire pour être utilisé plus tard.
     *  Si null   → soin immédiat (comportement classique, rétrocompatible).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthPickup")
    TObjectPtr<UItemDefinition> ItemDefinition = nullptr;

    /** Joueur actuellement en zone — raw ptr (pas de UPROPERTY pour éviter CDO crash) */
    ARevenantOpsCharacter* PendingPlayer = nullptr;

    // Override de l'overlap pour passer en mode "appuie E" au lieu d'auto-pickup
    UFUNCTION()
    void OnHealthOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                              UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                              bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnHealthOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
    // ── IPickupInterface ─────────────────────────────────────────────────
    virtual UTexture2D*  GetPickupIcon_Implementation()          const override { return PickupIcon; }
    virtual FText        GetPickupDisplayName_Implementation()   const override { return PickupDisplayName; }
    virtual int32        GetPickupDisplayAmount_Implementation() const override;
    virtual void         TryPickupInteract_Implementation(ARevenantOpsCharacter* Player) override;

    /** Appele par le personnage quand il appuie sur E. */
    void TryPickup(ARevenantOpsCharacter* Player);
};
