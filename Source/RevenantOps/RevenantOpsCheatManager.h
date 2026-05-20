// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CheatManager.h"
#include "RevenantOpsCheatManager.generated.h"

/**
 *  Console commands debug pour RevenantOps.
 *  Accessible via la console (~) en PIE/Development.
 *
 *  Activation : RevenantOpsPlayerController::CheatClass = URevenantOpsCheatManager::StaticClass()
 *  Le moteur instancie automatiquement le CheatManager en Development build.
 */
UCLASS(Within = PlayerController)
class REVENANTOPS_API URevenantOpsCheatManager : public UCheatManager
{
    GENERATED_BODY()

public:
    /** Mode invincible — le joueur ne prend plus de dégâts */
    UFUNCTION(Exec, Category = "Cheats")
    void GodMode(bool bEnabled = true);

    /** Tue tous les ennemis encore en vie dans le monde */
    UFUNCTION(Exec, Category = "Cheats")
    void KillAll();

    /** Recharge à fond l'arme courante (+ recharges) */
    UFUNCTION(Exec, Category = "Cheats")
    void GiveAmmo(int32 Amount = 999);

    /** Fixe le nombre de kills du spawner (utile pour tester l'endgame / win screen) */
    UFUNCTION(Exec, Category = "Cheats")
    void SetKills(int32 NewCount);

    /** Ajoute du temps au timer de match */
    UFUNCTION(Exec, Category = "Cheats")
    void AddTime(float Seconds = 60.f);

    /** Heal complet (santé + bouclier) */
    UFUNCTION(Exec, Category = "Cheats")
    void HealFull();

    /** Fixe le combo multiplier */
    UFUNCTION(Exec, Category = "Cheats")
    void SetCombo(int32 Multiplier);

    /** Termine immédiatement le match */
    UFUNCTION(Exec, Category = "Cheats")
    void EndMatch();
};
