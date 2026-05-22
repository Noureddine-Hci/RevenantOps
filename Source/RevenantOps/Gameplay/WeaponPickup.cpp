// Copyright RevenantOps. All Rights Reserved.

#include "WeaponPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
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

    PickupSkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PickupSkeletalMesh"));
    PickupSkeletalMeshComp->SetupAttachment(RootComponent);
    PickupSkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PickupSkeletalMeshComp->SetHiddenInGame(true); // caché par défaut

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    CollisionSphere->SetupAttachment(RootComponent);
    CollisionSphere->SetSphereRadius(100.f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
}

void AWeaponPickup::BeginPlay()
{
    Super::BeginPlay();
    InitialZ = GetActorLocation().Z;

    // Appliquer les données depuis le DA si défini
    if (ItemDefinition)
    {
        if (ItemDefinition->WeaponClass)            WeaponClass       = ItemDefinition->WeaponClass;
        if (ItemDefinition->ItemIcon)               WeaponIcon        = ItemDefinition->ItemIcon.Get();
        if (!ItemDefinition->DisplayName.IsEmpty()) WeaponDisplayName = ItemDefinition->DisplayName;

        if (ItemDefinition->PickupSkeletalMesh)
        {
            // Skeletal mesh — cache le static, montre le skeletal
            PickupMesh->SetHiddenInGame(true);
            PickupSkeletalMeshComp->SetSkeletalMesh(ItemDefinition->PickupSkeletalMesh);
            PickupSkeletalMeshComp->SetRelativeScale3D(ItemDefinition->PickupMeshScale);
            PickupSkeletalMeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
            PickupSkeletalMeshComp->SetAnimation(nullptr); // pas d'animation
            PickupSkeletalMeshComp->SetSimulatePhysics(false);
            PickupSkeletalMeshComp->SetHiddenInGame(false);
        }
        else if (ItemDefinition->PickupMesh)
        {
            PickupMesh->SetStaticMesh(ItemDefinition->PickupMesh);
            PickupMesh->SetRelativeScale3D(ItemDefinition->PickupMeshScale);
        }
    }

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponPickup::OnOverlapBegin);
    CollisionSphere->OnComponentEndOverlap.AddDynamic(this, &AWeaponPickup::OnOverlapEnd);
    CollisionSphere->UpdateOverlaps();
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
    UE_LOG(LogTemp, Warning, TEXT("WeaponPickup::OnOverlapBegin — OtherActor=%s, Player=%s"),
        *GetNameSafe(OtherActor), *GetNameSafe(Player));
    if (!Player) return;

    PendingPlayer = Player;
    Player->SetPendingPickup(this);
    UTexture2D* Icon = (ItemDefinition && ItemDefinition->ItemIcon) ? ItemDefinition->ItemIcon.Get() : WeaponIcon;
    FText Name = (ItemDefinition && !ItemDefinition->DisplayName.IsEmpty()) ? ItemDefinition->DisplayName : WeaponDisplayName;
    Player->ShowPickupPrompt(Icon, Name, 1);
}

void AWeaponPickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    ARevenantOpsCharacter* Player = Cast<ARevenantOpsCharacter>(OtherActor);
    if (!Player || Player != PendingPlayer) return;

    PendingPlayer = nullptr;
    Player->ClearPendingPickup();
    Player->HidePickupPrompt();
}

void AWeaponPickup::TryPickup(ARevenantOpsCharacter* Player)
{
    UE_LOG(LogTemp, Warning, TEXT("WeaponPickup::TryPickup — Player=%s WeaponClass=%s ItemDef=%s"),
        *GetNameSafe(Player), *GetNameSafe(WeaponClass), *GetNameSafe(ItemDefinition));
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
