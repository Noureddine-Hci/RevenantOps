// Copyright RevenantOps. All Rights Reserved.
#include "UI/MenuWidgetBase.h"
#include "UI/UIHelpers.h"
#include "UI/UITheme.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Blueprint/WidgetTree.h"
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

    // ── Vignette permanente ───────────────────────────────────────────────────
    // Overlay sombre full-screen (ZOrder 99, HitTestInvisible) sur chaque menu.
    // Donne un aspect cinématique / "combat zone" sans bloquer les inputs.
    if (UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr))
    {
        UUITheme* T = GetTheme();
        // On utilise VignetteColor du thème mais on plafonne l'alpha à 0.2
        // pour rester subtil même si le DataAsset a une valeur plus forte.
        const FLinearColor VigColor = T ? T->VignetteColor : FLinearColor(0.f, 0.f, 0.f, 0.35f);
        const float        VigAlpha = FMath::Min(VigColor.A, 0.20f);

        if (UImage* Vig = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), FName("MenuVignette")))
        {
            FSlateBrush VBrush;
            VBrush.DrawAs   = ESlateBrushDrawType::Box;
            VBrush.TintColor = FSlateColor(FLinearColor(VigColor.R, VigColor.G, VigColor.B, VigAlpha));
            Vig->SetBrush(VBrush);
            Vig->SetVisibility(ESlateVisibility::HitTestInvisible);

            if (UCanvasPanelSlot* S = Root->AddChildToCanvas(Vig))
            {
                S->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
                S->SetOffsets(FMargin(0.f));
                S->SetZOrder(99);
            }
        }
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
