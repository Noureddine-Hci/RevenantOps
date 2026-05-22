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
  CollisionSphere->UpdateOverlaps();

  // Pickup statique dans le niveau : ItemDefinition déjà connu en BeginPlay
  // Pour les drops ennemis, c'est StartLifetimeTimer() qui applique le mesh (appelé après assignation)
  if (DropLifetime == 0.f && ItemDefinition && ItemDefinition->PickupMesh)
  {
      PickupMesh->SetStaticMesh(ItemDefinition->PickupMesh);
      PickupMesh->SetRelativeScale3D(ItemDefinition->PickupMeshScale);
  }
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

  UStaticMesh* EffectiveDropMesh  = nullptr;
  FVector       EffectiveDropScale = FVector(1.f);
  if (ItemDefinition && ItemDefinition->PickupMesh)
  {
    EffectiveDropMesh  = ItemDefinition->PickupMesh.Get();
    EffectiveDropScale = ItemDefinition->PickupMeshScale;
  }

  if (TypeToAdd == EAmmoType::None) return;

  bool bAdded = Player->AddInventoryAmmo(TypeToAdd, AmmoAmount, EffectiveIcon, EffectiveName,
                                          999, EffectiveDropMesh, EffectiveDropScale);
  if (!bAdded) return; // Inventaire plein

  PendingPlayer = nullptr;
  Player->ClearPendingPickup();
  Player->HidePickupPrompt();

  if (PickupSound)
    UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());

  BP_OnPickedUp(Player, AmmoAmount);

  // Drop ennemi : cherche un autre pickup proche avant de se détruire
  if (DropLifetime > 0.f)
  {
    GetWorldTimerManager().ClearTimer(LifetimeTimer);
    ScanNearbyAmmoPickups(Player);
    Destroy();
  }
  else
    HidePickup();
}

void AAmmoBonusPickup::ScanNearbyAmmoPickups(ARevenantOpsCharacter* Player)
{
  if (!Player || !GetWorld()) return;

  TArray<AActor*> Nearby;
  UGameplayStatics::GetAllActorsOfClass(GetWorld(), AAmmoBonusPickup::StaticClass(), Nearby);

  const float Radius = CollisionSphere->GetScaledSphereRadius() * 1.5f;
  for (AActor* Actor : Nearby)
  {
    AAmmoBonusPickup* Other = Cast<AAmmoBonusPickup>(Actor);
    if (!Other || Other == this || Other->DropLifetime <= 0.f) continue;

    if (FVector::Dist(GetActorLocation(), Other->GetActorLocation()) <= Radius)
    {
      Other->PendingPlayer = Player;
      Player->SetPendingPickup(Other);
      Player->ShowPickupPrompt(
          Other->GetPickupIcon_Implementation(),
          Other->GetPickupDisplayName_Implementation(),
          Other->GetPickupDisplayAmount_Implementation());
      return;
    }
  }
}

void AAmmoBonusPickup::StartLifetimeTimer()
{
  // Applique le mesh ici — appelé APRÈS que ItemDefinition est assigné par le code de spawn
  if (ItemDefinition && ItemDefinition->PickupMesh)
  {
    PickupMesh->SetStaticMesh(ItemDefinition->PickupMesh);
    PickupMesh->SetRelativeScale3D(ItemDefinition->PickupMeshScale);
  }
  else if (!PickupMesh->GetStaticMesh())
  {
    // Fallback : cube Engine si aucun mesh n'est configuré dans le DA
    if (UStaticMesh* Cube = LoadObject<UStaticMesh>(
            nullptr, TEXT("/Engine/BasicShapes/Cube.Cube")))
    {
        PickupMesh->SetStaticMesh(Cube);
        PickupMesh->SetRelativeScale3D(FVector(0.15f));
    }
  }

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
