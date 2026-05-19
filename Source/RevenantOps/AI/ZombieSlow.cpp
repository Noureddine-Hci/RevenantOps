// Copyright RevenantOps. All Rights Reserved.

#include "ZombieSlow.h"

// Stats (HP, vitesse, dégâts mêlée) viennent de DT_EnemyStats via ApplyEnemyDataRow().
AZombieSlow::AZombieSlow()
{
    EnemyName = FText::FromString(TEXT("Zombie Lent"));
}
