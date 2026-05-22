// Copyright RevenantOps. All Rights Reserved.

#include "Gameplay/HealthPickup.h"
#include "Components/SphereComponent.h"
#include "RevenantOpsCharacter.h"
#include "HealthComponent.h"

AHealthPickup::AHealthPickup()
{
    // Par défaut : type Health (hérité de PickupBase), pas d'auto-pickup
    PickupType = EPickupType::Health;
    RespawnTime = 30.f;
}

void AHealthPickup::BeginPlay()
{
    // On NE appelle PAS Super::BeginPlay() pour le bind overlap —
    // PickupBase bind son propre OnOverlapBegin (auto-pickup).
    // On rebind avec notre version manuelle à la place.
    AActor::BeginPlay(); // Sauter APickupBase::BeginPlay intentionnellement

    InitialZ = GetActorLocation().Z;

    // Applique le mesh 3D depuis le DA si défini
    if (ItemDefinition && ItemDefinition->PickupMesh && PickupMesh)
    {
        PickupMesh->SetStaticMesh(ItemDefinition->PickupMesh);
        PickupMesh->SetRelativeScale3D(ItemDefinition->PickupMeshScale);
    }

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(
        this, &AHealthPickup::OnHealthOverlapBegin);
    CollisionSphere->OnComponentEndOverlap.AddDynamic(
        this, &AHealthPickup::OnHealthOverlapEnd);

    // Force la détection d'overlaps existants au spawn
    CollisionSphere->UpdateOverlaps();
}

void AHealthPickup::OnHealthOverlapBegin(
    UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    ARevenantOpsCharacter* Player = Cast<ARevenantOpsCharacter>(OtherActor);
    if (!Player) return;

    // Bloquer soin immédiat si pleine santé (mais laisser passer si ItemDefinition = inventaire)
    if (!ItemDefinition)
    {
        if (UHealthComponent* HC = Player->FindComponentByClass<UHealthComponent>())
            if (HC->GetHealthPercent() >= 1.f) return;
    }

    UE_LOG(LogTemp, Warning, TEXT("HealthPickup: overlap avec %s"), *Player->GetName());
    PendingPlayer = Player;
    Player->SetPendingHealthPickup(this);

    // Calcule la quantité à afficher dans le popup
    int32 DisplayQty = 0;
    if (UHealthComponent* HC = Player->FindComponentByClass<UHealthComponent>())
        DisplayQty = FMath::RoundToInt(HC->GetMaxHealth() * HealPercent);

    UTexture2D* Icon = (ItemDefinition && ItemDefinition->ItemIcon) ? ItemDefinition->ItemIcon.Get() : PickupIcon.Get();
    FText Name = (ItemDefinition && !ItemDefinition->DisplayName.IsEmpty()) ? ItemDefinition->DisplayName : PickupDisplayName;
    Player->ShowPickupPrompt(Icon, Name, DisplayQty);
}

void AHealthPickup::OnHealthOverlapEnd(
    UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32)
{
    ARevenantOpsCharacter* Player = Cast<ARevenantOpsCharacter>(OtherActor);
    if (!Player || Player != PendingPlayer) return;

    PendingPlayer = nullptr;
    Player->ClearPendingPickup();
    Player->HidePickupPrompt();
}

int32 AHealthPickup::GetPickupDisplayAmount_Implementation() const
{
    // Calcule les HP que ce pickup donnerait (sans référence au joueur ici)
    // On affiche le % directement : 25, 50 ou 100
    return FMath::RoundToInt(HealPercent * 100.f);
}

void AHealthPickup::TryPickupInteract_Implementation(ARevenantOpsCharacter* Player)
{
    TryPickup(Player);
}

void AHealthPickup::TryPickup(ARevenantOpsCharacter* Player)
{
    if (!Player) return;

    UHealthComponent* HC = Player->FindComponentByClass<UHealthComponent>();

    // ── Chemin RE5 : soin dans l'inventaire ───────────────────────────────
    if (ItemDefinition && ItemDefinition->ItemType == EInventoryItemType::Health)
    {
        // Calcule le HealAmount si le DA laisse 0 (on calcule depuis HealPercent)
        FInventoryItem NewItem = ItemDefinition->MakeInventoryItem(1);
        if (NewItem.HealAmount <= 0.f && HC)
            NewItem.HealAmount = HC->GetMaxHealth() * HealPercent;

        if (Player->AddItemToInventory(NewItem))
        {
            PendingPlayer = nullptr;
            Player->ClearPendingPickup();
            Player->HidePickupPrompt();
            BP_OnPickedUp(Player);
            HidePickup();
        }
        // Inventaire plein → on ne ramasse pas
        return;
    }

    // Pas d'ItemDefinition → soin immédiat classique (rétrocompat pickups statiques)
    if (HC)
    {
        if (HC->GetHealthPercent() >= 1.f) return;
        HC->Heal(HC->GetMaxHealth() * HealPercent);
    }

    PendingPlayer = nullptr;
    Player->ClearPendingPickup();
    Player->HidePickupPrompt();

    BP_OnPickedUp(Player);
    HidePickup();
}
