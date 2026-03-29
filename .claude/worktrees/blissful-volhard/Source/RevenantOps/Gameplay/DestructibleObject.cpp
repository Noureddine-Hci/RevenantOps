// Copyright RevenantOps. All Rights Reserved.

#include "DestructibleObject.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"

ADestructibleObject::ADestructibleObject() {
  PrimaryActorTick.bCanEverTick = false;

  // Mesh
  ObjectMesh =
      CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectMesh"));
  RootComponent = ObjectMesh;
  ObjectMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

  // Health
  HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
}

void ADestructibleObject::BeginPlay() {
  Super::BeginPlay();

  if (HealthComp) {
    HealthComp->OnDeath.AddDynamic(this, &ADestructibleObject::HandleDeath);
  }
}

void ADestructibleObject::HandleDeath(UHealthComponent *HealthComponent,
                                       const AController *InstigatedBy,
                                       AActor *DamageCauser) {
  // Explosion damage
  if (bExplodesOnDestruction) {
    ApplyExplosionDamage(const_cast<AController *>(InstigatedBy));
  }

  // Spawn loot
  SpawnLoot();

  // Broadcast and BP hook
  OnObjectDestroyed.Broadcast(this);
  BP_OnDestroyed();

  // Destroy the actor
  Destroy();
}

void ADestructibleObject::SpawnLoot() {
  for (const TSubclassOf<AActor> &LootClass : LootDrops) {
    if (!LootClass) {
      continue;
    }

    if (FMath::FRand() <= LootDropChance) {
      FVector SpawnLocation = GetActorLocation();
      SpawnLocation.Z += 50.f; // Drop slightly above
      SpawnLocation.X += FMath::FRandRange(-50.f, 50.f);
      SpawnLocation.Y += FMath::FRandRange(-50.f, 50.f);

      FActorSpawnParameters SpawnParams;
      SpawnParams.SpawnCollisionHandlingOverride =
          ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

      GetWorld()->SpawnActor<AActor>(LootClass, SpawnLocation,
                                     FRotator::ZeroRotator, SpawnParams);
    }
  }
}

void ADestructibleObject::ApplyExplosionDamage(AController *InstigatedBy) {
  UGameplayStatics::ApplyRadialDamage(
      this, ExplosionDamage, GetActorLocation(), ExplosionRadius, nullptr,
      TArray<AActor *>{this}, this, InstigatedBy, true);
}
