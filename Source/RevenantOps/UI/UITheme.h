// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Styling/SlateBrush.h"
#include "Fonts/SlateFontInfo.h"
#include "UITheme.generated.h"

/**
 *  UUITheme — DataAsset centralisant toute l'identite visuelle "RE Grit".
 *  Creer DA_Theme_Default dans Content/Mercenaires/UI/Themes/
 *  et le referencer depuis les widgets via GetUITheme() ou UUIHelpers.
 *
 *  Palette "Combat Zone Forsaken" :
 *    Fond profond noir terre brulee / accents rouge sang + or terni / blanc casse
 */
UCLASS(BlueprintType)
class REVENANTOPS_API UUITheme : public UDataAsset
{
    GENERATED_BODY()

public:
    // ─────────────────────────────────────────────────────────────────────────
    // COULEURS — Backgrounds
    // ─────────────────────────────────────────────────────────────────────────

    /** Fond profond (noir terre brulee) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors|Background")
    FLinearColor BgDeep = FLinearColor(0.03f, 0.025f, 0.02f, 1.f);

    /** Fond panneau usé */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors|Background")
    FLinearColor BgPanel = FLinearColor(0.07f, 0.06f, 0.04f, 1.f);

    /** Overlay translucide (semi-transparent) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors|Background")
    FLinearColor BgPanelDim = FLinearColor(0.05f, 0.04f, 0.03f, 0.85f);

    // ─────────────────────────────────────────────────────────────────────────
    // COULEURS — Accents
    // ─────────────────────────────────────────────────────────────────────────

    /** Rouge sang militaire — accents principaux, bordures actives */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors|Accent")
    FLinearColor RedBlood = FLinearColor(0.75f, 0.15f, 0.10f, 1.f);

    /** Or terni / laiton — titres, etiquettes */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors|Accent")
    FLinearColor GoldTarnish = FLinearColor(0.85f, 0.70f, 0.30f, 1.f);

    /** Or eteint — sub-textes, valeurs secondaires */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors|Accent")
    FLinearColor GoldDim = FLinearColor(0.55f, 0.45f, 0.20f, 1.f);

    /** Rouge alerte — low health, danger critique */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors|Accent")
    FLinearColor RedAlert = FLinearColor(1.0f, 0.25f, 0.15f, 1.f);

    // ─────────────────────────────────────────────────────────────────────────
    // COULEURS — Textes
    // ─────────────────────────────────────────────────────────────────────────

    /** Blanc casse — texte courant */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors|Text")
    FLinearColor WhiteText = FLinearColor(0.95f, 0.93f, 0.88f, 1.f);

    /** Gris doux — texte secondaire, hints, labels */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors|Text")
    FLinearColor GreySoft = FLinearColor(0.45f, 0.42f, 0.38f, 1.f);

    /** Blanc pur — valeurs critiques HUD (ammo bas, timer danger) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Colors|Text")
    FLinearColor WhitePure = FLinearColor(1.f, 1.f, 1.f, 1.f);

    // ─────────────────────────────────────────────────────────────────────────
    // TYPOGRAPHIE
    // ─────────────────────────────────────────────────────────────────────────

    /**
     *  Police "Stencil" — titres, noms d'ecran, logos.
     *  Assigner Allerta Stencil ou Saira Stencil One (Google Fonts).
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Typography")
    FSlateFontInfo FontTitle;

    /**
     *  Police "Condensed" — corps de texte, descriptions, stats.
     *  Assigner Roboto Condensed ou Inter Tight.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Typography")
    FSlateFontInfo FontBody;

    /**
     *  Police "Mono" — valeurs numeriques HUD (ammo, timer, score).
     *  Assigner Share Tech Mono ou JetBrains Mono.
     */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Typography")
    FSlateFontInfo FontMono;

    // ─────────────────────────────────────────────────────────────────────────
    // TAILLES DE POLICE (pour variants sans FSlateFontInfo configure)
    // ─────────────────────────────────────────────────────────────────────────

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Typography|Sizes")
    int32 SizeTitleLarge = 48;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Typography|Sizes")
    int32 SizeTitleMedium = 32;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Typography|Sizes")
    int32 SizeBody = 18;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Typography|Sizes")
    int32 SizeBodySmall = 14;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Typography|Sizes")
    int32 SizeHUD = 22;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Typography|Sizes")
    int32 SizeHUDSmall = 16;

    // ─────────────────────────────────────────────────────────────────────────
    // BOUTONS
    // ─────────────────────────────────────────────────────────────────────────

    /** Couleur du texte d'un bouton au repos */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buttons")
    FLinearColor ButtonTextNormal = FLinearColor(0.95f, 0.93f, 0.88f, 1.f);

    /** Couleur du texte d'un bouton au survol */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buttons")
    FLinearColor ButtonTextHovered = FLinearColor(0.85f, 0.70f, 0.30f, 1.f);

    /** Couleur du fond de bouton au repos (transparent par defaut) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buttons")
    FLinearColor ButtonBgNormal = FLinearColor(0.f, 0.f, 0.f, 0.f);

    /** Couleur du fond de bouton au survol */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buttons")
    FLinearColor ButtonBgHovered = FLinearColor(0.75f, 0.15f, 0.10f, 0.25f);

    /** Couleur du fond de bouton enfonce */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Buttons")
    FLinearColor ButtonBgPressed = FLinearColor(0.75f, 0.15f, 0.10f, 0.5f);

    // ─────────────────────────────────────────────────────────────────────────
    // PANNEAUX
    // ─────────────────────────────────────────────────────────────────────────

    /** Epaisseur des bordures "metal plating" sur les panneaux (px) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Panels",
              meta = (ClampMin = 1.f, ClampMax = 8.f))
    float BorderThickness = 2.f;

    /** Couleur des bordures de panneau par defaut */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Panels")
    FLinearColor BorderColor = FLinearColor(0.55f, 0.45f, 0.20f, 0.8f);

    /** Couleur des bordures de panneau actif / selectionne */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Panels")
    FLinearColor BorderColorActive = FLinearColor(0.75f, 0.15f, 0.10f, 1.f);

    /** Padding interne des panneaux (px) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Panels")
    FMargin PanelPadding = FMargin(16.f, 12.f, 16.f, 12.f);

    // ─────────────────────────────────────────────────────────────────────────
    // VIGNETTE
    // ─────────────────────────────────────────────────────────────────────────

    /** Opacite de la vignette permanente (0=invisible, 1=tres sombre) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects",
              meta = (ClampMin = 0.f, ClampMax = 1.f))
    float VignetteOpacity = 0.35f;

    /** Couleur de la vignette */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Effects")
    FLinearColor VignetteColor = FLinearColor(0.f, 0.f, 0.f, 0.35f);

    // ─────────────────────────────────────────────────────────────────────────
    // ANIMATIONS
    // ─────────────────────────────────────────────────────────────────────────

    /** Duree du fade-in a l'ouverture d'un ecran (secondes) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations",
              meta = (ClampMin = 0.f, ClampMax = 2.f))
    float ScreenFadeInDuration = 0.3f;

    /** Duree du slide lateral entre ecrans (secondes) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animations",
              meta = (ClampMin = 0.f, ClampMax = 1.f))
    float ScreenSlideDuration = 0.25f;

    // ─────────────────────────────────────────────────────────────────────────
    // HELPERS
    // ─────────────────────────────────────────────────────────────────────────

    /** Retourne une FLinearColor avec l'alpha override */
    UFUNCTION(BlueprintPure, Category = "UITheme|Helpers")
    static FLinearColor WithAlpha(const FLinearColor& Color, float Alpha)
    {
        return FLinearColor(Color.R, Color.G, Color.B, Alpha);
    }
};
