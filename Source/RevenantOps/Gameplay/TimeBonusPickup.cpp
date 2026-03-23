// Copyright RevenantOps. All Rights Reserved.

#include "TimeBonusPickup.h"
#include "MercenairesGameState.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"

ATimeBonusPickup::ATimeBonusPickup() {
  PrimaryActorTick.bCanEverTick = true;

  PickupMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
  RootComponent = PickupMesh;
  PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

  CollisionSphere =
      CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
  CollisionSphere->SetupAttachment(RootComponent);
  CollisionSphere->SetSphereRadius(100.f);
  CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void ATimeBonusPickup::BeginPlay() {
  Super::BeginPlay();

  InitialZ = GetActorLocation().Z;
  CollisionSphere->OnComponentBeginOverlap.AddDynamic(
      this, &ATimeBonusPickup::OnOverlapBegin);
}

void ATimeBonusPickup::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Bobbing
  if (bBobbing) {
    FVector Loc = GetActorLocation();
    Loc.Z =
        InitialZ + FMath::Sin(GetGameTimeSinceCreation() * BobSpeed) *
                        BobAmplitude;
    SetActorLocation(Loc);
  }

  // Rotation
  if (bRotating) {
    AddActorWorldRotation(
        FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
  }
}

void ATimeBonusPickup::OnOverlapBegin(
    UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  APawn *PlayerPawn = Cast<APawn>(OtherActor);
  if (!PlayerPawn || !PlayerPawn->IsPlayerControlled()) {
    return;
  }

  // Add time to the match
  AMercenairesGameState *GameState =
      GetWorld()->GetGameState<AMercenairesGameState>();
  if (GameState && GameState->IsMatchActive()) {
    GameState->AddBonusTime(BonusSeconds);
  }

  BP_OnPickedUp(PlayerPawn, BonusSeconds);
  HidePickup();
}

void ATimeBonusPickup::HidePickup() {
  SetActorHiddenInGame(true);
  SetActorEnableCollision(false);
  SetActorTickEnabled(false);

  if (RespawnTime > 0.f) {
    GetWorldTimerManager().SetTimer(
        RespawnTimer, this, &ATimeBonusPickup::RespawnPickup, RespawnTime,
        false);
  }
}

void ATimeBonusPickup::RespawnPickup() {
  SetActorHiddenInGame(false);
  SetActorEnableCollision(true);
  SetActorTickEnabled(true);
}
