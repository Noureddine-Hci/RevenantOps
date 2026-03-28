// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ZombieBase.h"
#include "ZombieRunner.generated.h"

/**
 *  Runner zombie - fast and aggressive.
 *  Sprints at high speed (600), low HP (50), higher melee damage (15).
 *  Dangerous individually, lethal in small groups.
 */
UCLASS(Blueprintable)
class AZombieRunner : public AZombieBase {
  GENERATED_BODY()

public:
  AZombieRunner();
  virtual int32 GetKillPoints() const override { return 75; }
};
