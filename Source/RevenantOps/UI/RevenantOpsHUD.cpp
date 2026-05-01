// Copyright RevenantOps. All Rights Reserved.

#include "RevenantOpsHUD.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Styling/CoreStyle.h"
#include "UI/UITheme.h"
#include "UI/UIHelpers.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MercenairesGameState.h"
#include "RevenantOpsCharacter.h"
#include "WeaponBase.h"
#include "EnemyWaveSpawner.h"

// Helper : positionne un widget dans son slot Canvas Panel
static void SetCanvasSlot(UWidget* Widget, FVector2D Position, FVector2D Size, FAnchors Anchors)
{
  if (!Widget) return;
  if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Widget->Slot))
  {
    Slot->SetAnchors(Anchors);
    Slot->SetPosition(Position);
    Slot->SetSize(Size);
    Slot->SetAutoSize(false);
  }
}

TSharedRef<SWidget> URevenantOpsHUD::RebuildWidget()
{
  if (!IsDesignTime() && WidgetTree && !WidgetTree->RootWidget)
    BuildDefaultUI();
  return Super::RebuildWidget();
}

void URevenantOpsHUD::BuildDefaultUI()
{
  if (!WidgetTree) return;

  // Root canvas panel
  UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(
      UCanvasPanel::StaticClass(), FName("RootCanvas"));
  WidgetTree->RootWidget = Root;
  if (!Root) return;

  // ── Theme ────────────────────────────────────────────────────────────────
  UUITheme* T = UUIHelpers::GetDefaultTheme();

  const FLinearColor C_BgPanel  = T ? T->BgPanelDim    : FLinearColor(0.05f, 0.04f, 0.03f, 0.85f);
  const FLinearColor C_Accent   = T ? T->RedBlood       : FLinearColor(0.75f, 0.15f, 0.10f, 1.f);
  const FLinearColor C_Gold     = T ? T->GoldTarnish    : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);
  const FLinearColor C_GoldDim  = T ? T->GoldDim        : FLinearColor(0.55f, 0.45f, 0.20f, 1.f);
  const FLinearColor C_White    = T ? T->WhiteText      : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);
  const FLinearColor C_Grey     = T ? T->GreySoft       : FLinearColor(0.45f, 0.42f, 0.38f, 1.f);

  // ── Typed helpers ────────────────────────────────────────────────────────
  auto MakeBar = [&](UProgressBar*& Out, FName Name, FVector2D Pos, FVector2D Sz, FAnchors A, int32 Z = 1) {
    Out = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass(), Name);
    if (Out) { if (auto* S = Root->AddChildToCanvas(Out)) { S->SetAnchors(A); S->SetPosition(Pos); S->SetSize(Sz); S->SetZOrder(Z); } }
  };
  auto MakeText = [&](UTextBlock*& Out, FName Name, FVector2D Pos, FVector2D Sz, FAnchors A, int32 Z = 1) {
    Out = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
    if (Out) { if (auto* S = Root->AddChildToCanvas(Out)) { S->SetAnchors(A); S->SetPosition(Pos); S->SetSize(Sz); S->SetZOrder(Z); } }
  };
  auto MakeImg = [&](UImage*& Out, FName Name, FVector2D Pos, FVector2D Sz, FAnchors A, int32 Z = 1) {
    Out = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
    if (Out) { if (auto* S = Root->AddChildToCanvas(Out)) { S->SetAnchors(A); S->SetPosition(Pos); S->SetSize(Sz); S->SetZOrder(Z); } }
  };

  // Brush solide générique
  auto MakeSolid = [](FLinearColor C) -> FSlateBrush {
    FSlateBrush B; B.DrawAs = ESlateBrushDrawType::Box; B.TintColor = FSlateColor(C); return B;
  };

  // ── Panneaux stencil (ZOrder 0, derrière le contenu à ZOrder 1) ──────────
  // Panel fond + accent rouge gauche sur chaque groupe HUD

  // Ajoute un UBorder de fond sombre
  auto MakePanelBg = [&](FName Name, FVector2D Pos, FVector2D Sz, FAnchors A) {
    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), Name);
    if (Bg) {
      if (UCanvasPanelSlot* S = Root->AddChildToCanvas(Bg)) {
        S->SetAnchors(A); S->SetPosition(Pos); S->SetSize(Sz); S->SetZOrder(0);
      }
      Bg->SetBrushColor(C_BgPanel);
      Bg->SetPadding(FMargin(0.f));
      Bg->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    return Bg;
  };

  // Ajoute une fine barre d'accent (2px) sur le bord gauche d'un panneau
  auto MakeAccentBar = [&](FName Name, FVector2D Pos, FVector2D Sz, FAnchors A) {
    UImage* Acc = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
    if (Acc) {
      if (UCanvasPanelSlot* S = Root->AddChildToCanvas(Acc)) {
        S->SetAnchors(A); S->SetPosition(Pos); S->SetSize(Sz); S->SetZOrder(0);
      }
      Acc->SetBrush(MakeSolid(C_Accent));
      Acc->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
    return Acc;
  };

  // TOP-LEFT : health + stamina  (8, 8) → 278 × 76
  MakePanelBg(FName("Panel_Health"), FVector2D(8.f, 8.f), FVector2D(278.f, 76.f), FAnchors(0.f, 0.f));
  MakeAccentBar(FName("Accent_Health"), FVector2D(8.f, 8.f), FVector2D(3.f, 76.f), FAnchors(0.f, 0.f));

  // TOP-CENTER : timer  (-72, 8) → 144 × 52
  MakePanelBg(FName("Panel_Timer"), FVector2D(-72.f, 8.f), FVector2D(144.f, 52.f), FAnchors(0.5f, 0.f));
  {
    // Barre d'accent en bas du panneau timer
    UImage* Acc = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), FName("Accent_Timer"));
    if (Acc) {
      if (UCanvasPanelSlot* S = Root->AddChildToCanvas(Acc)) {
        S->SetAnchors(FAnchors(0.5f, 0.f)); S->SetPosition(FVector2D(-72.f, 58.f));
        S->SetSize(FVector2D(144.f, 3.f)); S->SetZOrder(0);
      }
      Acc->SetBrush(MakeSolid(C_Accent));
      Acc->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
  }

  // TOP-RIGHT : score + wave + combo  (-178, 8) → 170 × 142
  MakePanelBg(FName("Panel_Score"), FVector2D(-178.f, 8.f), FVector2D(170.f, 142.f), FAnchors(1.f, 0.f));
  {
    // Accent rouge sur le bord droit (2px, toute la hauteur)
    UImage* Acc = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), FName("Accent_Score"));
    if (Acc) {
      if (UCanvasPanelSlot* S = Root->AddChildToCanvas(Acc)) {
        S->SetAnchors(FAnchors(1.f, 0.f)); S->SetPosition(FVector2D(-8.f, 8.f));
        S->SetSize(FVector2D(3.f, 142.f)); S->SetZOrder(0);
      }
      Acc->SetBrush(MakeSolid(C_Accent));
      Acc->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
  }

  // BOTTOM-RIGHT : ammo + weapon  (-240, -118) → 230 × 108
  MakePanelBg(FName("Panel_Ammo"), FVector2D(-240.f, -118.f), FVector2D(230.f, 108.f), FAnchors(1.f, 1.f));
  {
    // Accent rouge sur le bord supérieur (2px, toute la largeur)
    UImage* Acc = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), FName("Accent_Ammo"));
    if (Acc) {
      if (UCanvasPanelSlot* S = Root->AddChildToCanvas(Acc)) {
        S->SetAnchors(FAnchors(1.f, 1.f)); S->SetPosition(FVector2D(-240.f, -118.f));
        S->SetSize(FVector2D(230.f, 3.f)); S->SetZOrder(0);
      }
      Acc->SetBrush(MakeSolid(C_Accent));
      Acc->SetVisibility(ESlateVisibility::HitTestInvisible);
    }
  }

  // ── Progress bars ────────────────────────────────────────────────────────
  MakeBar(HealthBar,     FName("HealthBar"),     FVector2D(20.f,   22.f),  FVector2D(250.f, 18.f), FAnchors(0.f,  0.f));
  MakeBar(ShieldBar,     FName("ShieldBar"),     FVector2D(20.f,   44.f),  FVector2D(200.f, 10.f), FAnchors(0.f,  0.f));
  MakeBar(StaminaBar,    FName("StaminaBar"),    FVector2D(20.f,   58.f),  FVector2D(180.f, 12.f), FAnchors(0.f,  0.f));
  MakeBar(ComboTimerBar, FName("ComboTimerBar"), FVector2D(-165.f, 120.f), FVector2D(150.f, 8.f),  FAnchors(1.f,  0.f));
  MakeBar(ReloadBar,     FName("ReloadBar"),     FVector2D(-150.f, -55.f), FVector2D(300.f, 14.f), FAnchors(0.5f, 1.f));

  // Couleur de la barre combo : or terni
  if (ComboTimerBar) ComboTimerBar->SetFillColorAndOpacity(C_Gold);

  // Couleur reload bar : rouge sang
  if (ReloadBar) ReloadBar->SetFillColorAndOpacity(C_Accent);

  // ── Text blocks ──────────────────────────────────────────────────────────
  MakeText(TimerText,            FName("TimerText"),            FVector2D(-60.f,   16.f),  FVector2D(120.f, 36.f), FAnchors(0.5f, 0.f));
  MakeText(ScoreText,            FName("ScoreText"),            FVector2D(-168.f,  16.f),  FVector2D(152.f, 30.f), FAnchors(1.f,  0.f));
  MakeText(WaveText,             FName("WaveText"),             FVector2D(-168.f,  52.f),  FVector2D(152.f, 24.f), FAnchors(1.f,  0.f));
  MakeText(ComboText,            FName("ComboText"),            FVector2D(-168.f,  82.f),  FVector2D(152.f, 32.f), FAnchors(1.f,  0.f));
  MakeText(AmmoCurrentText,      FName("AmmoCurrentText"),      FVector2D(-178.f, -72.f),  FVector2D(130.f, 44.f), FAnchors(1.f,  1.f));
  MakeText(AmmoReserveText,      FName("AmmoReserveText"),      FVector2D(-90.f,  -38.f),  FVector2D(80.f,  28.f), FAnchors(1.f,  1.f));
  MakeText(WeaponNameText,       FName("WeaponNameText"),       FVector2D(-228.f, -112.f), FVector2D(210.f, 28.f), FAnchors(1.f,  1.f));
  MakeText(KillNotificationText, FName("KillNotificationText"), FVector2D(-120.f,  80.f),  FVector2D(240.f, 30.f), FAnchors(0.5f, 0.f));

  // ── Fonts thème (FontMono pour chiffres, FontBody pour labels) ──────────
  if (TimerText)            TimerText->SetFont(UUIHelpers::GetMonoFont(T, 28));
  if (ScoreText)            ScoreText->SetFont(UUIHelpers::GetMonoFont(T, 22));
  if (WaveText)             WaveText->SetFont(UUIHelpers::GetFont(T, 15));
  if (ComboText)            ComboText->SetFont(UUIHelpers::GetMonoFont(T, 32));
  if (AmmoCurrentText)      AmmoCurrentText->SetFont(UUIHelpers::GetMonoFont(T, 38));
  if (AmmoReserveText)      AmmoReserveText->SetFont(UUIHelpers::GetMonoFont(T, 18));
  if (WeaponNameText)       WeaponNameText->SetFont(UUIHelpers::GetFont(T, 15));
  if (KillNotificationText) KillNotificationText->SetFont(UUIHelpers::GetFont(T, 20));

  // ── Couleurs textes depuis le theme ─────────────────────────────────────
  if (TimerText)            TimerText->SetColorAndOpacity(FSlateColor(C_White));
  if (ScoreText)            ScoreText->SetColorAndOpacity(FSlateColor(C_Gold));
  if (WaveText)             WaveText->SetColorAndOpacity(FSlateColor(C_GoldDim));
  if (ComboText)            ComboText->SetColorAndOpacity(FSlateColor(C_Gold));
  if (AmmoCurrentText)      AmmoCurrentText->SetColorAndOpacity(FSlateColor(C_White));
  if (AmmoReserveText)      AmmoReserveText->SetColorAndOpacity(FSlateColor(C_Grey));
  if (WeaponNameText)       WeaponNameText->SetColorAndOpacity(FSlateColor(C_GoldDim));
  if (KillNotificationText) KillNotificationText->SetColorAndOpacity(FSlateColor(C_Gold));

  // ── Images ───────────────────────────────────────────────────────────────
  // Ancien CrosshairImage masqué — remplacé par les 4 traits
  MakeImg(CrosshairImage,       FName("CrosshairImage"),       FVector2D(0.f, 0.f), FVector2D(1.f, 1.f), FAnchors(0.5f, 0.5f));
  if (CrosshairImage) CrosshairImage->SetVisibility(ESlateVisibility::Collapsed);

  // 4 traits du viseur (positions initiales au repos, gap=4)
  MakeImg(CrosshairTop,    FName("CrosshairTop"),    FVector2D(-1.f, -(4.f + 10.f)), FVector2D(2.f, 10.f), FAnchors(0.5f, 0.5f));
  MakeImg(CrosshairBottom, FName("CrosshairBottom"), FVector2D(-1.f,   4.f),          FVector2D(2.f, 10.f), FAnchors(0.5f, 0.5f));
  MakeImg(CrosshairLeft,   FName("CrosshairLeft"),   FVector2D(-(4.f + 10.f), -1.f), FVector2D(10.f, 2.f), FAnchors(0.5f, 0.5f));
  MakeImg(CrosshairRight,  FName("CrosshairRight"),  FVector2D(4.f,          -1.f),  FVector2D(10.f, 2.f), FAnchors(0.5f, 0.5f));

  const FLinearColor LineColor(1.f, 1.f, 1.f, 0.9f);
  for (UImage* Line : {CrosshairTop, CrosshairBottom, CrosshairLeft, CrosshairRight}) {
    if (Line) {
      Line->SetBrush(MakeSolid(FLinearColor::White));
      Line->SetColorAndOpacity(LineColor);
    }
  }

  MakeImg(HitMarkerImage,       FName("HitMarkerImage"),       FVector2D(-16.f, -16.f), FVector2D(32.f, 32.f), FAnchors(0.5f, 0.5f));
  MakeImg(DamageDirectionImage, FName("DamageDirectionImage"), FVector2D(-5.f, -103.f), FVector2D(10.f, 36.f), FAnchors(0.5f, 0.5f));

  // LowHealthVignette : full-screen stretch
  LowHealthVignette = WidgetTree->ConstructWidget<UImage>(
      UImage::StaticClass(), FName("LowHealthVignette"));
  if (LowHealthVignette) {
    if (UCanvasPanelSlot* VigSlot = Root->AddChildToCanvas(LowHealthVignette)) {
      VigSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
      VigSlot->SetOffsets(FMargin(0.f));
      VigSlot->SetZOrder(2);
    }
    LowHealthVignette->SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, 0.f));
  }

  // ── Popup RE5 pickup ─────────────────────────────────────────────────────
  PickupPromptBG = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), FName("PickupPromptBG"));
  if (PickupPromptBG && Root)
  {
    if (UCanvasPanelSlot* S = Root->AddChildToCanvas(PickupPromptBG))
    {
      S->SetAnchors(FAnchors(0.5f, 1.f));
      S->SetAlignment(FVector2D(0.5f, 1.f));
      S->SetPosition(FVector2D(0.f, -120.f));
      S->SetAutoSize(true);
      S->SetZOrder(5);
    }
    // Fond sombre thème + bordure or
    PickupPromptBG->SetBrushColor(T ? T->BgPanel : FLinearColor(0.07f, 0.06f, 0.04f, 0.92f));
    PickupPromptBG->SetPadding(FMargin(14.f, 10.f));
    PickupPromptBG->SetVisibility(ESlateVisibility::Collapsed);

    UHorizontalBox* HBox = WidgetTree->ConstructWidget<UHorizontalBox>();
    PickupPromptBG->SetContent(HBox);
    if (HBox)
    {
      PickupPromptIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), FName("PickupPromptIcon"));
      if (PickupPromptIcon)
      {
        if (UHorizontalBoxSlot* HS = HBox->AddChildToHorizontalBox(PickupPromptIcon))
        {
          HS->SetVerticalAlignment(VAlign_Center);
          HS->SetPadding(FMargin(0.f, 0.f, 12.f, 0.f));
        }
        PickupPromptIcon->SetDesiredSizeOverride(FVector2D(48.f, 48.f));
      }

      UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
      if (VBox && HBox->AddChildToHorizontalBox(VBox))
      {
        UTextBlock* KeyHint = WidgetTree->ConstructWidget<UTextBlock>();
        if (KeyHint)
        {
          KeyHint->SetText(FText::FromString(TEXT("[E]  Prendre")));
          KeyHint->SetFont(UUIHelpers::GetFont(T, 16));
          KeyHint->SetColorAndOpacity(FSlateColor(C_Gold));
          VBox->AddChildToVerticalBox(KeyHint);
        }

        PickupPromptName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName("PickupPromptName"));
        if (PickupPromptName)
        {
          PickupPromptName->SetFont(UUIHelpers::GetFont(T, 13));
          PickupPromptName->SetColorAndOpacity(FSlateColor(C_White));
          VBox->AddChildToVerticalBox(PickupPromptName);
        }
      }
    }
  }
}

void URevenantOpsHUD::NativeConstruct() {
  Super::NativeConstruct();

  // Fallback si RebuildWidget n'a pas pu construire (ex: WBP rechargé à chaud)
  if (!HealthBar) BuildDefaultUI();

  // --- Positionnement HUD (layout) ---
  // Top-left : HealthBar + StaminaBar
  SetCanvasSlot(HealthBar,   FVector2D(20.f,  20.f), FVector2D(250.f, 18.f), FAnchors(0.f, 0.f));
  SetCanvasSlot(StaminaBar,  FVector2D(20.f,  46.f), FVector2D(180.f, 12.f), FAnchors(0.f, 0.f));

  // Top-center : Timer
  SetCanvasSlot(TimerText,   FVector2D(-60.f, 20.f), FVector2D(120.f, 36.f), FAnchors(0.5f, 0.f));

  // Top-right : Score + Wave
  SetCanvasSlot(ScoreText,   FVector2D(-160.f, 20.f), FVector2D(150.f, 30.f), FAnchors(1.f, 0.f));
  SetCanvasSlot(WaveText,    FVector2D(-160.f, 55.f), FVector2D(150.f, 24.f), FAnchors(1.f, 0.f));

  // Bottom-right : Ammo
  SetCanvasSlot(AmmoCurrentText, FVector2D(-170.f, -70.f), FVector2D(120.f, 40.f), FAnchors(1.f, 1.f));
  SetCanvasSlot(AmmoReserveText, FVector2D(-80.f,  -40.f), FVector2D(70.f,  28.f), FAnchors(1.f, 1.f));
  SetCanvasSlot(WeaponNameText,  FVector2D(-220.f, -105.f),FVector2D(200.f, 28.f), FAnchors(1.f, 1.f));

  // Center : HitMarker (32x32 centré)
  SetCanvasSlot(HitMarkerImage, FVector2D(-16.f, -16.f), FVector2D(32.f, 32.f), FAnchors(0.5f, 0.5f));

  // Bottom-center : ReloadBar
  SetCanvasSlot(ReloadBar, FVector2D(-150.f, -55.f), FVector2D(300.f, 14.f), FAnchors(0.5f, 1.f));

  // Top-center (sous timer) : KillNotification
  SetCanvasSlot(KillNotificationText, FVector2D(-120.f, 80.f), FVector2D(240.f, 30.f), FAnchors(0.5f, 0.f));

  // Cache character reference
  if (APawn *Pawn = GetOwningPlayerPawn()) {
    CachedCharacter = Cast<ARevenantOpsCharacter>(Pawn);

    if (CachedCharacter) {
      // Find health component on character
      CachedHealthComp =
          CachedCharacter->FindComponentByClass<UHealthComponent>();
    }
  }

  // Create any combo widgets missing from the WBP (BindWidgetOptional won't create them)
  if (!ComboText || !ComboTimerBar)
  {
    if (UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree->RootWidget))
    {
      UUITheme* Th = UUIHelpers::GetDefaultTheme();
      const FLinearColor C_Gold = Th ? Th->GoldTarnish : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);

      if (!ComboText)
      {
        ComboText = WidgetTree->ConstructWidget<UTextBlock>(
            UTextBlock::StaticClass(), FName("ComboText"));
        if (ComboText)
        {
          if (UCanvasPanelSlot* S = Root->AddChildToCanvas(ComboText))
          {
            S->SetAnchors(FAnchors(1.f, 0.f));
            S->SetPosition(FVector2D(-168.f, 82.f));
            S->SetSize(FVector2D(152.f, 36.f));
            S->SetZOrder(1);
          }
          ComboText->SetFont(UUIHelpers::GetMonoFont(Th, 32));
          ComboText->SetColorAndOpacity(FSlateColor(C_Gold));
        }
      }

      if (!ComboTimerBar)
      {
        ComboTimerBar = WidgetTree->ConstructWidget<UProgressBar>(
            UProgressBar::StaticClass(), FName("ComboTimerBar"));
        if (ComboTimerBar)
        {
          if (UCanvasPanelSlot* S = Root->AddChildToCanvas(ComboTimerBar))
          {
            S->SetAnchors(FAnchors(1.f, 0.f));
            S->SetPosition(FVector2D(-165.f, 122.f));
            S->SetSize(FVector2D(150.f, 8.f));
            S->SetZOrder(1);
          }
          ComboTimerBar->SetFillColorAndOpacity(C_Gold);
        }
      }
    }
  }

  // Créer les 4 traits du viseur sur le canvas existant
  if (UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr)) {
    FSlateBrush WhiteBrush;
    WhiteBrush.DrawAs = ESlateBrushDrawType::Box;
    WhiteBrush.TintColor = FSlateColor(FLinearColor::White);
    const FLinearColor LineColor(1.f, 1.f, 1.f, 0.9f);

    auto MakeLine = [&](UImage*& Out, FName Name, FVector2D Pos, FVector2D Size) {
      if (Out) return; // déjà créé par BuildDefaultUI
      Out = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), Name);
      if (!Out) return;
      Out->SetBrush(WhiteBrush);
      Out->SetColorAndOpacity(LineColor);
      if (UCanvasPanelSlot* S = Root->AddChildToCanvas(Out)) {
        S->SetAnchors(FAnchors(0.5f, 0.5f));
        S->SetPosition(Pos);
        S->SetSize(Size);
        S->SetAlignment(FVector2D(0.5f, 0.5f));
      }
    };

    const float G = CrosshairGapMin;
    const float L = CrosshairLineLength;
    const float T = CrosshairLineThickness;
    MakeLine(CrosshairTop,    FName("CrosshairTop"),    FVector2D(0.f, -(G + L * 0.5f)), FVector2D(T, L));
    MakeLine(CrosshairBottom, FName("CrosshairBottom"), FVector2D(0.f,   G + L * 0.5f),  FVector2D(T, L));
    MakeLine(CrosshairLeft,   FName("CrosshairLeft"),   FVector2D(-(G + L * 0.5f), 0.f), FVector2D(L, T));
    MakeLine(CrosshairRight,  FName("CrosshairRight"),  FVector2D(  G + L * 0.5f,  0.f), FVector2D(L, T));
    CrosshairCurrentGap = G;
  }

  // Initialize hit marker as hidden
  if (HitMarkerImage) {
    HitMarkerImage->SetOpacity(0.f);
  }

  // Cache game state for timer/score
  CachedGameState = GetWorld()->GetGameState<AMercenairesGameState>();

  // Cache wave spawner for wave counter
  TArray<AActor *> Spawners;
  UGameplayStatics::GetAllActorsOfClass(
      GetWorld(), AEnemyWaveSpawner::StaticClass(), Spawners);
  if (Spawners.Num() > 0) {
    CachedWaveSpawner = Cast<AEnemyWaveSpawner>(Spawners[0]);
  }

  // Initialize new HUD elements as hidden
  if (ReloadBar) {
    ReloadBar->SetVisibility(ESlateVisibility::Collapsed);
  }
  if (DamageDirectionImage) {
    // Rouge semi-transparent, bords arrondis simulés par la couleur
    FSlateBrush RedBrush;
    RedBrush.DrawAs = ESlateBrushDrawType::Box;
    RedBrush.TintColor = FSlateColor(FLinearColor(1.f, 0.1f, 0.1f, 1.f));
    DamageDirectionImage->SetBrush(RedBrush);
    DamageDirectionImage->SetVisibility(ESlateVisibility::Collapsed);
  }
  if (KillNotificationText) {
    KillNotificationText->SetVisibility(ESlateVisibility::Collapsed);
  }

  // Créer le popup RE5 si pas déjà fait par BuildDefaultUI
  if (!PickupPromptBG)
  {
    UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (Root && WidgetTree)
    {
      PickupPromptBG = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), FName("PickupPromptBG"));
      if (PickupPromptBG)
      {
        if (UCanvasPanelSlot* S = Root->AddChildToCanvas(PickupPromptBG))
        {
          S->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
          S->SetAlignment(FVector2D(0.5f, 1.f));
          S->SetPosition(FVector2D(0.f, -120.f));
          S->SetAutoSize(true);
        }
        PickupPromptBG->SetBrushColor(FLinearColor(0.f, 0.f, 0.f, 0.78f));
        PickupPromptBG->SetPadding(FMargin(14.f, 10.f));
        PickupPromptBG->SetVisibility(ESlateVisibility::Collapsed);

        UHorizontalBox* HBox = WidgetTree->ConstructWidget<UHorizontalBox>();
        PickupPromptBG->SetContent(HBox);

        // Icone 48x48
        PickupPromptIcon = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), FName("PickupPromptIcon"));
        if (PickupPromptIcon)
        {
          if (UHorizontalBoxSlot* HS = Cast<UHorizontalBoxSlot>(HBox->AddChild(PickupPromptIcon)))
          {
            HS->SetPadding(FMargin(0.f, 0.f, 14.f, 0.f));
            HS->SetVerticalAlignment(VAlign_Center);
          }
          FSlateBrush IconBrush;
          IconBrush.ImageSize = FVector2D(48.f, 48.f);
          PickupPromptIcon->SetBrush(IconBrush);
        }

        // Textes : "[E]  Prendre" + nom
        UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
        if (UHorizontalBoxSlot* HS = Cast<UHorizontalBoxSlot>(HBox->AddChild(VBox)))
          HS->SetVerticalAlignment(VAlign_Center);

        UTextBlock* PressLabel = WidgetTree->ConstructWidget<UTextBlock>();
        PressLabel->SetText(FText::FromString(TEXT("[E]  Prendre")));
        FSlateFontInfo PF = PressLabel->GetFont();
        PF.Size = 17;
        PressLabel->SetFont(PF);
        PressLabel->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.95f, 0.3f)));
        VBox->AddChildToVerticalBox(PressLabel);

        PickupPromptName = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName("PickupPromptName"));
        if (PickupPromptName)
        {
          FSlateFontInfo NF = PickupPromptName->GetFont();
          NF.Size = 14;
          PickupPromptName->SetFont(NF);
          PickupPromptName->SetColorAndOpacity(FSlateColor(FLinearColor::White));
          VBox->AddChildToVerticalBox(PickupPromptName);
        }
      }
    }
  }

  // Prompt finisher [F] — au-dessus du pickup prompt
  if (!FinisherPromptBG)
  {
    UCanvasPanel* Root = Cast<UCanvasPanel>(WidgetTree ? WidgetTree->RootWidget : nullptr);
    if (Root && WidgetTree)
    {
      FinisherPromptBG = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass(), FName("FinisherPromptBG"));
      if (FinisherPromptBG)
      {
        if (UCanvasPanelSlot* S = Root->AddChildToCanvas(FinisherPromptBG))
        {
          S->SetAnchors(FAnchors(0.5f, 1.f, 0.5f, 1.f));
          S->SetAlignment(FVector2D(0.5f, 1.f));
          S->SetPosition(FVector2D(0.f, -180.f)); // au-dessus du pickup prompt
          S->SetAutoSize(true);
          S->SetZOrder(6);
        }
        FinisherPromptBG->SetBrushColor(FLinearColor(0.6f, 0.05f, 0.05f, 0.85f));
        FinisherPromptBG->SetPadding(FMargin(18.f, 10.f));
        FinisherPromptBG->SetVisibility(ESlateVisibility::Collapsed);

        FinisherPromptText = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), FName("FinisherPromptText"));
        if (FinisherPromptText)
        {
          FinisherPromptText->SetText(FText::FromString(TEXT("[F]  Corps-a-corps")));
          FSlateFontInfo FF = FinisherPromptText->GetFont();
          FF.Size = 17;
          FinisherPromptText->SetFont(FF);
          FinisherPromptText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
          FinisherPromptBG->SetContent(FinisherPromptText);
        }
      }
    }
  }
}

void URevenantOpsHUD::NativeTick(const FGeometry &MyGeometry,
                                  float InDeltaTime) {
  Super::NativeTick(MyGeometry, InDeltaTime);

  if (!CachedCharacter) {
    return;
  }

  UpdateHealthDisplay();
  UpdateStaminaDisplay();
  UpdateAmmoDisplay();
  UpdateCrosshair();
  UpdateLowHealthVignette();
  UpdateHitMarker(InDeltaTime);
  UpdateMercenairesDisplay();
  UpdateWaveDisplay();
  UpdateReloadBar();
  UpdateDamageDirection(InDeltaTime);
  UpdateKillNotification(InDeltaTime);
}

// =============================================================================
// HEALTH & SHIELD
// =============================================================================

void URevenantOpsHUD::UpdateHealthDisplay() {
  if (!CachedHealthComp) {
    return;
  }

  if (HealthBar) {
    const float HP = CachedHealthComp->GetHealthPercent();
    HealthBar->SetPercent(HP);
    // Vert (≥50%) → Jaune (25-50%) → Rouge (≤25%)
    FLinearColor BarColor;
    if (HP >= 0.5f)
      BarColor = FLinearColor::LerpUsingHSV(FLinearColor(1.f, 1.f, 0.f), FLinearColor(0.1f, 0.9f, 0.1f), (HP - 0.5f) * 2.f);
    else if (HP >= 0.25f)
      BarColor = FLinearColor::LerpUsingHSV(FLinearColor(0.9f, 0.1f, 0.1f), FLinearColor(1.f, 1.f, 0.f), (HP - 0.25f) * 4.f);
    else
      BarColor = FLinearColor(0.9f, 0.1f, 0.1f);
    HealthBar->SetFillColorAndOpacity(BarColor);
  }

  if (ShieldBar) {
    const float ShieldPct = CachedHealthComp->GetShieldPercent();
    ShieldBar->SetPercent(ShieldPct);
    // Hide shield bar if no shield system
    ShieldBar->SetVisibility(CachedHealthComp->GetCurrentShield() > 0.f ||
                                     CachedHealthComp->GetShieldPercent() > 0.f
                                 ? ESlateVisibility::Visible
                                 : ESlateVisibility::Collapsed);
  }
}

// =============================================================================
// STAMINA
// =============================================================================

void URevenantOpsHUD::UpdateStaminaDisplay() {
  if (StaminaBar && CachedCharacter) {
    StaminaBar->SetPercent(CachedCharacter->GetStaminaPercent());
  }
}

// =============================================================================
// AMMO
// =============================================================================

void URevenantOpsHUD::UpdateAmmoDisplay() {
  AWeaponBase *Weapon =
      CachedCharacter ? CachedCharacter->GetCurrentWeapon() : nullptr;

  if (!Weapon) {
    if (AmmoCurrentText) {
      AmmoCurrentText->SetText(FText::FromString(TEXT("--")));
    }
    if (AmmoReserveText) {
      AmmoReserveText->SetText(FText::FromString(TEXT("--")));
    }
    if (WeaponNameText) {
      WeaponNameText->SetText(FText::GetEmpty());
    }
    return;
  }

  if (AmmoCurrentText) {
    AmmoCurrentText->SetText(
        FText::FromString(FString::Printf(TEXT("%d"), Weapon->GetCurrentAmmo())));
  }

  if (AmmoReserveText) {
    AmmoReserveText->SetText(
        FText::FromString(FString::Printf(TEXT("%d"), Weapon->GetCurrentReserveAmmo())));
  }

  if (WeaponNameText) {
    WeaponNameText->SetText(Weapon->GetWeaponName());
  }
}

// =============================================================================
// CROSSHAIR
// =============================================================================

void URevenantOpsHUD::UpdateCrosshair() {
  if (!CachedCharacter) return;
  if (!CrosshairTop || !CrosshairBottom || !CrosshairLeft || !CrosshairRight) return;

  AWeaponBase* Weapon = CachedCharacter->GetCurrentWeapon();

  // Calcul du gap cible selon la dispersion
  float TargetGap = CrosshairGapMin;
  if (Weapon) {
    const float SpreadAlpha = FMath::GetMappedRangeValueClamped(
        FVector2D(0.f, 10.f), FVector2D(0.f, 1.f), Weapon->GetCurrentSpread());
    TargetGap = FMath::Lerp(CrosshairGapMin, CrosshairGapMax, SpreadAlpha);
  }

  // Réduire davantage en ADS
  if (CachedCharacter->IsAiming()) {
    TargetGap = CrosshairGapMin * 0.5f;
  }

  // Interpolation fluide (style CS : réactif mais pas instantané)
  const float DeltaTime = GetWorld()->GetDeltaSeconds();
  CrosshairCurrentGap = FMath::FInterpTo(CrosshairCurrentGap, TargetGap,
                                          DeltaTime, CrosshairInterpSpeed);

  const float G = CrosshairCurrentGap;
  const float L = CrosshairLineLength;
  const float T = CrosshairLineThickness;

  // Repositionner chaque trait via son canvas slot
  auto MoveSlot = [](UImage* Img, FVector2D Pos, FVector2D Size) {
    if (!Img) return;
    if (UCanvasPanelSlot* S = Cast<UCanvasPanelSlot>(Img->Slot)) {
      S->SetPosition(Pos);
      S->SetSize(Size);
      S->SetAlignment(FVector2D(0.5f, 0.5f));
    }
  };

  MoveSlot(CrosshairTop,    FVector2D(0.f, -(G + L * 0.5f)), FVector2D(T, L));
  MoveSlot(CrosshairBottom, FVector2D(0.f,   G + L * 0.5f),  FVector2D(T, L));
  MoveSlot(CrosshairLeft,   FVector2D(-(G + L * 0.5f), 0.f), FVector2D(L, T));
  MoveSlot(CrosshairRight,  FVector2D(  G + L * 0.5f,  0.f), FVector2D(L, T));

  // Opacité : légèrement réduite en ADS (le viseur disparaît presque)
  const float Alpha = CachedCharacter->IsAiming() ? 0.4f : 0.9f;
  const FLinearColor Color(1.f, 1.f, 1.f, Alpha);
  CrosshairTop->SetColorAndOpacity(Color);
  CrosshairBottom->SetColorAndOpacity(Color);
  CrosshairLeft->SetColorAndOpacity(Color);
  CrosshairRight->SetColorAndOpacity(Color);
}

// =============================================================================
// LOW HEALTH VIGNETTE
// =============================================================================

void URevenantOpsHUD::UpdateLowHealthVignette() {
  if (!LowHealthVignette || !CachedHealthComp) {
    return;
  }

  const float HealthPct = CachedHealthComp->GetHealthPercent();

  if (HealthPct < LowHealthThreshold) {
    // Pulse effect: map health to opacity
    const float Alpha =
        FMath::GetMappedRangeValueClamped(
            FVector2D(0.f, LowHealthThreshold), FVector2D(0.8f, 0.f),
            HealthPct);

    // Add a pulsing effect
    const float Pulse =
        (FMath::Sin(GetWorld()->GetTimeSeconds() * 4.f) + 1.f) * 0.5f;
    const float FinalAlpha = Alpha * FMath::Lerp(0.6f, 1.0f, Pulse);

    LowHealthVignette->SetOpacity(FinalAlpha);
    LowHealthVignette->SetVisibility(ESlateVisibility::HitTestInvisible);
  } else {
    LowHealthVignette->SetVisibility(ESlateVisibility::Collapsed);
  }
}

// =============================================================================
// HIT MARKER
// =============================================================================

void URevenantOpsHUD::UpdateHitMarker(float DeltaTime) {
  if (HitMarkerTimer <= 0.f) return;

  HitMarkerTimer -= DeltaTime;
  const float T = FMath::Clamp(HitMarkerTimer / HitMarkerDuration, 0.f, 1.f);

  // Lerp rouge → blanc au fil du fade
  const FLinearColor HitColor  (1.f, 0.15f, 0.15f, 1.f);
  const FLinearColor IdleColor (1.f, 1.f,   1.f,   0.9f);
  const FLinearColor Current = FLinearColor::LerpUsingHSV(IdleColor, HitColor, T);

  for (UImage* Line : {CrosshairTop, CrosshairBottom, CrosshairLeft, CrosshairRight})
    if (Line) Line->SetColorAndOpacity(Current);

  if (HitMarkerTimer <= 0.f)
  {
    // Retour à la couleur normale du viseur
    for (UImage* Line : {CrosshairTop, CrosshairBottom, CrosshairLeft, CrosshairRight})
      if (Line) Line->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.9f));
  }
}

void URevenantOpsHUD::ShowHitMarker() {
  HitMarkerTimer = HitMarkerDuration;
  // Flash immédiat rouge
  const FLinearColor HitColor(1.f, 0.15f, 0.15f, 1.f);
  for (UImage* Line : {CrosshairTop, CrosshairBottom, CrosshairLeft, CrosshairRight})
    if (Line) Line->SetColorAndOpacity(HitColor);
  // Cacher l'ancien carré
  if (HitMarkerImage) HitMarkerImage->SetVisibility(ESlateVisibility::Collapsed);
}

void URevenantOpsHUD::ShowDamageDirection(const FVector &DamageOrigin) {
  if (!CachedCharacter) {
    return;
  }

  const FVector CharacterLocation = CachedCharacter->GetActorLocation();
  const FVector DamageDir =
      (DamageOrigin - CharacterLocation).GetSafeNormal2D();
  const FVector CharForward =
      CachedCharacter->GetActorForwardVector().GetSafeNormal2D();

  // Calculate angle between forward and damage direction
  const float Dot = FVector::DotProduct(CharForward, DamageDir);
  const float Cross = FVector::CrossProduct(CharForward, DamageDir).Z;
  const float AngleRad = FMath::Atan2(Cross, Dot);

  DamageDirectionAngle = FMath::RadiansToDegrees(AngleRad);
  DamageDirectionTimer = DamageDirectionDuration;

  if (DamageDirectionImage) {
    // La barre est positionnée à 85px au-dessus du centre.
    // On la fait pivoter autour de l'ancre (0.5, 0.5) du widget = son centre.
    // Le pivot du Canvas slot est le centre de l'écran → SetRenderTransformAngle
    // suffit car le widget est déjà décalé vers le haut dans son slot.
    FWidgetTransform T;
    T.Angle = DamageDirectionAngle;
    T.Scale  = FVector2D(1.f, 1.f);
    DamageDirectionImage->SetRenderTransform(T);
    DamageDirectionImage->SetRenderTransformPivot(FVector2D(0.5f, 3.4f)); // pivot = centre écran
    DamageDirectionImage->SetOpacity(1.f);
    DamageDirectionImage->SetVisibility(ESlateVisibility::HitTestInvisible);
  }
}

// =============================================================================
// WAVE DISPLAY
// =============================================================================

void URevenantOpsHUD::UpdateWaveDisplay() {
  if (!WaveText || !CachedWaveSpawner) {
    return;
  }

  const int32 Current = CachedWaveSpawner->GetCurrentWaveNumber();
  const int32 Total = CachedWaveSpawner->GetTotalWaves();

  if (Current > 0) {
    WaveText->SetText(FText::FromString(
        FString::Printf(TEXT("Wave %d/%d"), Current, Total)));
    WaveText->SetVisibility(ESlateVisibility::HitTestInvisible);
  } else {
    WaveText->SetVisibility(ESlateVisibility::Collapsed);
  }
}

// =============================================================================
// RELOAD BAR
// =============================================================================

void URevenantOpsHUD::UpdateReloadBar() {
  if (!ReloadBar || !CachedCharacter) {
    return;
  }

  AWeaponBase *Weapon = CachedCharacter->GetCurrentWeapon();
  if (Weapon && Weapon->GetCurrentState() == EWeaponState::Reloading) {
    ReloadBar->SetPercent(Weapon->GetReloadProgress());
    ReloadBar->SetVisibility(ESlateVisibility::HitTestInvisible);
  } else {
    ReloadBar->SetVisibility(ESlateVisibility::Collapsed);
  }
}

// =============================================================================
// DAMAGE DIRECTION
// =============================================================================

void URevenantOpsHUD::UpdateDamageDirection(float DeltaTime) {
  if (!DamageDirectionImage) {
    return;
  }

  if (DamageDirectionTimer > 0.f) {
    DamageDirectionTimer -= DeltaTime;
    const float Alpha =
        FMath::Clamp(DamageDirectionTimer / DamageDirectionDuration, 0.f, 1.f);
    DamageDirectionImage->SetOpacity(Alpha);
  } else {
    DamageDirectionImage->SetVisibility(ESlateVisibility::Collapsed);
  }
}

// =============================================================================
// KILL NOTIFICATION
// =============================================================================

void URevenantOpsHUD::UpdateKillNotification(float DeltaTime) {
  if (!KillNotificationText) {
    return;
  }

  if (KillNotificationTimer > 0.f) {
    KillNotificationTimer -= DeltaTime;
    const float Alpha = FMath::Clamp(KillNotificationTimer / KillNotificationDuration, 0.f, 1.f);
    KillNotificationText->SetOpacity(Alpha);
  } else if (KillNotificationText->GetVisibility() != ESlateVisibility::Collapsed) {
    KillNotificationText->SetVisibility(ESlateVisibility::Collapsed);
  }
}

void URevenantOpsHUD::ShowKillNotification(const FString &EnemyName,
                                            int32 Points) {
  if (KillNotificationText) {
    KillNotificationText->SetText(FText::FromString(
        FString::Printf(TEXT("+%d  %s"), Points, *EnemyName)));
    KillNotificationTimer = KillNotificationDuration;
    KillNotificationText->SetOpacity(1.f);
    KillNotificationText->SetVisibility(ESlateVisibility::HitTestInvisible);
  }
}

// =============================================================================
// MERCENAIRES DISPLAY (Timer, Score, Combo)
// =============================================================================

void URevenantOpsHUD::UpdateMercenairesDisplay() {
  if (!CachedGameState) {
    CachedGameState = GetWorld()->GetGameState<AMercenairesGameState>();
    if (!CachedGameState) {
      return;
    }
  }

  // Timer (MM:SS format)
  if (TimerText) {
    const float TimeLeft = CachedGameState->GetTimeRemaining();
    const int32 Minutes = FMath::FloorToInt(TimeLeft / 60.f);
    const int32 Seconds = FMath::FloorToInt(FMath::Fmod(TimeLeft, 60.f));
    TimerText->SetText(FText::FromString(
        FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));

    // Flash red when under 30 seconds
    if (TimeLeft < 30.f) {
      const float Pulse =
          (FMath::Sin(GetWorld()->GetTimeSeconds() * 6.f) + 1.f) * 0.5f;
      TimerText->SetColorAndOpacity(
          FSlateColor(FLinearColor::LerpUsingHSV(
              FLinearColor::Red, FLinearColor::White, Pulse)));
    } else {
      TimerText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    }
  }

  // Score
  if (ScoreText) {
    ScoreText->SetText(FText::FromString(
        FString::Printf(TEXT("%d"), CachedGameState->GetCurrentScore())));
  }

  // Combo multiplier
  if (ComboText) {
    const int32 Combo = CachedGameState->GetComboMultiplier();
    if (Combo > 1) {
      ComboText->SetText(FText::FromString(
          FString::Printf(TEXT("x%d"), Combo)));
      ComboText->SetVisibility(ESlateVisibility::HitTestInvisible);
    } else {
      ComboText->SetVisibility(ESlateVisibility::Collapsed);
    }
  }

  // Combo timer bar
  if (ComboTimerBar) {
    const float ComboTime = CachedGameState->GetComboTimeRemaining();
    if (ComboTime > 0.f) {
      ComboTimerBar->SetPercent(ComboTime / 5.0f); // 5s window
      ComboTimerBar->SetVisibility(ESlateVisibility::HitTestInvisible);
    } else {
      ComboTimerBar->SetVisibility(ESlateVisibility::Collapsed);
    }
  }
}

// =============================================================================
// PICKUP PROMPT RE5
// =============================================================================

void URevenantOpsHUD::ShowPickupPrompt(UTexture2D* Icon, const FText& Name, int32 Qty)
{
  if (!PickupPromptBG) return;

  if (PickupPromptIcon)
  {
    if (Icon)
    {
      PickupPromptIcon->SetBrushFromTexture(Icon, true);
      PickupPromptIcon->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }
    else
    {
      PickupPromptIcon->SetVisibility(ESlateVisibility::Collapsed);
    }
  }

  if (PickupPromptName)
  {
    FString Str = Name.ToString();
    if (Qty > 1) Str += FString::Printf(TEXT("  x%d"), Qty);
    PickupPromptName->SetText(FText::FromString(Str));
  }

  // HitTestInvisible = visible à l'écran mais ne capte JAMAIS le clavier/souris
  PickupPromptBG->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void URevenantOpsHUD::HidePickupPrompt()
{
  if (PickupPromptBG)
    PickupPromptBG->SetVisibility(ESlateVisibility::Collapsed);
}

void URevenantOpsHUD::ShowFinisherPrompt()
{
  if (FinisherPromptBG)
    FinisherPromptBG->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void URevenantOpsHUD::HideFinisherPrompt()
{
  if (FinisherPromptBG)
    FinisherPromptBG->SetVisibility(ESlateVisibility::Collapsed);
}
