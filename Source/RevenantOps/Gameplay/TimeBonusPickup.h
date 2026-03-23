// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TimeBonusPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;

/**
 *  Time bonus pickup for Mercenaires mode.
 *  Adds seconds to the match timer when the player overlaps.
 *  Bobs and rotates for visibility. Can respawn after a delay.
 */
UCLASS(Blueprintable)
class ATimeBonusPickup : public AActor {
  GENERATED_BODY()

public:
  ATimeBonusPickup();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  // ========== COMPONENTS ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *PickupMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USphereComponent *CollisionSphere;

  // ========== CONFIG ==========

  /** Seconds added to the match timer */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeBonus",
            meta = (ClampMin = 5, ClampMax = 120))
  float BonusSeconds = 30.f;

  /** Bob up and down */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeBonus|Visual")
  bool bBobbing = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeBonus|Visual",
            meta = (ClampMin = 0, ClampMax = 50))
  float BobAmplitude = 10.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeBonus|Visual",
            meta = (ClampMin = 0, ClampMax = 10))
  float BobSpeed = 2.f;

  /** Rotate on Z axis */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeBonus|Visual")
  bool bRotating = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeBonus|Visual",
            meta = (ClampMin = 0, ClampMax = 360))
  float RotationSpeed = 90.f;

  /** Respawn time (0 = no respawn, one-time pickup) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TimeBonus",
            meta = (ClampMin = 0, ClampMax = 300))
  float RespawnTime = 60.f;

  // ========== INTERNAL ==========

  float InitialZ = 0.f;
  FTimerHandle RespawnTimer;

  UFUNCTION()
  void OnOverlapBegin(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                      UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                      bool bFromSweep, const FHitResult &SweepResult);

  void HidePickup();
  void RespawnPickup();

  /** Blueprint hook for pickup VFX/SFX */
  UFUNCTION(BlueprintImplementableEvent, Category = "TimeBonus",
            meta = (DisplayName = "On Time Bonus Picked Up"))
  void BP_OnPickedUp(APawn *PickedUpBy, float SecondsAdded);
};
