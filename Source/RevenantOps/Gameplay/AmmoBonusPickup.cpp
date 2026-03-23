// Copyright RevenantOps. All Rights Reserved.

#include "AmmoBonusPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Character.h"
#include "RevenantOpsCharacter.h"
#include "WeaponBase.h"

AAmmoBonusPickup::AAmmoBonusPickup() {
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

void AAmmoBonusPickup::BeginPlay() {
  Super::BeginPlay();
  InitialZ = GetActorLocation().Z;
  CollisionSphere->OnComponentBeginOverlap.AddDynamic(
      this, &AAmmoBonusPickup::OnOverlapBegin);
}

void AAmmoBonusPickup::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  if (bBobbing) {
    FVector Loc = GetActorLocation();
    Loc.Z = InitialZ +
            FMath::Sin(GetGameTimeSinceCreation() * BobSpeed) * BobAmplitude;
    SetActorLocation(Loc);
  }

  if (bRotating) {
    AddActorWorldRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
  }
}

void AAmmoBonusPickup::OnOverlapBegin(
    UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep,
    const FHitResult &SweepResult) {
  ARevenantOpsCharacter *Player =
      Cast<ARevenantOpsCharacter>(OtherActor);
  if (!Player) {
    return;
  }

  AWeaponBase *Weapon = Player->GetCurrentWeapon();
  if (Weapon) {
    Weapon->AddReserveAmmo(AmmoAmount);
  }

  BP_OnPickedUp(Player, AmmoAmount);
  HidePickup();
}

void AAmmoBonusPickup::HidePickup() {
  SetActorHiddenInGame(true);
  SetActorEnableCollision(false);
  SetActorTickEnabled(false);

  if (RespawnTime > 0.f) {
    GetWorldTimerManager().SetTimer(
        RespawnTimer, this, &AAmmoBonusPickup::RespawnPickup, RespawnTime,
        false);
  }
}

void AAmmoBonusPickup::RespawnPickup() {
  SetActorHiddenInGame(false);
  SetActorEnableCollision(true);
  SetActorTickEnabled(true);
}
