// Copyright RevenantOps. All Rights Reserved.

#include "ZombieProjectile.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AZombieProjectile::AZombieProjectile() {
  PrimaryActorTick.bCanEverTick = false;

  // Collision sphere
  CollisionSphere =
      CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
  CollisionSphere->InitSphereRadius(15.f);
  CollisionSphere->SetCollisionProfileName(TEXT("BlockAllDynamic"));
  RootComponent = CollisionSphere;

  // Projectile movement
  ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(
      TEXT("ProjectileMovement"));
  ProjectileMovement->UpdatedComponent = CollisionSphere;
  ProjectileMovement->InitialSpeed = ProjectileSpeed;
  ProjectileMovement->MaxSpeed = ProjectileSpeed;
  ProjectileMovement->bRotationFollowsVelocity = true;
  ProjectileMovement->bShouldBounce = false;
  ProjectileMovement->ProjectileGravityScale = 0.1f;

  // Bind hit event
  CollisionSphere->OnComponentHit.AddDynamic(this,
                                             &AZombieProjectile::OnHit);
}

void AZombieProjectile::BeginPlay() {
  Super::BeginPlay();

  // Auto-destroy after lifetime expires
  SetLifeSpan(ProjectileLifetime);
}

void AZombieProjectile::InitProjectile(float Damage, AActor* InOwner) {
  ProjectileDamage = Damage;
  OwnerActor = InOwner;
  SetOwner(InOwner);
  SetInstigator(Cast<APawn>(InOwner));
}

void AZombieProjectile::OnHit(UPrimitiveComponent* HitComp,
                               AActor* OtherActor,
                               UPrimitiveComponent* OtherComp,
                               FVector NormalImpulse, const FHitResult& Hit) {
  // Don't damage self or owner
  if (!OtherActor || OtherActor == OwnerActor) {
    Destroy();
    return;
  }

  // Don't damage other zombies (friendly fire prevention)
  if (OtherActor->ActorHasTag(FName("Enemy"))) {
    Destroy();
    return;
  }

  // Apply damage to the hit actor
  AController* IC = GetInstigatorController();
  UGameplayStatics::ApplyDamage(OtherActor, ProjectileDamage, IC, this,
                                nullptr);

  // Blueprint hook for VFX/SFX
  BP_OnProjectileHit(Hit);

  Destroy();
}
