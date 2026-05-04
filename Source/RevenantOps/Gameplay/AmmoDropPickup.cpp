// Copyright RevenantOps. All Rights Reserved.
#include "Gameplay/AmmoDropPickup.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "RevenantOpsCharacter.h"
#include "WeaponBase.h"

AAmmoDropPickup::AAmmoDropPickup()
{
    PrimaryActorTick.bCanEverTick = true;

    CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
    SetRootComponent(CollisionSphere);
    CollisionSphere->SetSphereRadius(60.f);
    CollisionSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

    PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
    PickupMesh->SetupAttachment(RootComponent);
    PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PickupMesh->SetCastShadow(false);

    // Mesh par défaut : cube moteur
    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(
        TEXT("/Engine/BasicShapes/Cube.Cube"));
    if (CubeMesh.Succeeded())
    {
        PickupMesh->SetStaticMesh(CubeMesh.Object);
        PickupMesh->SetRelativeScale3D(FVector(0.2f)); // petit cube
    }
}

void AAmmoDropPickup::BeginPlay()
{
    Super::BeginPlay();

    InitialZ = GetActorLocation().Z;

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(
        this, &AAmmoDropPickup::OnOverlapBegin);

    // Couleur du mesh selon le type de munitions
    if (UMaterialInstanceDynamic* MID = PickupMesh->CreateAndSetMaterialInstanceDynamic(0))
    {
        FLinearColor TypeColor;
        switch (AmmoType)
        {
            case EAmmoType::Pistol:  TypeColor = FLinearColor(1.f,  0.85f, 0.1f,  1.f); break; // jaune
            case EAmmoType::Rifle:   TypeColor = FLinearColor(0.1f, 0.6f,  1.f,   1.f); break; // bleu
            case EAmmoType::Shotgun: TypeColor = FLinearColor(1.f,  0.3f,  0.1f,  1.f); break; // rouge
            case EAmmoType::SMG:     TypeColor = FLinearColor(0.1f, 1.f,   0.4f,  1.f); break; // vert
            case EAmmoType::Sniper:  TypeColor = FLinearColor(0.8f, 0.1f,  1.f,   1.f); break; // violet
            default:                 TypeColor = FLinearColor(0.8f, 0.8f,  0.8f,  1.f); break; // gris
        }
        MID->SetVectorParameterValue(TEXT("BaseColor"), TypeColor);
    }
}

void AAmmoDropPickup::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    TimeAlive += DeltaTime;

    // Disparition après Lifetime secondes
    if (TimeAlive >= Lifetime)
    {
        Destroy();
        return;
    }

    // Clignote dans les 3 dernières secondes
    if (TimeAlive > Lifetime - 3.f)
    {
        float Blink = FMath::Fmod(TimeAlive * 6.f, 1.f) > 0.5f ? 1.f : 0.f;
        PickupMesh->SetVisibility(Blink > 0.5f);
    }

    // Bob vertical
    FVector Loc = GetActorLocation();
    Loc.Z = InitialZ + FMath::Sin(TimeAlive * BobSpeed) * BobAmplitude;
    SetActorLocation(Loc);

    // Rotation
    AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaTime, 0.f));
}

void AAmmoDropPickup::OnOverlapBegin(UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    ARevenantOpsCharacter* Player = Cast<ARevenantOpsCharacter>(OtherActor);
    if (!Player) return;

    // Ajoute les munitions à l'arme active si elle correspond au type
    bool bPickedUp = false;
    if (AWeaponBase* Wpn = Player->GetCurrentWeapon())
    {
        if (Wpn->GetWeaponAmmoType() == AmmoType)
        {
            Wpn->AddReserveAmmo(AmmoAmount);
            bPickedUp = true;
        }
    }

    if (bPickedUp)
        Destroy();
}
