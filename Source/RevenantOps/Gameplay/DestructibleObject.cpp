// Copyright RevenantOps. All Rights Reserved.

#include "Gameplay/DestructibleObject.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DamageEvents.h"
#include "Engine/StaticMeshActor.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RevenantOpsCharacter.h"
#include "WeaponBase.h"
#include "Gameplay/AmmoBonusPickup.h"
#include "Gameplay/HealthPickup.h"

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
    UE_LOG(LogTemp, Warning, TEXT("DestructibleObject: HandleDeath appelé, AmmoPickupClasses=%d, HealthFallback=%s"),
        AmmoPickupClasses.Num(),
        HealthFallbackClass ? *HealthFallbackClass->GetName() : TEXT("NULL"));

    if (bExplodesOnDestruction)
        ApplyExplosionDamage(const_cast<AController*>(InstigatedBy));

    SpawnLoot();
    SpawnDebris();

    OnObjectDestroyed.Broadcast(this);
    BP_OnDestroyed();

    // Cacher et désactiver le mesh principal immédiatement
    if (ObjectMesh)
    {
        ObjectMesh->SetHiddenInGame(true, true);
        ObjectMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // Destruction différée pour laisser le temps aux fragments / debris de bouger
    if (DestroyDelay > 0.f)
        SetLifeSpan(DestroyDelay);
    else
        Destroy();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Spawn du loot — chaque entrée est tirée indépendamment
// ─────────────────────────────────────────────────────────────────────────────
void ADestructibleObject::SpawnLoot()
{
    UE_LOG(LogTemp, Warning, TEXT("SpawnLoot: LootTable=%d AmmoClasses=%d HealthDrops=%d FallbackClass=%s"),
        LootTable.Num(), AmmoPickupClasses.Num(), HealthDrops.Num(),
        HealthFallbackClass ? *HealthFallbackClass->GetName() : TEXT("NULL"));
    if (LootTable.IsEmpty() && AmmoPickupClasses.IsEmpty() && !HealthFallbackClass && HealthDrops.IsEmpty()) return;

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
            20.f);
        AActor* Spawned = GetWorld()->SpawnActor<AActor>(Entry.ActorClass, SpawnLoc,
                                                          FRotator::ZeroRotator, SP);
        // Appliquer le mesh depuis le DA si c'est un ammo pickup
        if (AAmmoBonusPickup* AmmoPU = Cast<AAmmoBonusPickup>(Spawned))
            AmmoPU->StartLifetimeTimer();
    };

    // Helper : tire un soin au hasard dans HealthFallbackClasses
    auto SpawnRandomHealth = [&]()
    {
        // Nouveau système : HealthDrops avec DA intégré
        UE_LOG(LogTemp, Warning, TEXT("SpawnRandomHealth: HealthDrops=%d"), HealthDrops.Num());
        if (!HealthDrops.IsEmpty())
        {
            TArray<const FHealthDropEntry*> Valid;
            for (const FHealthDropEntry& E : HealthDrops)
                if (E.PickupClass) Valid.Add(&E);
            UE_LOG(LogTemp, Warning, TEXT("SpawnRandomHealth: Valid=%d"), Valid.Num());
            if (Valid.IsEmpty()) return;

            const FHealthDropEntry* Chosen = Valid[FMath::RandRange(0, Valid.Num() - 1)];
            FCrateLootEntry HealthEntry;
            HealthEntry.ActorClass = Chosen->PickupClass;
            SpawnEntry(HealthEntry);
            return;
        }
        // Fallback ancien système
        if (HealthFallbackClass)
        {
            FCrateLootEntry FallbackEntry;
            FallbackEntry.ActorClass = HealthFallbackClass;
            SpawnEntry(FallbackEntry);
        }
    };

    // ── Drop automatique selon armes du joueur ────────────────────────────
    if (!AmmoPickupClasses.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("SpawnLoot: PlayerAmmoTypes=%d"), PlayerAmmoTypes.Num());
        // Collecter les paires (classe, DA) valides puis en choisir une au hasard
        TArray<TPair<TSubclassOf<AActor>, UItemDefinition*>> ValidPairs;
        for (EAmmoType AmmoType : PlayerAmmoTypes)
        {
            TSubclassOf<AActor>* PickupClass = AmmoPickupClasses.Find(AmmoType);
            if (!PickupClass || !*PickupClass) continue;
            UItemDefinition* DA = nullptr;
            if (TObjectPtr<UItemDefinition>* DAPtr = AmmoItemDefinitions.Find(AmmoType))
                DA = DAPtr->Get();
            ValidPairs.Add(TPair<TSubclassOf<AActor>, UItemDefinition*>(*PickupClass, DA));
        }
        bool bAnyAmmoDropped = false;
        if (!ValidPairs.IsEmpty())
        {
            auto& Chosen = ValidPairs[FMath::RandRange(0, ValidPairs.Num() - 1)];
            FCrateLootEntry AutoEntry;
            AutoEntry.ActorClass = Chosen.Key;
            // Spawn puis assigner le DA
            FVector SpawnLoc = GetActorLocation() + FVector(
                FMath::RandRange(-LootScatterRadius, LootScatterRadius),
                FMath::RandRange(-LootScatterRadius, LootScatterRadius),
                20.f);
            FActorSpawnParameters SP2;
            SP2.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
            AActor* Spawned = GetWorld()->SpawnActor<AActor>(Chosen.Key, SpawnLoc, FRotator::ZeroRotator, SP2);
            if (AAmmoBonusPickup* AmmoPU = Cast<AAmmoBonusPickup>(Spawned))
            {
                if (Chosen.Value)
                {
                    AmmoPU->ItemDefinition = Chosen.Value;
                    if (Chosen.Value->DefaultDropAmount > 0)
                        AmmoPU->AmmoAmount = Chosen.Value->DefaultDropAmount;
                }
                AmmoPU->StartLifetimeTimer();
            }
            bAnyAmmoDropped = true;
        }
        // Fallback soin si aucune munition matchée
        if (!bAnyAmmoDropped)
            SpawnRandomHealth();
        return;
    }

    // ── Mode Independent : chaque entree tiree separement (LootTable manuel) ─
    if (LootMode == ELootMode::Independent)
    {
        bool bAnyAmmoDropped = false;
        for (const FCrateLootEntry& Entry : LootTable)
        {
            if (!Entry.ActorClass) continue;
            if (Entry.AmmoTypeFilter != EAmmoType::None &&
                !PlayerAmmoTypes.Contains(Entry.AmmoTypeFilter)) continue;
            if (!Entry.bGuaranteed && FMath::FRand() > Entry.DropChance) continue;
            SpawnEntry(Entry);
            if (Entry.AmmoTypeFilter != EAmmoType::None)
                bAnyAmmoDropped = true;
        }
        if (!bAnyAmmoDropped)
            SpawnRandomHealth();
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

    if (TotalWeight <= 0.f) { SpawnRandomHealth(); return; }

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


// ─────────────────────────────────────────────────────────────────────────────
//  Spawn de fragments physiques avec impulsion radiale
// ─────────────────────────────────────────────────────────────────────────────
void ADestructibleObject::SpawnDebris()
{
    if (DebrisMeshes.IsEmpty() || DebrisCount <= 0) return;

    const FVector Center = GetActorLocation();

    FActorSpawnParameters SP;
    SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    for (int32 i = 0; i < DebrisCount; ++i)
    {
        UStaticMesh* Mesh = DebrisMeshes[FMath::RandRange(0, DebrisMeshes.Num() - 1)];
        if (!Mesh) continue;

        const FVector Offset(
            FMath::FRandRange(-30.f, 30.f),
            FMath::FRandRange(-30.f, 30.f),
            FMath::FRandRange(0.f, 40.f));
        const FRotator RandRot(
            FMath::FRandRange(0.f, 360.f),
            FMath::FRandRange(0.f, 360.f),
            FMath::FRandRange(0.f, 360.f));

        AStaticMeshActor* Debris = GetWorld()->SpawnActor<AStaticMeshActor>(
            AStaticMeshActor::StaticClass(), Center + Offset, RandRot, SP);
        if (!Debris) continue;

        Debris->SetMobility(EComponentMobility::Movable);
        if (UStaticMeshComponent* SMC = Debris->GetStaticMeshComponent())
        {
            SMC->SetStaticMesh(Mesh);
            SMC->SetRelativeScale3D(FVector(DebrisScale));
            SMC->SetSimulatePhysics(true);
            SMC->SetCollisionProfileName(TEXT("PhysicsActor"));
            // Ignorer le pawn — les débris ne bloquent pas le joueur
            SMC->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

            // Impulsion radiale depuis le centre de la caisse
            const FVector Impulse = (Offset.GetSafeNormal() + FVector(0.f, 0.f, 0.5f))
                                    .GetSafeNormal() * DebrisImpulseStrength;
            SMC->AddImpulse(Impulse, NAME_None, true);
            SMC->AddAngularImpulseInDegrees(
                FVector(FMath::FRandRange(-360.f, 360.f),
                        FMath::FRandRange(-360.f, 360.f),
                        FMath::FRandRange(-360.f, 360.f)),
                NAME_None, true);
        }

        Debris->SetLifeSpan(DebrisLifetime);
    }
}
