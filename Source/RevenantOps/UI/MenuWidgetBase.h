// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MenuWidgetBase.generated.h"

class UButton;
class UUITheme;

/**
 *  Classe de base pour tous les widgets de menu RevenantOps.
 *  - Sons hover + clic centralises
 *  - Acces au UUITheme via GetTheme()
 *  - Fade-in automatique a l'ouverture (smoothstep 0→1)
 *  - Hover pulse : opacite des boutons pulse au survol via NativeTick
 */
UCLASS(abstract, Blueprintable)
class REVENANTOPS_API UMenuWidgetBase : public UUserWidget
{
    GENERATED_BODY()

public:
    // ── Sons ──────────────────────────────────────────────────────────────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio|Menu")
    USoundBase* SoundHover = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio|Menu")
    USoundBase* SoundClick = nullptr;

    // ── Theme ─────────────────────────────────────────────────────────────────
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance|Theme")
    TObjectPtr<UUITheme> ThemeOverride = nullptr;

    // ── Animations ────────────────────────────────────────────────────────────

    /** Duree du fade-in (0 = instantane) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance|Animation",
              meta = (ClampMin = "0.0", ClampMax = "2.0"))
    float FadeInDuration = 0.25f;

    /** Opacite min du pulse hover (ex: 0.7 = 70%) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance|Animation",
              meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HoverPulseMin = 0.75f;

    /** Frequence du pulse hover en Hz */
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Appearance|Animation",
              meta = (ClampMin = "0.5", ClampMax = "5.0"))
    float HoverPulseFreq = 2.f;

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

    UFUNCTION(BlueprintPure, Category = "Appearance|Theme")
    UUITheme* GetTheme() const;

    /**
     *  Appeler sur chaque UButton apres creation :
     *  - Branche le son hover
     *  - Enregistre le bouton pour le hover pulse (NativeTick)
     */
    void BindButtonSounds(UButton* Btn);

    void PlayClickSound();

private:
    // Fade-in
    float FadeElapsed = 0.f;
    bool  bFadeActive = false;

    // Hover pulse : liste de tous les boutons enregistrés
    TArray<TWeakObjectPtr<UButton>> RegisteredButtons;

    UFUNCTION() void HandleHover();
};
