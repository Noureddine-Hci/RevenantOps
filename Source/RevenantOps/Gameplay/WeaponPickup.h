// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventoryItem.h"
#include "Gameplay/ItemDefinition.h"
#include "Gameplay/PickupInterface.h"
#include "WeaponPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USkeletalMeshComponent;
class UTexture2D;
class ARevenantOpsCharacter;
class AWeaponBase;
class USoundBase;

/**
 *  Weapon pickup — appuie sur E pour ramasser et équiper une arme.
 *  Même pattern que AmmoBonusPickup.
 */
UCLASS(Blueprintable)
class AWeaponPickup : public AActor, public IPickupInterface
{
    GENERATED_BODY()

public:
    AWeaponPickup();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PickupMesh;

    /** Utilisé si ItemDefinition a un PickupSkeletalMesh */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* PickupSkeletalMeshComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    /**
     * DA source de vérité (DA_Item_Weapon_Pistol, etc.)
     * Si assigné : WeaponClass, mesh, icône et nom viennent du DA.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup")
    TObjectPtr<UItemDefinition> ItemDefinition = nullptr;

    /** Classe de l'arme — ignoré si ItemDefinition assigné */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup",
              meta = (EditCondition = "ItemDefinition == nullptr"))
    TSubclassOf<AWeaponBase> WeaponClass;

    /** Icône affichée dans le popup [E] — ignorée si ItemDefinition assigné */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup",
              meta = (EditCondition = "ItemDefinition == nullptr"))
    UTexture2D* WeaponIcon = nullptr;

    /** Son joué quand le joueur ramasse l'arme */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup|Audio")
    USoundBase* PickupSound = nullptr;

    /** Nom affiché dans le popup */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup")
    FText WeaponDisplayName = FText::FromString(TEXT("Arme"));

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup|Visual")
    bool bBobbing = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup|Visual")
    float BobAmplitude = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup|Visual")
    bool bRotating = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup|Visual")
    float RotationSpeed = 90.f;

    float InitialZ = 0.f;

    /** Joueur en zone */
    ARevenantOpsCharacter* PendingPlayer = nullptr;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    void HidePickup();

public:
    /** Appelé par le personnage quand il appuie sur E */
    void TryPickup(ARevenantOpsCharacter* Player);

    // ── IPickupInterface ─────────────────────────────────────────────────
    virtual UTexture2D* GetPickupIcon_Implementation() const override { return WeaponIcon; }
    virtual FText GetPickupDisplayName_Implementation() const override { return WeaponDisplayName; }
    virtual int32 GetPickupDisplayAmount_Implementation() const override { return 1; }
    virtual void TryPickupInteract_Implementation(ARevenantOpsCharacter* Player) override { TryPickup(Player); }

    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon Pickup")
    void BP_OnPickedUp(APawn* PickedUpBy);
};
