// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieBase.h"
#include "ZombieSlow.generated.h"

/**
 *  Slow zombie - the basic horde enemy.
 *  Walks slowly (200), moderate HP (80), low melee damage (10).
 *  Relies on numbers to overwhelm the player.
 */
UCLASS(Blueprintable)
class AZombieSlow : public AZombieBase {
  GENERATED_BODY()

public:
  AZombieSlow();
  virtual int32 GetKillPoints() const override { return 50; }
};
