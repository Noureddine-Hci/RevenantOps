// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventoryItem.h"
#include "WeaponPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UTexture2D;
class ARevenantOpsCharacter;
class AWeaponBase;
class USoundBase;

/**
 *  Weapon pickup — appuie sur E pour ramasser et équiper une arme.
 *  Même pattern que AmmoBonusPickup.
 */
UCLASS(Blueprintable)
class AWeaponPickup : public AActor
{
    GENERATED_BODY()

public:
    AWeaponPickup();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PickupMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    /** Classe de l'arme à donner au joueur */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup")
    TSubclassOf<AWeaponBase> WeaponClass;

    /** Icône affichée dans le popup [E] */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Pickup")
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

    UFUNCTION(BlueprintImplementableEvent, Category = "Weapon Pickup")
    void BP_OnPickedUp(APawn* PickedUpBy);
};
