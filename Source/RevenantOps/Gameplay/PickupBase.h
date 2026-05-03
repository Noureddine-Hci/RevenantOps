// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupBase.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class USoundBase;

/**
 *  Pickup type
 */
UENUM(BlueprintType)
enum class EPickupType : uint8 {
  Health UMETA(DisplayName = "Health Pack"),
  Armor UMETA(DisplayName = "Armor/Shield"),
  Ammo UMETA(DisplayName = "Ammo Box"),
  Weapon UMETA(DisplayName = "Weapon Pickup")
};

/**
 *  Base pickup class.
 *  Automatically picked up on overlap with the player.
 *  Can optionally bob and rotate for visual appeal.
 */
UCLASS(Blueprintable)
class APickupBase : public AActor {
  GENERATED_BODY()

public:
  APickupBase();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  // ========== COMPONENTS ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent *PickupMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USphereComponent *CollisionSphere;

  // ========== CONFIG ==========

  /** Type of pickup */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup")
  EPickupType PickupType = EPickupType::Health;

  /** Amount to restore (health points, ammo count, etc.) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup",
            meta = (ClampMin = 1, ClampMax = 999))
  float Amount = 25.f;

  /** Does this pickup bob up and down */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual")
  bool bBobbing = true;

  /** Bob amplitude in cm */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual",
            meta = (ClampMin = 0, ClampMax = 50))
  float BobAmplitude = 10.f;

  /** Bob speed */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual",
            meta = (ClampMin = 0, ClampMax = 10))
  float BobSpeed = 2.f;

  /** Does this pickup rotate */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual")
  bool bRotating = true;

  /** Rotation speed (degrees per second) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Visual",
            meta = (ClampMin = 0, ClampMax = 360))
  float RotationSpeed = 90.f;

  /** Sound played when the pickup is collected */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup|Audio")
  USoundBase* PickupSound = nullptr;

  /** Respawn time (0 = no respawn) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup",
            meta = (ClampMin = 0, ClampMax = 120))
  float RespawnTime = 0.f;

  /** Initial Z location for bobbing */
  float InitialZ = 0.f;

  /** Timer for respawn */
  FTimerHandle RespawnTimer;

  // ========== LOGIC ==========

  UFUNCTION()
  void OnOverlapBegin(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                      UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                      bool bFromSweep, const FHitResult &SweepResult);

  /** Applies the pickup effect to the pawn */
  void ApplyPickup(APawn *TargetPawn);

  /** Hides the pickup and starts respawn timer */
  void HidePickup();

  /** Shows the pickup again */
  void RespawnPickup();

  /** BP hook for pickup effects (sound, VFX) */
  UFUNCTION(BlueprintImplementableEvent, Category = "Pickup",
            meta = (DisplayName = "On Picked Up"))
  void BP_OnPickedUp(APawn *PickedUpBy);
};
