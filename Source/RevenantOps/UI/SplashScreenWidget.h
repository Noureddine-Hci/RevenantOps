// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SplashScreenWidget.generated.h"

class UImage;
class UTextBlock;
class UOverlay;
class USizeBox;

// ─────────────────────────────────────────────────────────────────────────────
//  Une entree de la sequence d'intro.
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FSplashEntry
{
    GENERATED_BODY()

    /** Logo ou image a afficher (ex : logo UE5, logo studio, partenaires) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splash")
    TObjectPtr<UTexture2D> Logo = nullptr;

    /** Texte optionnel sous le logo (ex : "Un jeu RevenantOps Studio") */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splash")
    FText SubText;

    /** Duree d'affichage a pleine opacite (secondes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splash",
              meta = (ClampMin = 0.5f, ClampMax = 10.f))
    float HoldDuration = 2.f;

    /** Duree du fondu entrant (secondes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splash",
              meta = (ClampMin = 0.1f, ClampMax = 3.f))
    float FadeInDuration = 0.6f;

    /** Duree du fondu sortant (secondes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splash",
              meta = (ClampMin = 0.1f, ClampMax = 3.f))
    float FadeOutDuration = 0.5f;
};

// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSplashSequenceDone);

/**
 *  Widget d'intro — affiche une sequence de logos avant le menu principal.
 *
 *  Usage :
 *    1. Creer WBP_SplashScreen (parent = USplashScreenWidget)
 *    2. Assigner dans BP_MainMenuGameMode → SplashWidgetClass
 *    3. Remplir SplashSequence dans le BP du GameMode
 *    4. Clic ou touche quelconque avance / saute la sequence
 */
UCLASS(abstract, Blueprintable)
class REVENANTOPS_API USplashScreenWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /** Sequence d'ecrans a afficher dans l'ordre */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splash")
    TArray<FSplashEntry> SplashSequence;

    /**
     *  Taille d'affichage du logo (en pixels a l'ecran).
     *  Defaut : 400x400 — ajuster selon la proportion de ton logo.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Splash",
              meta = (ClampMin = 50.f, ClampMax = 1920.f))
    FVector2D LogoSize = FVector2D(400.f, 400.f);

    /** Broadcast quand toute la sequence est terminee */
    UPROPERTY(BlueprintAssignable, Category = "Splash")
    FOnSplashSequenceDone OnSequenceDone;

    /** Saute immediatement a la fin de la sequence */
    UFUNCTION(BlueprintCallable, Category = "Splash")
    void SkipAll();

protected:
    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    virtual FReply NativeOnMouseButtonDown(const FGeometry&, const FPointerEvent&) override;
    virtual FReply NativeOnKeyDown(const FGeometry&, const FKeyEvent&) override;

private:
    // ── Widgets crees programmatiquement ─────────────────────────────────────
    TObjectPtr<UOverlay>    RootOverlay   = nullptr;
    TObjectPtr<UImage>      BgImage       = nullptr;
    TObjectPtr<USizeBox>    LogoSizeBox   = nullptr;
    TObjectPtr<UImage>      LogoImage     = nullptr;
    TObjectPtr<UTextBlock>  SubTextBlock  = nullptr;

    // ── Etat de la machine a etats ──────────────────────────────────────────
    enum class ESplashState : uint8 { FadeIn, Hold, FadeOut, Done };

    int32       CurrentIndex  = 0;
    ESplashState State        = ESplashState::FadeIn;
    float       StateTimer    = 0.f;
    bool        bSkipping     = false;

    void BuildUI();
    void LoadEntry(int32 Index);
    void AdvanceToNext();
    void SetLogoOpacity(float Alpha);
    void FinishSequence();
};
