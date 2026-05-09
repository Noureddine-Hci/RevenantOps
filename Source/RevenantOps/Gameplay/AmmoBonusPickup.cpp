// Copyright RevenantOps. All Rights Reserved.

#include "AmmoBonusPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RevenantOpsCharacter.h"
#include "WeaponBase.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AAmmoBonusPickup::AAmmoBonusPickup() {
  PrimaryActorTick.bCanEverTick = true;

  static ConstructorHelpers::FObjectFinder<USoundBase> DefaultSound(
      TEXT("/Game/Mercenaires/Audio/SFX/SW_Hit"));
  if (DefaultSound.Succeeded()) PickupSound = DefaultSound.Object;

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

  // Lire icône et nom depuis DA si assigné, sinon champs manuels
  UTexture2D* PromptIcon = (ItemDefinition && ItemDefinition->ItemIcon.Get())
      ? ItemDefinition->ItemIcon.Get() : ItemIcon.Get();
  FText PromptName = (ItemDefinition && !ItemDefinition->DisplayName.IsEmpty())
      ? ItemDefinition->DisplayName : DisplayName;

  Player->ShowPickupPrompt(PromptIcon, PromptName, AmmoAmount);
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

  // Lire type/icône/nom depuis le DA si présent, sinon champs manuels
  EAmmoType   TypeToAdd      = TargetAmmoType;
  UTexture2D* EffectiveIcon  = ItemIcon.Get();
  FText       EffectiveName  = DisplayName;

  if (ItemDefinition && ItemDefinition->AmmoType != EAmmoType::None)
  {
    TypeToAdd     = ItemDefinition->AmmoType;
    EffectiveIcon = ItemDefinition->ItemIcon.Get();
    EffectiveName = ItemDefinition->DisplayName;
  }

  // Fallback : si toujours None, on prend l'arme courante
  if (TypeToAdd == EAmmoType::None)
  {
    if (AWeaponBase* Weapon = Player->GetCurrentWeapon())
      TypeToAdd = Weapon->GetWeaponAmmoType();
  }

  if (TypeToAdd != EAmmoType::None)
    Player->AddInventoryAmmo(TypeToAdd, AmmoAmount, EffectiveIcon, EffectiveName);

  PendingPlayer = nullptr;
  Player->ClearPendingPickup();
  Player->HidePickupPrompt();

  if (PickupSound)
    UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());

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
