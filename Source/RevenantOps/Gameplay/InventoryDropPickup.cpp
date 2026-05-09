// Copyright RevenantOps. All Rights Reserved.
#include "Gameplay/InventoryDropPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "RevenantOpsCharacter.h"
#include "UI/RevenantOpsHUD.h"
#include "Gameplay/PickupInterface.h"

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
    // Aucun mesh par défaut — assigner dans le BP enfant BP_InventoryDropPickup
    // (ou laisser invisible si pas de BP)
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
        Destroy();
    }
    // Si l'inventaire est plein, le prompt reste affiché — rien ne se passe
}
