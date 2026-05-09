// Copyright RevenantOps. All Rights Reserved.
#include "UI/UIHelpers.h"
#include "UI/UITheme.h"
#include "Engine/Texture2D.h"
#include "Math/UnrealMathUtility.h"
#include "InputAction.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"

// ─────────────────────────────────────────────────────────────────────────────
// BRUSHES
// ─────────────────────────────────────────────────────────────────────────────

FSlateBrush UUIHelpers::MakePanelBrush(const UUITheme* Theme)
{
    FSlateBrush Brush;
    Brush.DrawAs = ESlateBrushDrawType::Box;
    Brush.TintColor = FSlateColor(Theme ? Theme->BgPanel : FLinearColor(0.07f, 0.06f, 0.04f, 1.f));
    return Brush;
}

FSlateBrush UUIHelpers::MakeDimBrush(const UUITheme* Theme)
{
    FSlateBrush Brush;
    Brush.DrawAs = ESlateBrushDrawType::Box;
    Brush.TintColor = FSlateColor(Theme ? Theme->BgPanelDim : FLinearColor(0.05f, 0.04f, 0.03f, 0.85f));
    return Brush;
}

FSlateBrush UUIHelpers::MakeSolidBrush(const FLinearColor& Color)
{
    FSlateBrush Brush;
    Brush.DrawAs = ESlateBrushDrawType::Box;
    Brush.TintColor = FSlateColor(Color);
    return Brush;
}

FSlateBrush UUIHelpers::MakeTextureBrush(UTexture2D* Texture, FVector2D Size)
{
    FSlateBrush Brush;
    if (Texture)
    {
        Brush.SetResourceObject(Texture);
        Brush.ImageSize = Size;
        Brush.DrawAs = ESlateBrushDrawType::Image;
    }
    return Brush;
}

FSlateBrush UUIHelpers::MakeTransparentBrush()
{
    FSlateBrush Brush;
    Brush.DrawAs = ESlateBrushDrawType::NoDrawType;
    Brush.TintColor = FSlateColor(FLinearColor::Transparent);
    return Brush;
}

// ─────────────────────────────────────────────────────────────────────────────
// STYLES DE BOUTON
// ─────────────────────────────────────────────────────────────────────────────

FButtonStyle UUIHelpers::MakeStandardButtonStyle(const UUITheme* Theme)
{
    const FLinearColor BgNormal   = Theme ? Theme->ButtonBgNormal   : FLinearColor(0.f,    0.f,    0.f,    0.f);
    const FLinearColor BgHovered  = Theme ? Theme->ButtonBgHovered  : FLinearColor(0.75f,  0.15f,  0.10f,  0.25f);
    const FLinearColor BgPressed  = Theme ? Theme->ButtonBgPressed  : FLinearColor(0.75f,  0.15f,  0.10f,  0.5f);

    FButtonStyle Style;
    Style.Normal   = MakeSolidBrush(BgNormal);
    Style.Hovered  = MakeSolidBrush(BgHovered);
    Style.Pressed  = MakeSolidBrush(BgPressed);
    Style.Disabled = MakeSolidBrush(FLinearColor(0.1f, 0.1f, 0.1f, 0.4f));
    Style.NormalPadding  = FMargin(12.f, 8.f);
    Style.PressedPadding = FMargin(12.f, 9.f, 12.f, 7.f);
    return Style;
}

FButtonStyle UUIHelpers::MakeDangerButtonStyle(const UUITheme* Theme)
{
    FButtonStyle Style = MakeStandardButtonStyle(Theme);
    const FLinearColor RedAlert = Theme ? Theme->RedAlert : FLinearColor(1.f, 0.25f, 0.15f, 1.f);
    Style.Hovered = MakeSolidBrush(WithAlpha(RedAlert, 0.35f));
    Style.Pressed = MakeSolidBrush(WithAlpha(RedAlert, 0.6f));
    return Style;
}

// ─────────────────────────────────────────────────────────────────────────────
// COULEURS UTILITAIRES
// ─────────────────────────────────────────────────────────────────────────────

FLinearColor UUIHelpers::LerpColor(const FLinearColor& A, const FLinearColor& B, float Alpha)
{
    return FLinearColor::LerpUsingHSV(A, B, FMath::Clamp(Alpha, 0.f, 1.f));
}

float UUIHelpers::PulseValue(float GameTime, float Frequency, float Min, float Max)
{
    const float T = (FMath::Sin(GameTime * Frequency * PI * 2.f) + 1.f) * 0.5f;
    return FMath::Lerp(Min, Max, T);
}

FLinearColor UUIHelpers::WithAlpha(const FLinearColor& Color, float Alpha)
{
    return FLinearColor(Color.R, Color.G, Color.B, Alpha);
}

// ─────────────────────────────────────────────────────────────────────────────
// TYPOGRAPHIE
// ─────────────────────────────────────────────────────────────────────────────

#include "Styling/CoreStyle.h"

FSlateFontInfo UUIHelpers::GetFont(const UUITheme* Theme, int32 Size)
{
    FSlateFontInfo Fi;

    if (Theme)
    {
        if (Size >= 24 && Theme->FontTitle.HasValidFont())
            Fi = Theme->FontTitle;
        else if (Size < 24 && Theme->FontBody.HasValidFont())
            Fi = Theme->FontBody;
        else if (Theme->FontBody.HasValidFont())
            Fi = Theme->FontBody; // fallback body même pour grands textes si Title absent
    }

    if (!Fi.HasValidFont())
        Fi = FCoreStyle::GetDefaultFontStyle(Size >= 18 ? "Bold" : "Regular", Size);

    Fi.Size = Size; // toujours forcer la taille demandée
    return Fi;
}

FSlateFontInfo UUIHelpers::GetMonoFont(const UUITheme* Theme, int32 Size)
{
    FSlateFontInfo Fi;
    if (Theme && Theme->FontMono.HasValidFont())
        Fi = Theme->FontMono;
    else if (Theme && Theme->FontBody.HasValidFont())
        Fi = Theme->FontBody; // fallback body si FontMono non assigne
    if (!Fi.HasValidFont())
        Fi = FCoreStyle::GetDefaultFontStyle("Regular", Size);
    Fi.Size = Size;
    return Fi;
}

// ─────────────────────────────────────────────────────────────────────────────
// INPUT — TOUCHES DYNAMIQUES
// ─────────────────────────────────────────────────────────────────────────────

bool UUIHelpers::IsKeyMappedToAction(ULocalPlayer* LocalPlayer,
                                     const FKey& Key,
                                     UInputAction* Action)
{
    if (!LocalPlayer || !Action) return false;

    const UEnhancedInputLocalPlayerSubsystem* EIS =
        LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (!EIS) return false;

    TArray<FKey> Keys = EIS->QueryKeysMappedToAction(Action);
    return Keys.Contains(Key);
}

FString UUIHelpers::GetKeyLabel(ULocalPlayer* LocalPlayer,
                                UInputAction* Action,
                                const FString& Fallback)
{
    if (!LocalPlayer || !Action)
        return Fallback;

    const UEnhancedInputLocalPlayerSubsystem* EIS =
        LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (!EIS)
        return Fallback;

    TArray<FKey> Keys = EIS->QueryKeysMappedToAction(Action);
    if (Keys.IsEmpty())
        return Fallback;

    // Préférer une touche clavier/souris (pas gamepad) si disponible
    const FKey* BestKey = Keys.FindByPredicate(
        [](const FKey& K){ return !K.IsGamepadKey(); });
    if (!BestKey)
        BestKey = &Keys[0];

    return FString::Printf(TEXT("[%s]"), *BestKey->GetDisplayName().ToString());
}

// ─────────────────────────────────────────────────────────────────────────────
// THEME GLOBAL
// ─────────────────────────────────────────────────────────────────────────────

UUITheme* UUIHelpers::GetDefaultTheme()
{
    static const FSoftObjectPath ThemePath(
        TEXT("/Game/Mercenaires/UI/Themes/DA_Theme_Default.DA_Theme_Default"));

    return Cast<UUITheme>(ThemePath.TryLoad());
}
