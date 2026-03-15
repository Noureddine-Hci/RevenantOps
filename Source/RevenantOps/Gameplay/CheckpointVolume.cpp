// Copyright RevenantOps. All Rights Reserved.

#include "CheckpointVolume.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RevenantOpsSaveGame.h"

ACheckpointVolume::ACheckpointVolume() {
  PrimaryActorTick.bCanEverTick = false;

  TriggerVolume =
      CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
  RootComponent = TriggerVolume;
  TriggerVolume->SetBoxExtent(FVector(200.f, 200.f, 150.f));
  TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void ACheckpointVolume::BeginPlay() {
  Super::BeginPlay();

  TriggerVolume->OnComponentBeginOverlap.AddDynamic(
      this, &ACheckpointVolume::OnOverlapBegin);

  // Default respawn to checkpoint location if not set
  if (RespawnTransform.GetLocation().IsNearlyZero()) {
    RespawnTransform = GetActorTransform();
  }
}

void ACheckpointVolume::OnOverlapBegin(
    UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  if (!OtherActor || !OtherActor->ActorHasTag(FName("Player"))) {
    return;
  }

  if (bActivated) {
    return;
  }

  bActivated = true;

  // Create/update save data
  URevenantOpsSaveGame *SaveData = Cast<URevenantOpsSaveGame>(
      UGameplayStatics::CreateSaveGameObject(
          URevenantOpsSaveGame::StaticClass()));

  if (SaveData) {
    // Store checkpoint info
    SaveData->WorldData.LastCheckpointID = CheckpointID;
    SaveData->WorldData.CurrentLevelName =
        FName(*GetWorld()->GetMapName());
    SaveData->SaveTimestamp = FDateTime::Now();

    // Store player transform
    if (APawn *Pawn = Cast<APawn>(OtherActor)) {
      SaveData->PlayerData.PlayerTransform = Pawn->GetActorTransform();
    }

    // Async save
    UGameplayStatics::AsyncSaveGameToSlot(SaveData, TEXT("Checkpoint"), 0);
  }

  // Notify
  OnCheckpointActivated.Broadcast(this, Cast<APawn>(OtherActor));
  BP_OnCheckpointReached();

  // Disable trigger after activation
  TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
