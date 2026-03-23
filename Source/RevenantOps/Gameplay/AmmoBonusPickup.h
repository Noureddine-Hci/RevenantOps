// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoBonusPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 *  Ammo pickup for the arena.
 *  Refills a percentage of the current weapon's reserve ammo on overlap.
 */
UCLASS(Blueprintable)
class AAmmoBonusPickup : public AActor {
  GENERATED_BODY()

public:
  AAmmoBonusPickup();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *PickupMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USphereComponent *CollisionSphere;

  /** Ammo to add to each weapon's reserve */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup",
            meta = (ClampMin = 1, ClampMax = 999))
  int32 AmmoAmount = 30;

  /** Bob and rotate like other pickups */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Visual")
  bool bBobbing = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Visual")
  float BobAmplitude = 8.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Visual")
  float BobSpeed = 2.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Visual")
  bool bRotating = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Visual")
  float RotationSpeed = 90.f;

  /** Respawn time (0 = no respawn) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup",
            meta = (ClampMin = 0, ClampMax = 300))
  float RespawnTime = 45.f;

  float InitialZ = 0.f;
  FTimerHandle RespawnTimer;

  UFUNCTION()
  void OnOverlapBegin(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                      UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                      bool bFromSweep, const FHitResult &SweepResult);

  void HidePickup();
  void RespawnPickup();

  UFUNCTION(BlueprintImplementableEvent, Category = "AmmoPickup",
            meta = (DisplayName = "On Ammo Picked Up"))
  void BP_OnPickedUp(APawn *PickedUpBy, int32 AmmoAdded);
};
