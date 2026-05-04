// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/AmmoTypes.h"
#include "AmmoDropPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 *  Drop de munitions spawné à la mort d'un ennemi.
 *  Se ramasse automatiquement par overlap si le joueur a l'arme correspondante.
 */
UCLASS(Blueprintable)
class REVENANTOPS_API AAmmoDropPickup : public AActor
{
    GENERATED_BODY()

public:
    AAmmoDropPickup();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* CollisionSphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* PickupMesh;

public:
    /** Type de munitions donné au joueur */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop")
    EAmmoType AmmoType = EAmmoType::Pistol;

    /** Quantité de munitions */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop",
              meta = (ClampMin = 1, ClampMax = 200))
    int32 AmmoAmount = 15;

    /** Durée de vie avant disparition (secondes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop",
              meta = (ClampMin = 1.f, ClampMax = 60.f))
    float Lifetime = 12.f;

    /** Amplitude du bob vertical */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop|Visual")
    float BobAmplitude = 8.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop|Visual")
    float BobSpeed = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoDrop|Visual")
    float RotationSpeed = 90.f;

private:
    float TimeAlive = 0.f;
    float InitialZ   = 0.f;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                        bool bFromSweep, const FHitResult& SweepResult);
};
