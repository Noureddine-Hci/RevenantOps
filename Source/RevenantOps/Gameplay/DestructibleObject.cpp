// Copyright RevenantOps. All Rights Reserved.

#include "Gameplay/DestructibleObject.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RevenantOpsCharacter.h"
#include "WeaponBase.h"

ADestructibleObject::ADestructibleObject()
{
    PrimaryActorTick.bCanEverTick = false;

    ObjectMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ObjectMesh"));
    RootComponent = ObjectMesh;
    ObjectMesh->SetCollisionProfileName(TEXT("BlockAllDynamic"));

    HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComp"));
}

void ADestructibleObject::BeginPlay()
{
    Super::BeginPlay();

    if (HealthComp)
    {
        HealthComp->OnDeath.AddDynamic(this, &ADestructibleObject::HandleDeath);
        HealthComp->OnHealthChanged.AddDynamic(this, &ADestructibleObject::HandleDamage);
    }

    // Appliquer le matériau normal si renseigné
    if (NormalMaterial)
        ObjectMesh->SetMaterial(0, NormalMaterial);
}

// ─────────────────────────────────────────────────────────────────────────────
//  Réaction aux dégâts : passage en état "endommagé"
// ─────────────────────────────────────────────────────────────────────────────
void ADestructibleObject::HandleDamage(UHealthComponent* /*HealthComponent*/,
                                        float Health, float /*HealthDelta*/,
                                        const AController* /*InstigatedBy*/)
{
    if (bDamagedMaterialApplied) return;

    float HealthPct = (HealthComp->GetMaxHealth() > 0.f)
                      ? (Health / HealthComp->GetMaxHealth() * 100.f)
                      : 100.f;

    if (HealthPct < DamagedThreshold)
    {
        bDamagedMaterialApplied = true;

        if (DamagedMaterial)
            ObjectMesh->SetMaterial(0, DamagedMaterial);

        BP_OnDamagedState();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Mort : explosion optionnelle → loot → destruction
// ─────────────────────────────────────────────────────────────────────────────
void ADestructibleObject::HandleDeath(UHealthComponent* /*HealthComponent*/,
                                       const AController* InstigatedBy,
                                       AActor* /*DamageCauser*/)
{
    if (bExplodesOnDestruction)
        ApplyExplosionDamage(const_cast<AController*>(InstigatedBy));

    SpawnLoot();

    OnObjectDestroyed.Broadcast(this);
    BP_OnDestroyed();

    Destroy();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Spawn du loot — chaque entrée est tirée indépendamment
// ─────────────────────────────────────────────────────────────────────────────
void ADestructibleObject::SpawnLoot()
{
    if (LootTable.IsEmpty()) return;

    // Collecter les types d'armes du joueur (filtre adaptatif munitions)
    TSet<EAmmoType> PlayerAmmoTypes;
    if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
    {
        if (ARevenantOpsCharacter* MercChar = Cast<ARevenantOpsCharacter>(PC->GetPawn()))
        {
            for (AWeaponBase* Wpn : MercChar->GetWeaponInventory())
            {
                if (Wpn) PlayerAmmoTypes.Add(Wpn->GetWeaponAmmoType());
            }
        }
    }

    FActorSpawnParameters SP;
    SP.SpawnCollisionHandlingOverride =
        ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    auto SpawnEntry = [&](const FCrateLootEntry& Entry)
    {
        if (!Entry.ActorClass) return;
        FVector SpawnLoc = GetActorLocation() + FVector(
            FMath::RandRange(-LootScatterRadius, LootScatterRadius),
            FMath::RandRange(-LootScatterRadius, LootScatterRadius),
            40.f);
        GetWorld()->SpawnActor<AActor>(Entry.ActorClass, SpawnLoc,
                                       FRotator::ZeroRotator, SP);
    };

    // ── Mode Independent : chaque entree tiree separement ─────────────────
    if (LootMode == ELootMode::Independent)
    {
        for (const FCrateLootEntry& Entry : LootTable)
        {
            if (!Entry.ActorClass) continue;
            if (Entry.AmmoTypeFilter != EAmmoType::None &&
                !PlayerAmmoTypes.Contains(Entry.AmmoTypeFilter)) continue;
            if (!Entry.bGuaranteed && FMath::FRand() > Entry.DropChance) continue;
            SpawnEntry(Entry);
        }
        return;
    }

    // ── Mode PickOne : 1 seule entree selectionnee par tirage pondere ──────
    // Calculer la somme totale des poids des entrees eligibles
    float TotalWeight = 0.f;
    for (const FCrateLootEntry& Entry : LootTable)
    {
        if (!Entry.ActorClass) continue;
        if (Entry.AmmoTypeFilter != EAmmoType::None &&
            !PlayerAmmoTypes.Contains(Entry.AmmoTypeFilter)) continue;
        TotalWeight += FMath::Max(Entry.Weight, 0.01f);
    }

    if (TotalWeight <= 0.f) return;

    // Tirer un nombre aleatoire dans [0, TotalWeight[
    float Roll = FMath::FRand() * TotalWeight;
    float Cumul = 0.f;

    for (const FCrateLootEntry& Entry : LootTable)
    {
        if (!Entry.ActorClass) continue;
        if (Entry.AmmoTypeFilter != EAmmoType::None &&
            !PlayerAmmoTypes.Contains(Entry.AmmoTypeFilter)) continue;

        Cumul += FMath::Max(Entry.Weight, 0.01f);
        if (Roll <= Cumul)
        {
            SpawnEntry(Entry);
            return; // exactement 1 drop
        }
    }

    // Securite : spawner le dernier eligible si le Roll arrive en fin de liste
    for (int32 i = LootTable.Num() - 1; i >= 0; --i)
    {
        if (LootTable[i].ActorClass)
        {
            SpawnEntry(LootTable[i]);
            return;
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Dégâts d'explosion en rayon
// ─────────────────────────────────────────────────────────────────────────────
void ADestructibleObject::ApplyExplosionDamage(AController* InstigatedBy)
{
    UGameplayStatics::ApplyRadialDamage(
        this, ExplosionDamage, GetActorLocation(), ExplosionRadius,
        nullptr, TArray<AActor*>{this}, this, InstigatedBy, true);
}
