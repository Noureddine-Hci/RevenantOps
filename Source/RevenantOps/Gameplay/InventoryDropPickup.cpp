// Copyright RevenantOps. All Rights Reserved.
#include "Gameplay/InventoryDropPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "RevenantOpsCharacter.h"
#include "UI/RevenantOpsHUD.h"
#include "Gameplay/PickupInterface.h"
#include "Kismet/GameplayStatics.h"

AInventoryDropPickup::AInventoryDropPickup()
{
    PrimaryActorTick.bCanEverTick = true;

    Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
    Sphere->SetSphereRadius(80.f);
    Sphere->SetCollisionProfileName(TEXT("Trigger"));
    RootComponent = Sphere;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    Mesh->SetupAttachment(RootComponent);
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Mesh->SetVisibility(false);

    SkelMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkelMesh"));
    SkelMesh->SetupAttachment(RootComponent);
    SkelMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SkelMesh->SetVisibility(false);
}

void AInventoryDropPickup::BeginPlay()
{
    Super::BeginPlay();
    InitialZ = GetActorLocation().Z;

    Sphere->OnComponentBeginOverlap.AddDynamic(this, &AInventoryDropPickup::OnOverlapBegin);
    Sphere->OnComponentEndOverlap.AddDynamic(this,   &AInventoryDropPickup::OnOverlapEnd);
}

void AInventoryDropPickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeAlive += DeltaTime;

    // Vie limitée
    if (Lifetime > 0.f && TimeAlive >= Lifetime)
    {
        if (PendingPlayer) PendingPlayer->ClearPendingPickup();
        Destroy();
        return;
    }

    // Bob vertical
    FVector Loc = GetActorLocation();
    Loc.Z = InitialZ + FMath::Sin(TimeAlive * BobSpeed) * BobAmplitude;
    SetActorLocation(Loc);

    // Rotation
    AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
}

void AInventoryDropPickup::Initialize(const FInventoryItem& Item)
{
    HeldItem = Item;

    if (Item.DropSkeletalMesh)
    {
        // Arme → skeletal mesh
        SkelMesh->SetSkeletalMeshAsset(Item.DropSkeletalMesh);
        SkelMesh->SetRelativeScale3D(Item.DropMeshScale);
        SkelMesh->SetVisibility(true);
    }
    else if (Item.DropMesh)
    {
        // Ammo / soin → static mesh depuis DA
        Mesh->SetStaticMesh(Item.DropMesh);
        Mesh->SetRelativeScale3D(Item.DropMeshScale);
        Mesh->SetVisibility(true);
    }
    else
    {
        // Fallback : sphère pour que le drop soit toujours visible
        if (UStaticMesh* FallbackMesh = LoadObject<UStaticMesh>(
                nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere")))
        {
            Mesh->SetStaticMesh(FallbackMesh);
            Mesh->SetRelativeScale3D(FVector(0.3f));
            Mesh->SetVisibility(true);
        }
    }
}

void AInventoryDropPickup::ScanNearbyPickups(ARevenantOpsCharacter* Player)
{
    if (!Player || !GetWorld()) return;

    // Cherche tous les drops dans le rayon de la sphère de collision
    TArray<AActor*> Nearby;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(),
        AInventoryDropPickup::StaticClass(), Nearby);

    const float Radius = Sphere->GetScaledSphereRadius();
    for (AActor* Actor : Nearby)
    {
        AInventoryDropPickup* Other = Cast<AInventoryDropPickup>(Actor);
        if (!Other || Other == this) continue;

        const float Dist = FVector::Dist(GetActorLocation(), Other->GetActorLocation());
        if (Dist <= Radius * 1.5f)
        {
            Other->PendingPlayer = Player;
            Player->SetPendingPickup(Other);
            Player->ShowPickupPrompt(
                Other->HeldItem.ItemIcon,
                Other->HeldItem.DisplayName,
                Other->HeldItem.Quantity);
            return;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────

void AInventoryDropPickup::OnOverlapBegin(UPrimitiveComponent* /*OverlappedComp*/,
                                           AActor* OtherActor,
                                           UPrimitiveComponent* /*OtherComp*/,
                                           int32 /*OtherBodyIndex*/,
                                           bool /*bFromSweep*/,
                                           const FHitResult& /*SweepResult*/)
{
    ARevenantOpsCharacter* Char = Cast<ARevenantOpsCharacter>(OtherActor);
    if (!Char) return;

    PendingPlayer = Char;
    Char->SetPendingPickup(this);
    Char->ShowPickupPrompt(HeldItem.ItemIcon, HeldItem.DisplayName, HeldItem.Quantity);
}

void AInventoryDropPickup::OnOverlapEnd(UPrimitiveComponent* /*OverlappedComp*/,
                                         AActor* OtherActor,
                                         UPrimitiveComponent* /*OtherComp*/,
                                         int32 /*OtherBodyIndex*/)
{
    ARevenantOpsCharacter* Char = Cast<ARevenantOpsCharacter>(OtherActor);
    if (!Char || Char != PendingPlayer) return;

    PendingPlayer = nullptr;
    Char->ClearPendingPickup();
    Char->HidePickupPrompt();
}

// ─────────────────────────────────────────────────────────────────────────────

void AInventoryDropPickup::TryPickupInteract_Implementation(ARevenantOpsCharacter* Player)
{
    if (!Player) return;

    if (Player->AddItemToInventory(HeldItem))
    {
        Player->ClearPendingPickup();
        Player->HidePickupPrompt();
        // Nullifier avant Destroy() — sinon OnOverlapEnd annule le prochain pickup
        ARevenantOpsCharacter* PlayerRef = Player;
        PendingPlayer = nullptr;
        ScanNearbyPickups(PlayerRef);
        Destroy();
    }
    // Si l'inventaire est plein, le prompt reste affiché — rien ne se passe
}
