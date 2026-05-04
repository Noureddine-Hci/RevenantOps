// Copyright RevenantOps. All Rights Reserved.

#include "WeaponPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RevenantOpsCharacter.h"
#include "Weapons/WeaponBase.h"
#include "Kismet/GameplayStatics.h"

AWeaponPickup::AWeaponPickup()
{
    PrimaryActorTick.bCanEverTick = true;

    static ConstructorHelpers::FObjectFinder<USoundBase> DefaultSound(
        TEXT("/Game/Mercenaires/Audio/SFX/SW_Hit"));
    if (DefaultSound.Succeeded()) PickupSound = DefaultSound.Object;

    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
    RootComponent = PickupMesh;
    PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(RootComponent);
    CollisionSphere->SetSphereRadius(100.f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AWeaponPickup::BeginPlay()
{
    Super::BeginPlay();
    InitialZ = GetActorLocation().Z;
    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponPickup::OnOverlapBegin);
    CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponPickup::OnOverlapEnd);
}

void AWeaponPickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bBobbing) {
        FVector Loc = GetActorLocation();
        Loc.Z = InitialZ + FMath::Sin(GetGameTimeSinceCreation() * 2.f) * BobAmplitude;
        SetActorLocation(Loc);
    }

    if (bRotating) {
        AddActorWorldRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
    }
}

void AWeaponPickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    ARevenantOpsCharacter* Player = Cast<ARevenantOpsCharacter>(OtherActor);
    if (!Player) return;

    PendingPlayer = Player;
    Player->SetPendingWeaponPickup(this);
    Player->ShowPickupPrompt(WeaponIcon, WeaponDisplayName, 1);
}

void AWeaponPickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ARevenantOpsCharacter* Player = Cast<ARevenantOpsCharacter>(OtherActor);
    if (!Player || Player != PendingPlayer) return;

    PendingPlayer = nullptr;
    Player->SetPendingWeaponPickup(nullptr);
    Player->HidePickupPrompt();
}

void AWeaponPickup::TryPickup(ARevenantOpsCharacter* Player)
{
    if (!Player || !WeaponClass) return;

    // Spawn l'arme et l'ajoute à l'inventaire
    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner      = Player;
    SpawnParams.Instigator = Player;

    AWeaponBase* NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(WeaponClass, SpawnParams);
    if (!NewWeapon) return;

    Player->AddAndEquipWeapon(NewWeapon);

    // Ajouter dans l'inventaire RE5
    FInventoryItem InvItem;
    InvItem.Type        = EInventoryItemType::Weapon;
    InvItem.DisplayName = WeaponDisplayName;
    InvItem.WeaponClass = WeaponClass;
    InvItem.ItemIcon    = WeaponIcon;
    InvItem.Quantity    = 1;
    Player->AddItemToInventory(InvItem);

    PendingPlayer = nullptr;
    Player->SetPendingWeaponPickup(nullptr);
    Player->HidePickupPrompt();

    if (PickupSound)
        UGameplayStatics::PlaySoundAtLocation(this, PickupSound, GetActorLocation());

    BP_OnPickedUp(Player);
    HidePickup();
}

void AWeaponPickup::HidePickup()
{
    SetActorHiddenInGame(true);
    SetActorEnableCollision(false);
    SetActorTickEnabled(false);
    Destroy();
}
