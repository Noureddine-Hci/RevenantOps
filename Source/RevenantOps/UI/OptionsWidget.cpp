// Copyright RevenantOps. All Rights Reserved.
#include "UI/OptionsWidget.h"
#include "UI/UITheme.h"
#include "UI/UIHelpers.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/BorderSlot.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Blueprint/WidgetTree.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "EnhancedActionKeyMapping.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "Kismet/GameplayStatics.h"
#include "UI/KeyRebindSaveGame.h"

// ─────────────────────────────────────────────────────────────────────────────


// ─────────────────────────────────────────────────────────────────────────────

void UKeyBindButtonHandler::OnClicked()
{
    if (Parent) Parent->StartListening(Index);
}

// ─────────────────────────────────────────────────────────────────────────────

void UOptionsWidget::SetIMC(UInputMappingContext* InIMC)
{
    IMC = InIMC;
}

void UOptionsWidget::ApplyKeyToIMC(int32 Index, const FKey& NewKey)
{
    if (!IMC || !CachedBindings.IsValidIndex(Index)) return;

    const UInputAction* TargetAction = CachedBindings[Index].Action;
    if (!TargetAction) return;

    // Modifie directement la touche dans l'IMC (keyboard only, ignore gamepad)
    TArray<FEnhancedActionKeyMapping>& Mappings = const_cast<TArray<FEnhancedActionKeyMapping>&>(IMC->GetMappings());
    const FName TargetName = TargetAction->GetFName();
    for (FEnhancedActionKeyMapping& Mapping : Mappings)
    {
        if (!Mapping.Action || Mapping.Key.IsGamepadKey()) continue;
        if (Mapping.Action->GetFName() == TargetName)
        {
            Mapping.Key = NewKey;
            break;
        }
    }

    // Retire et ré-ajoute l'IMC pour forcer Enhanced Input à relire les mappings modifiés
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (auto* Sub = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
            {
                Sub->RemoveMappingContext(IMC);
                Sub->AddMappingContext(IMC, 0);
                UE_LOG(LogTemp, Warning, TEXT("[KeyRebind] IMC retiré et ré-ajouté"));
            }
        }
    }

    // Sauvegarde dans notre SaveGame
    UKeyRebindSaveGame* Save = Cast<UKeyRebindSaveGame>(
        UGameplayStatics::LoadGameFromSlot(UKeyRebindSaveGame::SlotName, UKeyRebindSaveGame::UserIndex));
    if (!Save)
        Save = Cast<UKeyRebindSaveGame>(UGameplayStatics::CreateSaveGameObject(UKeyRebindSaveGame::StaticClass()));
    if (Save)
    {
        Save->MappedKeys.Add(CachedBindings[Index].MappingName, NewKey);
        UGameplayStatics::SaveGameToSlot(Save, UKeyRebindSaveGame::SlotName, UKeyRebindSaveGame::UserIndex);
    }
}

void UOptionsWidget::LoadAndApplySavedBindings()
{
    if (!IMC) return;

    UKeyRebindSaveGame* Save = Cast<UKeyRebindSaveGame>(
        UGameplayStatics::LoadGameFromSlot(UKeyRebindSaveGame::SlotName, UKeyRebindSaveGame::UserIndex));
    if (!Save) return;

    TArray<FEnhancedActionKeyMapping>& Mappings = const_cast<TArray<FEnhancedActionKeyMapping>&>(IMC->GetMappings());
    for (int32 i = 0; i < CachedBindings.Num(); ++i)
    {
        const FKey* SavedKey = Save->MappedKeys.Find(CachedBindings[i].MappingName);
        if (!SavedKey || !CachedBindings[i].Action) continue;

        for (FEnhancedActionKeyMapping& Mapping : Mappings)
        {
            if (Mapping.Action && !Mapping.Key.IsGamepadKey() &&
                Mapping.Action->GetFName() == CachedBindings[i].Action->GetFName())
            {
                Mapping.Key = *SavedKey;
                CachedBindings[i].DefaultKey = *SavedKey;
                break;
            }
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────

TSharedRef<SWidget> UOptionsWidget::RebuildWidget()
{
    if (!IsDesignTime() && WidgetTree && !WidgetTree->RootWidget && !bUIBuilt)
    {
        BuildDefaultUI();
    }
    return Super::RebuildWidget();
}

void UOptionsWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // BuildDefaultUI may not have run yet if the WBP had an existing root widget
    if (!RowContainer && WidgetTree)
    {
        BuildDefaultUI();
    }

    if (BtnBack)
    {
        BtnBack->OnClicked.AddDynamic(this, &UOptionsWidget::HandleBack);
        BindButtonSounds(BtnBack);
    }

    // Re-populate now that the widget tree is fully constructed
    if (CachedBindings.Num() > 0)
    {
        // Applique les bindings sauvegardés AVANT de construire les lignes
        LoadAndApplySavedBindings();
        PopulateBindings(CachedBindings);
    }

    SetKeyboardFocus();
}

// ─────────────────────────────────────────────────────────────────────────────

void UOptionsWidget::BuildDefaultUI()
{
    if (!WidgetTree) return;
    bUIBuilt = true;

    // ── Theme ────────────────────────────────────────────────────────────────
    UUITheme* Th = GetTheme();
    const FLinearColor C_Bg      = Th ? Th->BgDeep      : FLinearColor(0.03f, 0.025f, 0.02f, 1.f);
    const FLinearColor C_Panel   = Th ? Th->BgPanel      : FLinearColor(0.07f, 0.06f,  0.04f, 1.f);
    const FLinearColor C_Gold    = Th ? Th->GoldTarnish  : FLinearColor(0.85f, 0.70f,  0.30f, 1.f);
    const FLinearColor C_GoldDim = Th ? Th->GoldDim      : FLinearColor(0.55f, 0.45f,  0.20f, 1.f);
    const FLinearColor C_White   = Th ? Th->WhiteText    : FLinearColor(0.95f, 0.93f,  0.88f, 1.f);
    const FLinearColor C_Grey    = Th ? Th->GreySoft     : FLinearColor(0.45f, 0.42f,  0.38f, 1.f);
    const FLinearColor C_Red     = Th ? Th->RedBlood     : FLinearColor(0.75f, 0.15f,  0.10f, 1.f);
    const FLinearColor C_BtnHov  = Th ? Th->ButtonBgHovered : FLinearColor(0.75f, 0.15f, 0.10f, 0.25f);

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Root;

    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>();
    Bg->SetBrushColor(C_Bg);
    UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Bande rouge haut
    UBorder* TopBar = WidgetTree->ConstructWidget<UBorder>();
    TopBar->SetBrushColor(C_Red);
    TopBar->SetVisibility(ESlateVisibility::HitTestInvisible);
    UCanvasPanelSlot* TopSlot = Root->AddChildToCanvas(TopBar);
    TopSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 0.f));
    TopSlot->SetOffsets(FMargin(0.f, 0.f, 0.f, 5.f));
    TopSlot->SetAutoSize(true);

    UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* VSlot = Root->AddChildToCanvas(VBox);
    VSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    VSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    VSlot->SetSize(FVector2D(620.f, 540.f));

    // Titre
    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
    Title->SetText(FText::FromString(TEXT("BRIEFING — CONFIGURATION TOUCHES")));
    Title->SetFont(UUIHelpers::GetFont(Th, 22));
    Title->SetColorAndOpacity(FSlateColor(C_Gold));
    VBox->AddChildToVerticalBox(Title)->SetPadding(FMargin(0.f, 0.f, 0.f, 18.f));

    // Trait or
    UBorder* TitleLine = WidgetTree->ConstructWidget<UBorder>();
    TitleLine->SetBrushColor(UUIHelpers::WithAlpha(C_GoldDim, 0.5f));
    VBox->AddChildToVerticalBox(TitleLine)->SetPadding(FMargin(0.f, 0.f, 0.f, 12.f));
    if (UBorderSlot* BS = Cast<UBorderSlot>(TitleLine->AddChild(WidgetTree->ConstructWidget<UTextBlock>())))
        BS->SetPadding(FMargin(0.f, 1.f));

    // En-têtes
    UHorizontalBox* Headers = WidgetTree->ConstructWidget<UHorizontalBox>();
    VBox->AddChildToVerticalBox(Headers)->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    auto MakeHeaderText = [&](const FString& Label, float Width) -> UTextBlock*
    {
        UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>();
        T->SetText(FText::FromString(Label));
        T->SetFont(UUIHelpers::GetFont(Th, 12));
        T->SetColorAndOpacity(FSlateColor(C_GoldDim));
        UHorizontalBoxSlot* S = Headers->AddChildToHorizontalBox(T);
        S->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        S->SetPadding(FMargin(4.f, 0.f));
        (void)Width;
        return T;
    };
    MakeHeaderText(TEXT("ACTION"), 280.f);
    MakeHeaderText(TEXT("TOUCHE"), 160.f);
    MakeHeaderText(TEXT(""), 120.f);

    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
    UVerticalBoxSlot* ScrollSlot = VBox->AddChildToVerticalBox(Scroll);
    ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ScrollSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));

    RowContainer = WidgetTree->ConstructWidget<UVerticalBox>();
    Cast<UScrollBoxSlot>(Scroll->AddChild(RowContainer));

    // Bouton retour
    BtnBack = WidgetTree->ConstructWidget<UButton>();
    UTextBlock* BackLbl = WidgetTree->ConstructWidget<UTextBlock>();
    BackLbl->SetText(FText::FromString(TEXT("< RETOUR")));
    BackLbl->SetFont(UUIHelpers::GetFont(Th, 15));
    BackLbl->SetColorAndOpacity(FSlateColor(C_Grey));
    BtnBack->AddChild(BackLbl);
    BtnBack->SetStyle(UUIHelpers::MakeStandardButtonStyle(Th));
    VBox->AddChildToVerticalBox(BtnBack)->SetHorizontalAlignment(HAlign_Left);
}

// ─────────────────────────────────────────────────────────────────────────────

void UOptionsWidget::PopulateBindings(const TArray<FKeyRebindEntry>& Bindings)
{
    CachedBindings = Bindings;
    Rows.SetNum(Bindings.Num());

    if (!RowContainer) return;
    RowContainer->ClearChildren();

    UUITheme* Th = GetTheme();
    const FLinearColor C_RowA  = Th ? UUIHelpers::WithAlpha(Th->BgPanel, 0.7f)  : FLinearColor(0.06f, 0.05f, 0.03f, 0.7f);
    const FLinearColor C_RowB  = Th ? UUIHelpers::WithAlpha(Th->BgPanel, 0.95f) : FLinearColor(0.08f, 0.07f, 0.05f, 0.95f);
    const FLinearColor C_White = Th ? Th->WhiteText   : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);
    const FLinearColor C_Gold  = Th ? Th->GoldTarnish : FLinearColor(0.85f, 0.70f, 0.30f, 1.f);
    const FLinearColor C_Red   = Th ? Th->RedBlood    : FLinearColor(0.75f, 0.15f, 0.10f, 1.f);

    for (int32 i = 0; i < Bindings.Num(); ++i)
    {
        UBorder* RowBg = WidgetTree->ConstructWidget<UBorder>();
        RowBg->SetBrushColor(i % 2 == 0 ? C_RowA : C_RowB);

        RowContainer->AddChildToVerticalBox(RowBg)->SetPadding(FMargin(0.f, 2.f));

        UHorizontalBox* HBox = WidgetTree->ConstructWidget<UHorizontalBox>();
        if (UBorderSlot* BSlot = Cast<UBorderSlot>(RowBg->AddChild(HBox)))
            BSlot->SetPadding(FMargin(8.f, 6.f));

        // Action name
        UTextBlock* ActionLbl = WidgetTree->ConstructWidget<UTextBlock>();
        ActionLbl->SetText(Bindings[i].DisplayName);
        ActionLbl->SetFont(UUIHelpers::GetFont(Th, 14));
        ActionLbl->SetColorAndOpacity(FSlateColor(C_White));
        UHorizontalBoxSlot* ActionSlot = HBox->AddChildToHorizontalBox(ActionLbl);
        ActionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        ActionSlot->SetVerticalAlignment(VAlign_Center);

        // Touche courante
        UTextBlock* KeyTxt = WidgetTree->ConstructWidget<UTextBlock>();
        KeyTxt->SetColorAndOpacity(FSlateColor(C_Gold));
        KeyTxt->SetFont(UUIHelpers::GetFont(Th, 14));
        UHorizontalBoxSlot* KeySlot = HBox->AddChildToHorizontalBox(KeyTxt);
        KeySlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        KeySlot->SetPadding(FMargin(16.f, 0.f));
        KeySlot->SetVerticalAlignment(VAlign_Center);

        // Bouton changer
        UButton* ChangeBtn = WidgetTree->ConstructWidget<UButton>();
        UTextBlock* ChangeLbl = WidgetTree->ConstructWidget<UTextBlock>();
        ChangeLbl->SetFont(UUIHelpers::GetFont(Th, 12));
        ChangeLbl->SetText(FText::FromString(TEXT("Changer")));
        ChangeLbl->SetColorAndOpacity(FSlateColor(C_White));
        ChangeBtn->AddChild(ChangeLbl);
        {
            FButtonStyle BtnStyle;
            BtnStyle.SetNormal (UUIHelpers::MakeSolidBrush(UUIHelpers::WithAlpha(C_Red, 0.4f)));
            BtnStyle.SetHovered(UUIHelpers::MakeSolidBrush(UUIHelpers::WithAlpha(C_Red, 0.7f)));
            BtnStyle.SetPressed(UUIHelpers::MakeSolidBrush(C_Red));
            ChangeBtn->SetStyle(BtnStyle);
        }

        // Enregistre pour hover pulse (MenuWidgetBase) + son hover
        BindButtonSounds(ChangeBtn);

        // Create a per-row handler object so each button has its own UFUNCTION + index
        UKeyBindButtonHandler* Handler = NewObject<UKeyBindButtonHandler>(this);
        Handler->Parent = this;
        Handler->Index  = i;
        BtnHandlers.Add(Handler);
        ChangeBtn->OnClicked.AddDynamic(Handler, &UKeyBindButtonHandler::OnClicked);

        UHorizontalBoxSlot* BtnSlot = HBox->AddChildToHorizontalBox(ChangeBtn);
        BtnSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        BtnSlot->SetVerticalAlignment(VAlign_Center);
        BtnSlot->SetPadding(FMargin(8.f, 0.f, 0.f, 0.f));

        Rows[i].KeyText   = KeyTxt;
        Rows[i].ChangeBtn = ChangeBtn;
        Rows[i].ChangeLbl = ChangeLbl;

        RefreshRow(i);
    }

}

void UOptionsWidget::RefreshRow(int32 Index)
{
    if (!Rows.IsValidIndex(Index) || !CachedBindings.IsValidIndex(Index)) return;

    // CachedBindings[Index].DefaultKey est maintenant la touche courante (mise à jour par ApplyKeyToIMC)
    FKey CurrentKey = CachedBindings[Index].DefaultKey;

    if (Rows[Index].KeyText)
    {
        Rows[Index].KeyText->SetText(FText::FromString(CurrentKey.GetDisplayName().ToString()));
    }
}

void UOptionsWidget::StartListening(int32 Index)
{
    bListening     = true;
    ListeningIndex = Index;
    if (Rows.IsValidIndex(Index) && Rows[Index].ChangeLbl)
    {
        Rows[Index].ChangeLbl->SetText(FText::FromString(TEXT("Appuie une touche...")));
        {
            UUITheme* Th = GetTheme();
            const FLinearColor Alert = Th ? Th->RedAlert : FLinearColor(1.f, 0.25f, 0.15f, 1.f);
            Rows[Index].ChangeLbl->SetColorAndOpacity(FSlateColor(Alert));
        }
    }
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().SetTimer(ListenTimerHandle,
            FTimerDelegate::CreateUObject(this, &UOptionsWidget::Tick_ListenForKey, 0.f),
            0.016f, true);
    }
}

void UOptionsWidget::Tick_ListenForKey(float)
{
    // La capture de touche est gérée par NativeOnPreviewKeyDown.
    // Ce timer n'est plus utilisé — on le laisse vide au cas où il serait déclenché.
    if (!bListening) return;
}

void UOptionsWidget::StopListening(bool bCancelled)
{
    if (Rows.IsValidIndex(ListeningIndex) && Rows[ListeningIndex].ChangeLbl)
    {
        Rows[ListeningIndex].ChangeLbl->SetText(FText::FromString(TEXT("Changer")));
        {
            UUITheme* Th = GetTheme();
            const FLinearColor White = Th ? Th->WhiteText : FLinearColor(0.95f, 0.93f, 0.88f, 1.f);
            Rows[ListeningIndex].ChangeLbl->SetColorAndOpacity(FSlateColor(White));
        }
    }
    bListening     = false;
    ListeningIndex = -1;

    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ListenTimerHandle);
    }
}

void UOptionsWidget::NativeDestruct()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ListenTimerHandle);
    }
    Super::NativeDestruct();
}

// ─────────────────────────────────────────────────────────────────────────────

FReply UOptionsWidget::NativeOnPreviewKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
    if (!bListening) return Super::NativeOnPreviewKeyDown(MyGeometry, InKeyEvent);

    const FKey PressedKey = InKeyEvent.GetKey();
    if (PressedKey == EKeys::Escape)
    {
        StopListening(true);
        return FReply::Handled();
    }

    if (CachedBindings.IsValidIndex(ListeningIndex))
    {
        // Applique dans l'IMC + sauvegarde + met à jour DefaultKey
        ApplyKeyToIMC(ListeningIndex, PressedKey);
        CachedBindings[ListeningIndex].DefaultKey = PressedKey;
        RefreshRow(ListeningIndex);
    }
    StopListening(false);
    return FReply::Handled();
}

// ─────────────────────────────────────────────────────────────────────────────

void UOptionsWidget::HandleBack()
{
    PlayClickSound();
    if (bListening) { StopListening(true); return; }
    OnBackClicked.Broadcast();
}
