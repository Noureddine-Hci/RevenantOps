// Copyright RevenantOps. All Rights Reserved.

#include "InteractableBase.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/Pawn.h"

AInteractableBase::AInteractableBase() {
  PrimaryActorTick.bCanEverTick = false;

  // Root
  SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
  RootComponent = SceneRoot;

  // Interaction sphere
  InteractionSphere =
      CreateDefaultSubobject<USphereComponent>(TEXT("InteractionSphere"));
  InteractionSphere->SetupAttachment(RootComponent);
  InteractionSphere->SetSphereRadius(InteractionRadius);
  InteractionSphere->SetCollisionProfileName(TEXT("Trigger"));

  // Prompt widget
  PromptWidget =
      CreateDefaultSubobject<UWidgetComponent>(TEXT("PromptWidget"));
  PromptWidget->SetupAttachment(RootComponent);
  PromptWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
  PromptWidget->SetWidgetSpace(EWidgetSpace::Screen);
  PromptWidget->SetDrawSize(FVector2D(200.f, 50.f));
  PromptWidget->SetHiddenInGame(true);
}

void AInteractableBase::BeginPlay() {
  Super::BeginPlay();

  InteractionSphere->SetSphereRadius(InteractionRadius);

  InteractionSphere->OnComponentBeginOverlap.AddDynamic(
      this, &AInteractableBase::OnOverlapBegin);
  InteractionSphere->OnComponentEndOverlap.AddDynamic(
      this, &AInteractableBase::OnOverlapEnd);
}

// =============================================================================
// INTERACTION
// =============================================================================

void AInteractableBase::Interact(APawn *InteractingPawn) {
  if (!CanInteract()) {
    return;
  }

  if (!bReusable) {
    bHasBeenUsed = true;
  }

  // Notify via delegate and Blueprint
  OnInteracted.Broadcast(this, InteractingPawn);
  BP_OnInteract(InteractingPawn);

  // Hide prompt after single-use interaction
  if (!bReusable && PromptWidget) {
    PromptWidget->SetHiddenInGame(true);
  }
}

bool AInteractableBase::CanInteract() const {
  if (!bReusable && bHasBeenUsed) {
    return false;
  }
  return true;
}

// =============================================================================
// OVERLAP
// =============================================================================

void AInteractableBase::OnOverlapBegin(
    UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  if (OtherActor && OtherActor->ActorHasTag(FName("Player"))) {
    bPlayerInRange = true;
    if (CanInteract() && PromptWidget) {
      PromptWidget->SetHiddenInGame(false);
    }
  }
}

void AInteractableBase::OnOverlapEnd(UPrimitiveComponent *OverlappedComp,
                                      AActor *OtherActor,
                                      UPrimitiveComponent *OtherComp,
                                      int32 OtherBodyIndex) {
  if (OtherActor && OtherActor->ActorHasTag(FName("Player"))) {
    bPlayerInRange = false;
    if (PromptWidget) {
      PromptWidget->SetHiddenInGame(true);
    }
  }
}
