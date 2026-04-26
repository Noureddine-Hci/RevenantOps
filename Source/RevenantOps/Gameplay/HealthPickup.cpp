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

    CollisionSphere->OnComponentBeginOverlap.AddDynamic(
        this, &AHealthPickup::OnHealthOverlapBegin);
    CollisionSphere->OnComponentEndOverlap.AddDynamic(
        this, &AHealthPickup::OnHealthOverlapEnd);
}

void AHealthPickup::OnHealthOverlapBegin(
    UPrimitiveComponent*, AActor* OtherActor,
    UPrimitiveComponent*, int32, bool, const FHitResult&)
{
    ARevenantOpsCharacter* Player = Cast<ARevenantOpsCharacter>(OtherActor);
    if (!Player) return;

    // Ne rien proposer si le joueur est déjà à pleine santé
    if (UHealthComponent* HC = Player->FindComponentByClass<UHealthComponent>())
    {
        if (HC->GetHealthPercent() >= 1.f) return;
    }

    PendingPlayer = Player;
    Player->SetPendingHealthPickup(this);

    // Calcule la quantité à afficher dans le popup
    int32 DisplayQty = 0;
    if (UHealthComponent* HC = Player->FindComponentByClass<UHealthComponent>())
        DisplayQty = FMath::RoundToInt(HC->GetMaxHealth() * HealPercent);

    Player->ShowPickupPrompt(PickupIcon, PickupDisplayName, DisplayQty);
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

    if (UHealthComponent* HC = Player->FindComponentByClass<UHealthComponent>())
    {
        if (HC->GetHealthPercent() >= 1.f) return; // déjà plein

        float HealAmount = HC->GetMaxHealth() * HealPercent;
        HC->Heal(HealAmount);
    }

    PendingPlayer = nullptr;
    Player->ClearPendingPickup();
    Player->HidePickupPrompt();

    BP_OnPickedUp(Player);
    HidePickup();
}
