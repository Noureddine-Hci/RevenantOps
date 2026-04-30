// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuWidgetBase.generated.h"

class UButton;
class UUITheme;

/**
 *  Classe de base pour tous les widgets de menu RevenantOps.
 *  - Sons de navigation centralises (hover + clic)
 *  - Acces au UUITheme via GetTheme()
 *  - BindButtonSounds() a appeler sur chaque bouton du widget
 */
UCLASS(abstract, Blueprintable)
class REVENANTOPS_API UMenuWidgetBase : public UUserWidget
{
    GENERATED_BODY()

public:
    // ── Sons de navigation ────────────────────────────────────────────────────

    /** Son joue quand la souris survole un bouton */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio|Menu")
    USoundBase* SoundHover = nullptr;

    /** Son joue quand un bouton est clique */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio|Menu")
    USoundBase* SoundClick = nullptr;

    // ── Theme visuel ──────────────────────────────────────────────────────────

    /**
     *  Override du theme par defaut.
     *  Si null, GetTheme() charge DA_Theme_Default automatiquement.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance|Theme")
    TObjectPtr<UUITheme> ThemeOverride = nullptr;

protected:
    /**
     *  Retourne le theme actif : ThemeOverride s'il est set,
     *  sinon DA_Theme_Default via UUIHelpers::GetDefaultTheme().
     *  Peut retourner nullptr si le DataAsset n'existe pas encore en editor.
     */
    UFUNCTION(BlueprintPure, Category = "Appearance|Theme")
    UUITheme* GetTheme() const;

    /** A appeler sur chaque UButton apres sa creation pour brancher les sons */
    void BindButtonSounds(UButton* Btn);

    /** Joue le son de clic (appeler manuellement avant de broadcaster l'action) */
    void PlayClickSound();

private:
    UFUNCTION() void HandleHover();
};
