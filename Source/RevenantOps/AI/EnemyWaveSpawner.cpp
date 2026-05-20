// Copyright RevenantOps. All Rights Reserved.

#include "EnemyWaveSpawner.h"
#include "Components/BoxComponent.h"
#include "EnemyBase.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Weapons/HealthComponent.h"

AEnemyWaveSpawner::AEnemyWaveSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
    RootComponent = TriggerVolume;
    TriggerVolume->SetBoxExtent(FVector(500.f, 500.f, 200.f));
    TriggerVolume->SetCollisionProfileName(TEXT("Trigger"));
}

void AEnemyWaveSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (bTriggerOnOverlap)
    {
        TriggerVolume->OnComponentBeginOverlap.AddDynamic(
            this, &AEnemyWaveSpawner::OnTriggerOverlap);
    }

    if (EnemyPool.IsEmpty())
        SetupDefaultPool();

    if (bAutoStart)
        StartEncounter();
}

// =============================================================================
//  POOL PAR DÉFAUT
// =============================================================================

void AEnemyWaveSpawner::SetupDefaultPool()
{
    static const struct { const TCHAR* Path; float Weight; int32 UnlockAt; } Defaults[] =
    {
        { TEXT("/Game/Mercenaires/Zombies/BP_ZombieSlow.BP_ZombieSlow_C"),       3.f,  0  },
        { TEXT("/Game/Mercenaires/Zombies/BP_ZombieRunner.BP_ZombieRunner_C"),   2.f,  20 },
        { TEXT("/Game/Mercenaires/Zombies/BP_ZombieTank.BP_ZombieTank_C"),       1.f,  50 },
        { TEXT("/Game/Mercenaires/Zombies/BP_ZombieSpitter.BP_ZombieSpitter_C"), 1.5f, 30 },
    };

    for (const auto& D : Defaults)
    {
        if (UClass* C = LoadClass<AEnemyBase>(nullptr, D.Path))
        {
            FEnemyPoolEntry Entry;
            Entry.EnemyClass   = C;
            Entry.Weight       = D.Weight;
            Entry.UnlockAtKills = D.UnlockAt;
            EnemyPool.Add(Entry);
        }
    }

    UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Default pool built: %d types"), EnemyPool.Num());
}

// =============================================================================
//  ENCOUNTER
// =============================================================================

void AEnemyWaveSpawner::StartEncounter()
{
    if (bIsActive) return;

    bIsActive  = true;
    TotalKilled = 0;
    NextMilestoneIndex = 0;
    Milestones.Sort();
    AliveEnemies.Empty();

    // Remplir jusqu'au cap dès le départ
    for (int32 i = AliveEnemies.Num(); i < MaxAliveEnemies; ++i)
        SpawnOneEnemy();

    UE_LOG(LogTemp, Log, TEXT("[WaveSpawner] Encounter started — cap=%d, maxAlive=%d"), KillCap, MaxAliveEnemies);
}

void AEnemyWaveSpawner::StopEncounter()
{
    bIsActive = false;
    GetWorld()->GetTimerManager().ClearTimer(SpawnLoopTimer);
}

// =============================================================================
//  SPAWN
// =============================================================================

void AEnemyWaveSpawner::SpawnOneEnemy()
{
    if (!bIsActive) return;
    if (AliveEnemies.Num() >= MaxAliveEnemies) return;

    TSubclassOf<AEnemyBase> EnemyClass = PickEnemyClass();
    if (!EnemyClass) return;

    FActorSpawnParameters SP;
    SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    AEnemyBase* NewEnemy = GetWorld()->SpawnActor<AEnemyBase>(
        EnemyClass, GetRandomSpawnTransform(), SP);

    if (NewEnemy)
    {
        AliveEnemies.Add(NewEnemy);
        NewEnemy->OnEnemyDied.AddDynamic(this, &AEnemyWaveSpawner::OnEnemyDied);

        // Endgame HP scaling — appliqué AVANT BeginPlay du HealthComp (qui init CurrentHealth)
        const float HpMult = GetCurrentHealthMultiplier();
        if (HpMult > 1.001f)
        {
            if (UHealthComponent* HC = NewEnemy->FindComponentByClass<UHealthComponent>())
            {
                HC->SetMaxHealth(HC->GetMaxHealth() * HpMult);
            }
        }

        if (APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0))
            NewEnemy->AlertToLocation(Player->GetActorLocation());
    }
}

float AEnemyWaveSpawner::GetCurrentHealthMultiplier() const
{
    if (EndgameKillsWindow <= 0 || EndgameHealthMultiplier <= 1.f)
        return 1.f;

    const int32 EndgameStart = FMath::Max(0, KillCap - EndgameKillsWindow);
    if (TotalKilled < EndgameStart) return 1.f;

    const float Range = FMath::Max(1.f, (float)(KillCap - EndgameStart));
    const float Alpha = FMath::Clamp((float)(TotalKilled - EndgameStart) / Range, 0.f, 1.f);
    return FMath::Lerp(1.f, EndgameHealthMultiplier, Alpha);
}

TSubclassOf<AEnemyBase> AEnemyWaveSpawner::PickEnemyClass() const
{
    // Filtrer les entrées éligibles selon le nombre de kills
    float TotalWeight = 0.f;
    for (const FEnemyPoolEntry& Entry : EnemyPool)
    {
        if (!Entry.EnemyClass) continue;
        if (TotalKilled < Entry.UnlockAtKills) continue;
        TotalWeight += FMath::Max(Entry.Weight, 0.01f);
    }

    if (TotalWeight <= 0.f) return nullptr;

    float Roll  = FMath::FRand() * TotalWeight;
    float Cumul = 0.f;

    for (const FEnemyPoolEntry& Entry : EnemyPool)
    {
        if (!Entry.EnemyClass) continue;
        if (TotalKilled < Entry.UnlockAtKills) continue;
        Cumul += FMath::Max(Entry.Weight, 0.01f);
        if (Roll <= Cumul)
            return Entry.EnemyClass;
    }

    return EnemyPool.Last().EnemyClass;
}

float AEnemyWaveSpawner::GetCurrentSpawnDelay() const
{
    // Interpolation linéaire : SpawnDelayInitial → SpawnDelayMinimum sur KillCap kills
    const float Alpha = FMath::Clamp((float)TotalKilled / (float)KillCap, 0.f, 1.f);
    return FMath::Lerp(SpawnDelayInitial, SpawnDelayMinimum, Alpha);
}

FTransform AEnemyWaveSpawner::GetRandomSpawnTransform() const
{
    APawn* Player = UGameplayStatics::GetPlayerPawn(this, 0);
    const FVector PlayerLoc = Player ? Player->GetActorLocation() : GetActorLocation();

    if (SpawnPoints.Num() > 0)
    {
        TArray<int32> Valid;
        for (int32 i = 0; i < SpawnPoints.Num(); ++i)
            if (SpawnPoints[i]) Valid.Add(i);

        if (Valid.Num() > 0)
        {
            const int32 Pick = Valid[FMath::RandRange(0, Valid.Num() - 1)];
            return SpawnPoints[Pick]->GetActorTransform();
        }
    }

    // Fallback : spawn autour du joueur
    if (Player)
    {
        const FVector Fwd   = Player->GetActorForwardVector();
        const FVector Right = Player->GetActorRightVector();
        const FVector Loc   = PlayerLoc
            + Fwd   * FMath::FRandRange(300.f, 500.f)
            + Right * FMath::FRandRange(-300.f, 300.f);
        return FTransform(FRotator::ZeroRotator, FVector(Loc.X, Loc.Y, PlayerLoc.Z));
    }

    return FTransform(FRotator::ZeroRotator, GetActorLocation());
}

// =============================================================================
//  EVENTS
// =============================================================================

void AEnemyWaveSpawner::OnEnemyDied(AEnemyBase* Enemy, AController* /*KilledBy*/)
{
    AliveEnemies.Remove(Enemy);
    ++TotalKilled;

    OnKillCountChanged.Broadcast(TotalKilled);

    // Milestone franchi → annonceur (sons / flash écran via BP)
    while (NextMilestoneIndex < Milestones.Num() &&
           TotalKilled >= Milestones[NextMilestoneIndex])
    {
        const int32 M = Milestones[NextMilestoneIndex];
        OnMilestoneReached.Broadcast(M);
        ++NextMilestoneIndex;
    }

    // Cap atteint → fin du match
    if (TotalKilled >= KillCap)
    {
        StopEncounter();
        OnKillCapReached.Broadcast();
        return;
    }

    // Spawner un remplaçant avec un léger délai (difficulté croissante)
    if (bIsActive)
    {
        const float Delay = GetCurrentSpawnDelay();
        FTimerHandle TmpTimer;
        GetWorld()->GetTimerManager().SetTimer(
            TmpTimer,
            [this]() { SpawnOneEnemy(); },
            Delay, false);
    }
}

void AEnemyWaveSpawner::OnTriggerOverlap(
    UPrimitiveComponent* /*OverlappedComp*/, AActor* OtherActor,
    UPrimitiveComponent* /*OtherComp*/, int32 /*OtherBodyIndex*/,
    bool /*bFromSweep*/, const FHitResult& /*SweepResult*/)
{
    if (OtherActor && OtherActor->ActorHasTag(FName("Player")))
    {
        StartEncounter();
        TriggerVolume->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
}
