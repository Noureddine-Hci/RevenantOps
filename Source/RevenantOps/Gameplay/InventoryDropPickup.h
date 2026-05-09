// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/InventoryItem.h"
#include "Gameplay/PickupInterface.h"
#include "InventoryDropPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class ARevenantOpsCharacter;

/**
 *  Pickup générique spawné quand le joueur jette un objet de son inventaire.
 *  Stocke un FInventoryItem et le rend à l'inventaire du joueur qui appuie sur E.
 *  Bob + rotation visuelle. Disparaît après Lifetime secondes.
 *
 *  Créer un WBP enfant pour assigner un mesh (sphere par défaut).
 */
UCLASS(Blueprintable)
class REVENANTOPS_API AInventoryDropPickup : public AActor, public IPickupInterface
{
    GENERATED_BODY()

public:
    AInventoryDropPickup();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* Sphere;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* Mesh;

    // Joueur en zone — raw ptr pour éviter CDO crash
    ARevenantOpsCharacter* PendingPlayer = nullptr;

    UFUNCTION()
    void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    float TimeAlive = 0.f;
    float InitialZ  = 0.f;

public:
    // ── Données de l'item droppé ─────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop")
    FInventoryItem HeldItem;

    /** Durée de vie avant disparition (0 = permanent) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop",
              meta = (ClampMin = 0.f, ClampMax = 120.f))
    float Lifetime = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Visual")
    float BobAmplitude = 10.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Visual")
    float BobSpeed = 2.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drop|Visual")
    float RotationSpeed = 80.f;

    // ── IPickupInterface ────────────────────────────────────────────────────

    virtual UTexture2D* GetPickupIcon_Implementation() const override
        { return HeldItem.ItemIcon; }

    virtual FText GetPickupDisplayName_Implementation() const override
        { return HeldItem.DisplayName; }

    virtual int32 GetPickupDisplayAmount_Implementation() const override
        { return HeldItem.Quantity; }

    virtual void TryPickupInteract_Implementation(ARevenantOpsCharacter* Player) override;

    // ── API publique ────────────────────────────────────────────────────────

    /** Initialise l'item droppé (appeler juste après le spawn). */
    void Initialize(const FInventoryItem& Item);
};
