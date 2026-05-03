// Copyright RevenantOps. All Rights Reserved.

#include "PickupBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HealthComponent.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

APickupBase::APickupBase() {
  PrimaryActorTick.bCanEverTick = true;

  static ConstructorHelpers::FObjectFinder<USoundBase> DefaultSound(
      TEXT("/Game/Mercenaires/Audio/SFX/SW_Hit"));
  if (DefaultSound.Succeeded()) PickupSound = DefaultSound.Object;

  // Collision
  CollisionSphere =
      CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
  RootComponent = CollisionSphere;
  CollisionSphere->SetSphereRadius(60.f);
  CollisionSphere->SetCollisionProfileName(TEXT("Trigger"));

  // Visual mesh
  PickupMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
  PickupMesh->SetupAttachment(RootComponent);
  PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void APickupBase::BeginPlay() {
  Super::BeginPlay();

  InitialZ = GetActorLocation().Z;

  CollisionSphere->OnComponentBeginOverlap.AddDynamic(
      this, &APickupBase::OnOverlapBegin);
}

void APickupBase::Tick(float DeltaTime) {
  Super::Tick(DeltaTime);

  // Bobbing
  if (bBobbing) {
    FVector Location = GetActorLocation();
    Location.Z = InitialZ +
                 FMath::Sin(GetGameTimeSinceCreation() * BobSpeed) *
                     BobAmplitude;
    SetActorLocation(Location);
  }

  // Rotation
  if (bRotating) {
    AddActorWorldRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
  }
}

void APickupBase::OnOverlapBegin(UPrimitiveComponent *OverlappedComp,
                                  AActor *OtherActor,
                                  UPrimitiveComponent *OtherComp,
                                  int32 OtherBodyIndex, bool bFromSweep,
                                  const FHitResult &SweepResult) {
  if (OtherActor && OtherActor->ActorHasTag(FName("Player"))) {
    APawn *Pawn = Cast<APawn>(OtherActor);
    if (Pawn) {
      ApplyPickup(Pawn);
    }
  }
}

void APickupBase::ApplyPickup(APawn *TargetPawn) {
  if (!TargetPawn) {
    return;
  }

  switch (PickupType) {
  case EPickupType::Health: {
    if (UHealthComponent *HC =
            TargetPawn->FindComponentByClass<UHealthComponent>()) {
      // Don't pick up if already at full health
      if (HC->GetHealthPercent() >= 1.f) {
        return;
      }
      HC->Heal(Amount);
    }
    break;
  }

  case EPickupType::Armor: {
    // Shield/armor pickup - handled by HealthComponent shield
    // For now, heal as health since shield regen is automatic
    if (UHealthComponent *HC =
            TargetPawn->FindComponentByClass<UHealthComponent>()) {
      HC->Heal(Amount);
    }
    break;
  }

  case EPickupType::Ammo: {
    // Ammo pickup would need access to the weapon inventory
    // This is best handled through a delegate or Blueprint
    break;
  }

  case EPickupType::Weapon: {
    // Weapon pickup - handled in Blueprint
    break;
  }
  }

  if (PickupSound)
    UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());

  BP_OnPickedUp(TargetPawn);

  HidePickup();
}

void APickupBase::HidePickup() {
  SetActorHiddenInGame(true);
  SetActorEnableCollision(false);
  SetActorTickEnabled(false);

  if (RespawnTime > 0.f) {
    GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this,
                                           &APickupBase::RespawnPickup,
                                           RespawnTime, false);
  } else {
    Destroy();
  }
}

void APickupBase::RespawnPickup() {
  SetActorHiddenInGame(false);
  SetActorEnableCollision(true);
  SetActorTickEnabled(true);
}
