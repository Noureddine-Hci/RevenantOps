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
    virtual void BeginPlay() override;

    /** Musique jouée en boucle sur le menu principal */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* MenuMusic = nullptr;

    /** Volume de la musique menu (0.0 - 1.0) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio", meta = (ClampMin = 0.f, ClampMax = 1.f))
    float MenuMusicVolume = 0.7f;
};
