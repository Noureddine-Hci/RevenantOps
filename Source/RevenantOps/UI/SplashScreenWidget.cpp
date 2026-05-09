// Copyright RevenantOps. All Rights Reserved.

#include "UI/SplashScreenWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Components/SizeBox.h"
#include "Engine/Texture2D.h"
#include "Blueprint/WidgetTree.h"
#include "GameFramework/PlayerController.h"

TSharedRef<SWidget> USplashScreenWidget::RebuildWidget()
{
    if (!IsDesignTime() && WidgetTree && !WidgetTree->RootWidget)
        BuildUI();
    return Super::RebuildWidget();
}

void USplashScreenWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // Nécessaire pour recevoir NativeOnKeyDown
    SetIsFocusable(true);
    SetKeyboardFocus();

    // Mode UI pour que la souris soit active et les clics reçus
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->SetShowMouseCursor(true);
        FInputModeUIOnly Mode;
        Mode.SetWidgetToFocus(TakeWidget());
        PC->SetInputMode(Mode);
    }

    if (SplashSequence.Num() > 0)
        LoadEntry(0);
}

void USplashScreenWidget::BuildUI()
{
    if (!WidgetTree) return;

    RootOverlay  = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(),       TEXT("RootOverlay"));
    BgImage      = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(),            TEXT("BgImage"));
    LogoSizeBox  = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass(),        TEXT("LogoSizeBox"));
    LogoImage    = WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(),            TEXT("LogoImage"));
    SubTextBlock = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(),    TEXT("SubTextBlock"));

    WidgetTree->RootWidget = RootOverlay;

    BgImage->SetColorAndOpacity(FLinearColor::Black);
    BgImage->SetVisibility(ESlateVisibility::HitTestInvisible);

    LogoSizeBox->SetWidthOverride(LogoSize.X);
    LogoSizeBox->SetHeightOverride(LogoSize.Y);
    LogoSizeBox->AddChild(LogoImage);

    if (UOverlaySlot* BgSlot = RootOverlay->AddChildToOverlay(BgImage))
    {
        BgSlot->SetHorizontalAlignment(HAlign_Fill);
        BgSlot->SetVerticalAlignment(VAlign_Fill);
    }
    if (UOverlaySlot* LogoSlot = RootOverlay->AddChildToOverlay(LogoSizeBox))
    {
        LogoSlot->SetHorizontalAlignment(HAlign_Center);
        LogoSlot->SetVerticalAlignment(VAlign_Center);
    }
    if (UOverlaySlot* TextSlot = RootOverlay->AddChildToOverlay(SubTextBlock))
    {
        TextSlot->SetHorizontalAlignment(HAlign_Center);
        TextSlot->SetVerticalAlignment(VAlign_Bottom);
        TextSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 80.f));
    }
}

void USplashScreenWidget::LoadEntry(int32 Index)
{
    if (!SplashSequence.IsValidIndex(Index)) { FinishSequence(); return; }
    CurrentIndex = Index;
    State        = ESplashState::FadeIn;
    StateTimer   = 0.f;

    // Réacquérir le focus à chaque nouvelle entrée pour que NativeOnKeyDown/MouseButtonDown
    // continuent de fonctionner même sur le dernier splash
    SetKeyboardFocus();

    const FSplashEntry& Entry = SplashSequence[Index];
    if (LogoImage)
    {
        if (Entry.Logo)
        {
            FSlateBrush Brush;
            Brush.SetResourceObject(Entry.Logo);
            LogoImage->SetBrush(Brush);
        }
        SetLogoOpacity(0.f);
    }
    if (SubTextBlock)
        SubTextBlock->SetText(Entry.SubText);
}

void USplashScreenWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
    Super::NativeTick(MyGeometry, InDeltaTime);
    if (State == ESplashState::Done || SplashSequence.Num() == 0) return;

    const FSplashEntry& Entry = SplashSequence[CurrentIndex];
    StateTimer += InDeltaTime;

    switch (State)
    {
    case ESplashState::FadeIn:
    {
        float Alpha = (Entry.FadeInDuration > 0.f) ? FMath::Clamp(StateTimer / Entry.FadeInDuration, 0.f, 1.f) : 1.f;
        SetLogoOpacity(Alpha);
        if (StateTimer >= Entry.FadeInDuration) { State = ESplashState::Hold; StateTimer = 0.f; }
        break;
    }
    case ESplashState::Hold:
        if (StateTimer >= Entry.HoldDuration) { State = ESplashState::FadeOut; StateTimer = 0.f; }
        break;
    case ESplashState::FadeOut:
    {
        float Alpha = (Entry.FadeOutDuration > 0.f) ? FMath::Clamp(1.f - StateTimer / Entry.FadeOutDuration, 0.f, 1.f) : 0.f;
        SetLogoOpacity(Alpha);
        if (StateTimer >= Entry.FadeOutDuration) AdvanceToNext();
        break;
    }
    default: break;
    }
}

void USplashScreenWidget::AdvanceToNext()
{
    if (bSkipping || CurrentIndex + 1 >= SplashSequence.Num())
        FinishSequence();
    else
        LoadEntry(CurrentIndex + 1);
}

void USplashScreenWidget::SetLogoOpacity(float Alpha)
{
    if (LogoImage)    LogoImage->SetOpacity(Alpha);
    if (SubTextBlock) SubTextBlock->SetOpacity(Alpha);
}

void USplashScreenWidget::FinishSequence()
{
    if (State == ESplashState::Done) return; // garde anti double-broadcast
    State = ESplashState::Done;
    OnSequenceDone.Broadcast();
}

void USplashScreenWidget::SkipAll()
{
    bSkipping = true;
    FinishSequence();
}

FReply USplashScreenWidget::NativeOnMouseButtonDown(const FGeometry& Geo, const FPointerEvent& Event)
{
    if (State != ESplashState::Done)
        AdvanceToNext();
    return FReply::Handled();
}

FReply USplashScreenWidget::NativeOnKeyDown(const FGeometry& Geo, const FKeyEvent& Event)
{
    if (State != ESplashState::Done)
        AdvanceToNext();
    return FReply::Handled();
}
