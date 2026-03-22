// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieBase.h"
#include "ZombieTank.generated.h"

/**
 *  Tank zombie - slow damage sponge with heavy melee.
 *  Walks slowly (150), very high HP (300), high melee damage (30).
 *  Keeps advancing under fire thanks to massive health pool.
 */
UCLASS(Blueprintable)
class AZombieTank : public AZombieBase {
  GENERATED_BODY()

public:
  AZombieTank();
};
