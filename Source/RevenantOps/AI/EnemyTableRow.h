// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "EnemyTableRow.generated.h"

/**
 *  DataTable row struct for enemy balance stats.
 *  Used by ZombieBase::ApplyEnemyDataRow() at BeginPlay.
 *  Create DT_EnemyStats in /Game/Mercenaires/Data/ with this row type.
 */
USTRUCT(BlueprintType)
struct FEnemyTableRow : public FTableRowBase
{
    GENERATED_BODY()

    /** Maximum health points */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float MaxHP = 100.f;

    /** Damage dealt per melee hit */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float MeleeDamage = 10.f;

    /** MaxWalkSpeed in cm/s */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy")
    float MovementSpeed = 300.f;
};
