// Copyright RevenantOps. All Rights Reserved.
#include "UI/MenuWidgetBase.h"
#include "UI/UIHelpers.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ─────────────────────────────────────────────────────────────────────────────
// LIFECYCLE
// ─────────────────────────────────────────────────────────────────────────────

void UMenuWidgetBase::NativeConstruct()
{
    Super::NativeConstruct();

    if (FadeInDuration > 0.f)
    {
        SetRenderOpacity(0.f);
        FadeElapsed = 0.f;
        bFadeActive = true;
    }
}

void UMenuWidgetBase::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    // ── Fade-in ───────────────────────────────────────────────────────────────
    if (bFadeActive)
    {
        FadeElapsed += InDeltaTime;
        const float A = FMath::Clamp(FadeElapsed / FadeInDuration, 0.f, 1.f);
        SetRenderOpacity(A * A * (3.f - 2.f * A)); // smoothstep

        if (A >= 1.f)
        {
            SetRenderOpacity(1.f);
            bFadeActive = false;
        }
    }

    // ── Fade-out ──────────────────────────────────────────────────────────────
    if (bFadeOutActive)
    {
        FadeOutElapsed += InDeltaTime;
        const float A = FMath::Clamp(FadeOutElapsed / FadeOutDuration, 0.f, 1.f);
        SetRenderOpacity(1.f - A * A * (3.f - 2.f * A)); // smoothstep inverse

        if (A >= 1.f)
        {
            bFadeOutActive = false;
            if (FadeOutCallback)
            {
                auto CB = MoveTemp(FadeOutCallback);
                CB(); // peut appeler RemoveFromParent → ne plus accéder à this après
            }
        }
    }

    // ── Hover pulse ───────────────────────────────────────────────────────────
    // Pour chaque bouton enregistré : pulse l'opacité s'il est survolé,
    // sinon restaure à 1. IsHovered() est fiable en NativeTick.
    if (!RegisteredButtons.IsEmpty())
    {
        const float GameTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
        const float Pulse    = UUIHelpers::PulseValue(GameTime, HoverPulseFreq, HoverPulseMin, 1.f);

        for (auto& WeakBtn : RegisteredButtons)
        {
            if (!WeakBtn.IsValid()) continue;
            WeakBtn->SetRenderOpacity(WeakBtn->IsHovered() ? Pulse : 1.f);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// THEME
// ─────────────────────────────────────────────────────────────────────────────

UUITheme* UMenuWidgetBase::GetTheme() const
{
    if (ThemeOverride)
        return ThemeOverride;
    return UUIHelpers::GetDefaultTheme();
}

// ─────────────────────────────────────────────────────────────────────────────
// BOUTONS
// ─────────────────────────────────────────────────────────────────────────────

void UMenuWidgetBase::BindButtonSounds(UButton* Btn)
{
    if (!Btn) return;
    Btn->OnHovered.AddDynamic(this, &UMenuWidgetBase::HandleHover);
    RegisteredButtons.AddUnique(Btn);
}

void UMenuWidgetBase::PlayClickSound()
{
    if (SoundClick)
        UGameplayStatics::PlaySound2D(this, SoundClick);
}

void UMenuWidgetBase::FadeOutThen(float Duration, TFunction<void()> Callback)
{
    if (Duration <= 0.f)
    {
        if (Callback) Callback();
        return;
    }
    // Annuler le fade-in en cours s'il y en a un
    bFadeActive    = false;
    FadeOutDuration = Duration;
    FadeOutElapsed  = 0.f;
    FadeOutCallback = MoveTemp(Callback);
    bFadeOutActive  = true;
}

void UMenuWidgetBase::HandleHover()
{
    if (SoundHover)
        UGameplayStatics::PlaySound2D(this, SoundHover);
}
