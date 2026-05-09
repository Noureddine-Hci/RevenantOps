// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateBrush.h"
#include "Layout/Margin.h"
#include "UIHelpers.generated.h"

class UUITheme;
class UTexture2D;
class UInputAction;
class ULocalPlayer;

/**
 *  UUIHelpers — fonctions statiques Blueprint-callable pour creer des brushes,
 *  styles et widgets standardises a partir de UUITheme.
 *
 *  Usage C++ : UUIHelpers::MakePanelBrush(Theme)
 *  Usage BP   : noeud "Make Panel Brush" dans le graphe d'un widget
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class REVENANTOPS_API UUIHelpers : public UObject
{
    GENERATED_BODY()

public:
    // ─────────────────────────────────────────────────────────────────────────
    // BRUSHES
    // ─────────────────────────────────────────────────────────────────────────

    /**
     *  Brush de fond pour un panneau standard.
     *  Couleur BgPanel, pas de texture.
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Brushes",
              meta = (AutoCreateRefTerm = "Theme"))
    static FSlateBrush MakePanelBrush(const UUITheme* Theme);

    /**
     *  Brush de fond translucide (overlay semi-transparent).
     *  Couleur BgPanelDim.
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Brushes",
              meta = (AutoCreateRefTerm = "Theme"))
    static FSlateBrush MakeDimBrush(const UUITheme* Theme);

    /**
     *  Brush couleur unie simple (pas de texture).
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Brushes")
    static FSlateBrush MakeSolidBrush(const FLinearColor& Color);

    /**
     *  Brush depuis une texture (pour icones, logos).
     *  Size = taille desiree en pixels.
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Brushes")
    static FSlateBrush MakeTextureBrush(UTexture2D* Texture, FVector2D Size = FVector2D(64.f, 64.f));

    /**
     *  Brush transparent — fond invisible (pour layouts).
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Brushes")
    static FSlateBrush MakeTransparentBrush();

    // ─────────────────────────────────────────────────────────────────────────
    // STYLES DE BOUTON
    // ─────────────────────────────────────────────────────────────────────────

    /**
     *  Style de bouton standard "RE Grit" :
     *  - Normal    : fond transparent, texte blanc casse
     *  - Hovered   : fond rouge sang 25%, texte or terni
     *  - Pressed   : fond rouge sang 50%
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|ButtonStyle",
              meta = (AutoCreateRefTerm = "Theme"))
    static FButtonStyle MakeStandardButtonStyle(const UUITheme* Theme);

    /**
     *  Style de bouton "danger" (actions irreversibles) :
     *  Hovered en rouge alerte.
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|ButtonStyle",
              meta = (AutoCreateRefTerm = "Theme"))
    static FButtonStyle MakeDangerButtonStyle(const UUITheme* Theme);

    // ─────────────────────────────────────────────────────────────────────────
    // COULEURS UTILITAIRES
    // ─────────────────────────────────────────────────────────────────────────

    /**
     *  Interpole lineairement entre deux couleurs (pour animations BP).
     *  Alpha = 0 -> A, Alpha = 1 -> B.
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Colors")
    static FLinearColor LerpColor(const FLinearColor& A, const FLinearColor& B, float Alpha);

    /**
     *  Pulse sinusoidal entre Min et Max a la frequence donnee.
     *  Utiliser dans NativeTick pour les effets hover "breathing".
     *  GameTime = GetWorld()->GetTimeSeconds()
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Colors")
    static float PulseValue(float GameTime, float Frequency = 2.f,
                            float Min = 0.7f, float Max = 1.f);

    /**
     *  Retourne la meme couleur avec un alpha different.
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Colors")
    static FLinearColor WithAlpha(const FLinearColor& Color, float Alpha);

    // ─────────────────────────────────────────────────────────────────────────
    // TYPOGRAPHIE
    // ─────────────────────────────────────────────────────────────────────────

    /**
     *  Retourne la FSlateFontInfo appropriee depuis le theme :
     *  - Size >= 24 → FontTitle (stencil militaire)
     *  - Size <  24 → FontBody  (corps de texte)
     *  - Fallback   → FCoreStyle "Bold"/"Regular" si le theme est null ou
     *                 si la font n'a pas ete assignee dans le DataAsset.
     *  La taille demandee est toujours forcee sur le resultat.
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Typography")
    static FSlateFontInfo GetFont(const UUITheme* Theme, int32 Size);

    /**
     *  Retourne FontMono du theme (chiffres HUD : ammo, timer, score).
     *  Fallback → FCoreStyle "Regular" si non assigne.
     *  La taille demandee est toujours forcee sur le resultat.
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Typography")
    static FSlateFontInfo GetMonoFont(const UUITheme* Theme, int32 Size);

    // ─────────────────────────────────────────────────────────────────────────
    // INPUT — TOUCHES DYNAMIQUES
    // ─────────────────────────────────────────────────────────────────────────

    /**
     *  Retourne true si la touche donnée est mappée à l'InputAction pour ce joueur.
     *  Utiliser dans NativeOnKeyDown pour honorer les remappages joueur.
     *
     *  Usage :
     *    if (UUIHelpers::IsKeyMappedToAction(GetOwningLocalPlayer(), InKeyEvent.GetKey(), UseAction))
     *        { ... }
     */
    static bool IsKeyMappedToAction(ULocalPlayer* LocalPlayer,
                                    const FKey& Key,
                                    UInputAction* Action);

    /**
     *  Retourne le label de la première touche mappée à une InputAction
     *  pour le joueur local donné, formaté entre crochets : "[E]", "[Tab]", etc.
     *
     *  Fallback : retourne Fallback (ex: "[E]") si LocalPlayer est null,
     *  si le subsystème Enhanced Input n'est pas dispo, ou si aucune touche
     *  n'est mappée à cette action.
     *
     *  Usage :
     *    FString Label = UUIHelpers::GetKeyLabel(GetOwningLocalPlayer(), UseAction, TEXT("[E]"));
     *    BtnUseLabel->SetText(FText::FromString(Label + TEXT(" Utiliser")));
     */
    static FString GetKeyLabel(ULocalPlayer* LocalPlayer,
                               UInputAction* Action,
                               const FString& Fallback = TEXT("[?]"));

    // ─────────────────────────────────────────────────────────────────────────
    // THEME GLOBAL
    // ─────────────────────────────────────────────────────────────────────────

    /**
     *  Charge et retourne DA_Theme_Default depuis
     *  /Game/Mercenaires/UI/Themes/DA_Theme_Default.
     *  Retourne nullptr si le DataAsset n'a pas encore ete cree.
     */
    UFUNCTION(BlueprintPure, Category = "UIHelpers|Theme")
    static UUITheme* GetDefaultTheme();
};
