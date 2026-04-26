// Copyright RevenantOps. All Rights Reserved.

#include "AmmoBonusPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RevenantOpsCharacter.h"
#include "WeaponBase.h"
#include "TimerManager.h"

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
  CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AAmmoBonusPickup::OnOverlapBegin);
  CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AAmmoBonusPickup::OnOverlapEnd);

  // Le timer DropLifetime est démarré manuellement via StartLifetimeTimer()
  // après que les propriétés post-spawn ont été assignées.
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
  ARevenantOpsCharacter *Player = Cast<ARevenantOpsCharacter>(OtherActor);
  if (!Player) return;

  PendingPlayer = Player;
  Player->SetPendingPickup(this);
  Player->ShowPickupPrompt(ItemIcon, DisplayName, AmmoAmount);
}

void AAmmoBonusPickup::OnOverlapEnd(
    UPrimitiveComponent *OverlappedComp, AActor *OtherActor,
    UPrimitiveComponent *OtherComp, int32 OtherBodyIndex) {
  ARevenantOpsCharacter *Player = Cast<ARevenantOpsCharacter>(OtherActor);
  if (!Player || Player != PendingPlayer) return;

  PendingPlayer = nullptr;
  Player->ClearPendingPickup();
  Player->HidePickupPrompt();
}

void AAmmoBonusPickup::TryPickupInteract_Implementation(ARevenantOpsCharacter* Player)
{
  TryPickup(Player);
}

void AAmmoBonusPickup::TryPickup(ARevenantOpsCharacter* Player) {
  if (!Player) return;

  AWeaponBase* Weapon = Player->GetCurrentWeapon();
  if (Weapon)
  {
    // Si TargetAmmoType est défini, ne donner des munitions que si l'arme correspond
    bool bTypeMatch = (TargetAmmoType == EAmmoType::None)
                   || (Weapon->GetWeaponAmmoType() == TargetAmmoType);
    if (bTypeMatch)
      Weapon->AddReserveAmmo(AmmoAmount);
  }

  PendingPlayer = nullptr;
  Player->ClearPendingPickup();
  Player->HidePickupPrompt();

  BP_OnPickedUp(Player, AmmoAmount);

  // Drop ennemi : on détruit l'acteur, pas de respawn
  if (DropLifetime > 0.f)
  {
    GetWorldTimerManager().ClearTimer(LifetimeTimer);
    Destroy();
  }
  else
    HidePickup();
}

void AAmmoBonusPickup::StartLifetimeTimer()
{
  if (DropLifetime > 0.f && !GetWorldTimerManager().IsTimerActive(LifetimeTimer))
  {
    GetWorldTimerManager().SetTimer(LifetimeTimer, this,
        &AAmmoBonusPickup::OnLifetimeExpired, DropLifetime, false);
  }
}

void AAmmoBonusPickup::OnLifetimeExpired()
{
  // Cacher le prompt si le joueur est encore en zone
  if (PendingPlayer)
  {
    PendingPlayer->ClearPendingPickup();
    PendingPlayer->HidePickupPrompt();
  }
  Destroy();
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
