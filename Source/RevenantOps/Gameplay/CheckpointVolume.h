// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckpointVolume.generated.h"

class UBoxComponent;

/**
 *  Delegate when checkpoint is activated
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCheckpointActivated,
                                              ACheckpointVolume *, Checkpoint,
                                              APawn *, Activator);

/**
 *  Checkpoint volume. Triggers an autosave when the player walks through.
 *  Each checkpoint has a unique ID for save/load tracking.
 */
UCLASS(Blueprintable)
class ACheckpointVolume : public AActor {
  GENERATED_BODY()

public:
  ACheckpointVolume();

protected:
  virtual void BeginPlay() override;

  // ========== COMPONENTS ==========

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UBoxComponent *TriggerVolume;

  // ========== CONFIG ==========

  /** Unique checkpoint ID */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
  FName CheckpointID;

  /** Spawn point for respawning at this checkpoint */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
  FTransform RespawnTransform;

  /** Has this checkpoint been activated this session */
  UPROPERTY(BlueprintReadOnly, Category = "Checkpoint")
  bool bActivated = false;

public:
  UPROPERTY(BlueprintAssignable, Category = "Checkpoint|Events")
  FOnCheckpointActivated OnCheckpointActivated;

  /** Gets the checkpoint ID */
  UFUNCTION(BlueprintCallable, Category = "Checkpoint")
  FName GetCheckpointID() const { return CheckpointID; }

  /** Gets the respawn transform */
  UFUNCTION(BlueprintCallable, Category = "Checkpoint")
  FTransform GetRespawnTransform() const { return RespawnTransform; }

protected:
  UFUNCTION()
  void OnOverlapBegin(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                      UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                      bool bFromSweep, const FHitResult &SweepResult);

  UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint",
            meta = (DisplayName = "On Checkpoint Reached"))
  void BP_OnCheckpointReached();
};
