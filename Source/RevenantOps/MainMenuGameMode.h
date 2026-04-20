// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MainMenuGameMode.generated.h"

/**
 * GameMode dédié au menu principal.
 * Pas de pawn, pas de HUD gameplay — uniquement les widgets du menu.
 */
UCLASS()
class REVENANTOPS_API AMainMenuGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AMainMenuGameMode();
};
