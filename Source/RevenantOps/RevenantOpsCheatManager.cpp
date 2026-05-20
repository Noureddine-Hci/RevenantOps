// Copyright RevenantOps. All Rights Reserved.

#include "RevenantOpsCheatManager.h"
#include "RevenantOpsCharacter.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/HealthComponent.h"
#include "AI/EnemyBase.h"
#include "AI/EnemyWaveSpawner.h"
#include "Gameplay/MercenairesGameState.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    ARevenantOpsCharacter* GetPlayerChar(UWorld* World)
    {
        if (!World) return nullptr;
        APawn* P = UGameplayStatics::GetPlayerPawn(World, 0);
        return Cast<ARevenantOpsCharacter>(P);
    }

    AEnemyWaveSpawner* FindSpawner(UWorld* World)
    {
        if (!World) return nullptr;
        TArray<AActor*> Found;
        UGameplayStatics::GetAllActorsOfClass(World, AEnemyWaveSpawner::StaticClass(), Found);
        return Found.Num() > 0 ? Cast<AEnemyWaveSpawner>(Found[0]) : nullptr;
    }
}

void URevenantOpsCheatManager::GodMode(bool bEnabled)
{
    ARevenantOpsCharacter* PC = GetPlayerChar(GetWorld());
    if (!PC) return;

    if (UHealthComponent* HC = PC->FindComponentByClass<UHealthComponent>())
    {
        HC->DamageMultiplier = bEnabled ? 0.f : 1.f;
        UE_LOG(LogTemp, Log, TEXT("[Cheat] GodMode = %d"), bEnabled);
    }
}

void URevenantOpsCheatManager::KillAll()
{
    UWorld* World = GetWorld();
    if (!World) return;

    TArray<AActor*> Enemies;
    UGameplayStatics::GetAllActorsOfClass(World, AEnemyBase::StaticClass(), Enemies);

    for (AActor* A : Enemies)
    {
        if (A) UGameplayStatics::ApplyDamage(A, 9999.f, nullptr, nullptr, nullptr);
    }
    UE_LOG(LogTemp, Log, TEXT("[Cheat] KillAll → %d ennemis"), Enemies.Num());
}

void URevenantOpsCheatManager::GiveAmmo(int32 Amount)
{
    ARevenantOpsCharacter* PC = GetPlayerChar(GetWorld());
    if (!PC) return;

    if (AWeaponBase* W = PC->GetCurrentWeapon())
    {
        W->AddReserveAmmo(Amount);
        UE_LOG(LogTemp, Log, TEXT("[Cheat] +%d munitions"), Amount);
    }
}

void URevenantOpsCheatManager::SetKills(int32 NewCount)
{
    AEnemyWaveSpawner* Spawner = FindSpawner(GetWorld());
    if (!Spawner)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Cheat] Pas de spawner trouvé"));
        return;
    }

    // Simule des morts pour atteindre NewCount via le delegate
    const int32 Diff = NewCount - Spawner->GetTotalKilled();
    if (Diff <= 0) return;

    for (int32 i = 0; i < Diff; ++i)
    {
        // Trigger un kill virtuel (broadcast delegate)
        Spawner->OnKillCountChanged.Broadcast(Spawner->GetTotalKilled() + i + 1);
    }
    UE_LOG(LogTemp, Log, TEXT("[Cheat] SetKills → %d"), NewCount);
}

void URevenantOpsCheatManager::AddTime(float Seconds)
{
    if (UWorld* World = GetWorld())
    {
        if (AMercenairesGameState* GS = World->GetGameState<AMercenairesGameState>())
        {
            GS->AddBonusTime(Seconds);
            UE_LOG(LogTemp, Log, TEXT("[Cheat] +%.1fs au timer"), Seconds);
        }
    }
}

void URevenantOpsCheatManager::HealFull()
{
    ARevenantOpsCharacter* PC = GetPlayerChar(GetWorld());
    if (!PC) return;

    if (UHealthComponent* HC = PC->FindComponentByClass<UHealthComponent>())
    {
        HC->ResetHealth();
        UE_LOG(LogTemp, Log, TEXT("[Cheat] Heal full"));
    }
}

void URevenantOpsCheatManager::SetCombo(int32 /*Multiplier*/)
{
    // Pas d'API publique pour forcer le combo — laisse en placeholder
    UE_LOG(LogTemp, Warning, TEXT("[Cheat] SetCombo : pas encore implémenté côté GameState"));
}

void URevenantOpsCheatManager::EndMatch()
{
    if (UWorld* World = GetWorld())
    {
        if (AMercenairesGameState* GS = World->GetGameState<AMercenairesGameState>())
        {
            GS->EndMatch();
            UE_LOG(LogTemp, Log, TEXT("[Cheat] Match terminé"));
        }
    }
}
