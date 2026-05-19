// Copyright RevenantOps. All Rights Reserved.

#include "ZombieTank.h"

// Stats viennent de DT_EnemyStats via ApplyEnemyDataRow().
AZombieTank::AZombieTank()
{
    EnemyName = FText::FromString(TEXT("Zombie Tank"));
}
