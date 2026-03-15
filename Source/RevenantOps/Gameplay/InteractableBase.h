// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableBase.generated.h"

class USphereComponent;
class UWidgetComponent;

/**
 *  Delegate when interaction occurs
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnInteracted, AActor *,
                                              Interactable, APawn *,
                                              InteractingPawn);

/**
 *  Base class for all interactable objects in the game.
 *  Shows a prompt when the player is in range.
 *  Override in Blueprint to define specific interactions.
 */
UCLASS(abstract, Blueprintable)
class AInteractableBase : public AActor {
  GENERATED_BODY()

public:
  AInteractableBase();

protected:
  // ========== COMPONENTS ==========

  /** Root scene component */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USceneComponent *SceneRoot;

  /** Interaction trigger radius */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USphereComponent *InteractionSphere;

  /** Interaction prompt widget (shows "Press E") */
  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UWidgetComponent *PromptWidget;

  // ========== CONFIG ==========

  /** Interaction radius */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction",
            meta = (ClampMin = 50, ClampMax = 1000))
  float InteractionRadius = 200.f;

  /** Text displayed on the interaction prompt */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  FText InteractionPrompt = FText::FromString(TEXT("Interact"));

  /** Can this be interacted with multiple times */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interaction")
  bool bReusable = false;

  /** Has been used already */
  UPROPERTY(BlueprintReadOnly, Category = "Interaction")
  bool bHasBeenUsed = false;

  /** Is the player currently in range */
  bool bPlayerInRange = false;

public:
  // ========== EVENTS ==========

  UPROPERTY(BlueprintAssignable, Category = "Interaction|Events")
  FOnInteracted OnInteracted;

  // ========== PUBLIC API ==========

  /** Called by the player to interact */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  void Interact(APawn *InteractingPawn);

  /** Returns true if this can be interacted with */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  bool CanInteract() const;

  /** Returns the interaction prompt text */
  UFUNCTION(BlueprintCallable, Category = "Interaction")
  FText GetInteractionPrompt() const { return InteractionPrompt; }

protected:
  virtual void BeginPlay() override;

  /** Override in subclasses/BP to define what happens on interaction */
  UFUNCTION(BlueprintImplementableEvent, Category = "Interaction",
            meta = (DisplayName = "On Interact"))
  void BP_OnInteract(APawn *InteractingPawn);

  /** Overlap events */
  UFUNCTION()
  void OnOverlapBegin(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                      UPrimitiveComponent *OtherComp, int32 OtherBodyIndex,
                      bool bFromSweep, const FHitResult &SweepResult);

  UFUNCTION()
  void OnOverlapEnd(UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
                    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex);
};
