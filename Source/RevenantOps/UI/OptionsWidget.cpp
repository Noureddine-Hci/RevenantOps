// Copyright RevenantOps. All Rights Reserved.
#include "UI/OptionsWidget.h"
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

    UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>();
    WidgetTree->RootWidget = Root;

    // Background
    UBorder* Bg = WidgetTree->ConstructWidget<UBorder>();
    Bg->SetBrushColor(FLinearColor(0.02f, 0.03f, 0.06f, 0.96f));
    UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Bg);
    BgSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
    BgSlot->SetOffsets(FMargin(0.f));

    // Center panel
    UVerticalBox* VBox = WidgetTree->ConstructWidget<UVerticalBox>();
    UCanvasPanelSlot* VSlot = Root->AddChildToCanvas(VBox);
    VSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
    VSlot->SetAlignment(FVector2D(0.5f, 0.5f));
    VSlot->SetSize(FVector2D(600.f, 520.f));

    // Title
    UTextBlock* Title = WidgetTree->ConstructWidget<UTextBlock>();
    Title->SetText(FText::FromString("OPTIONS - TOUCHES"));
    FSlateFontInfo TitleFont = Title->GetFont();
    TitleFont.Size = 26;
    Title->SetFont(TitleFont);
    Title->SetColorAndOpacity(FSlateColor(FLinearColor(0.7f, 0.9f, 1.f, 1.f)));
    UVerticalBoxSlot* TitleSlot = VBox->AddChildToVerticalBox(Title);
    TitleSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 20.f));
    TitleSlot->SetHorizontalAlignment(HAlign_Center);

    // Column headers
    UHorizontalBox* Headers = WidgetTree->ConstructWidget<UHorizontalBox>();
    UVerticalBoxSlot* HeaderSlot = VBox->AddChildToVerticalBox(Headers);
    HeaderSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 8.f));

    auto MakeHeaderText = [&](const FString& Label, float Width) -> UTextBlock*
    {
        UTextBlock* T = WidgetTree->ConstructWidget<UTextBlock>();
        T->SetText(FText::FromString(Label));
        FSlateFontInfo F = T->GetFont();
        F.Size = 13;
        T->SetFont(F);
        T->SetColorAndOpacity(FSlateColor(FLinearColor(0.6f, 0.6f, 0.6f, 1.f)));
        UHorizontalBoxSlot* S = Headers->AddChildToHorizontalBox(T);
        S->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        S->SetPadding(FMargin(4.f, 0.f));
        (void)Width;
        return T;
    };
    MakeHeaderText("ACTION", 280.f);
    MakeHeaderText("TOUCHE", 160.f);
    MakeHeaderText("", 120.f);

    // Scrollable row container
    UScrollBox* Scroll = WidgetTree->ConstructWidget<UScrollBox>();
    UVerticalBoxSlot* ScrollSlot = VBox->AddChildToVerticalBox(Scroll);
    ScrollSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
    ScrollSlot->SetPadding(FMargin(0.f, 0.f, 0.f, 16.f));

    RowContainer = WidgetTree->ConstructWidget<UVerticalBox>();
    UScrollBoxSlot* RowScrollSlot = Cast<UScrollBoxSlot>(Scroll->AddChild(RowContainer));

    // Back button
    BtnBack = WidgetTree->ConstructWidget<UButton>();
    UTextBlock* BackLbl = WidgetTree->ConstructWidget<UTextBlock>();
    BackLbl->SetText(FText::FromString("< Retour"));
    FSlateFontInfo BackFont = BackLbl->GetFont();
    BackFont.Size = 16;
    BackLbl->SetFont(BackFont);
    BackLbl->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    BtnBack->AddChild(BackLbl);

    FSlateBrush BackNormal;
    BackNormal.TintColor = FSlateColor(FLinearColor(0.1f, 0.1f, 0.2f, 1.f));
    FButtonStyle BackStyle;
    BackStyle.SetNormal(BackNormal);
    FSlateBrush BackHover = BackNormal;
    BackHover.TintColor = FSlateColor(FLinearColor(0.2f, 0.2f, 0.4f, 1.f));
    BackStyle.SetHovered(BackHover);
    BackStyle.SetPressed(BackNormal);
    BtnBack->SetStyle(BackStyle);

    UVerticalBoxSlot* BackSlot = VBox->AddChildToVerticalBox(BtnBack);
    BackSlot->SetHorizontalAlignment(HAlign_Left);
    BackSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
}

// ─────────────────────────────────────────────────────────────────────────────

void UOptionsWidget::PopulateBindings(const TArray<FKeyRebindEntry>& Bindings)
{
    CachedBindings = Bindings;
    Rows.SetNum(Bindings.Num());

    if (!RowContainer) return;
    RowContainer->ClearChildren();

    for (int32 i = 0; i < Bindings.Num(); ++i)
    {
        // Row background
        UBorder* RowBg = WidgetTree->ConstructWidget<UBorder>();
        RowBg->SetBrushColor(i % 2 == 0
            ? FLinearColor(0.05f, 0.05f, 0.1f, 1.f)
            : FLinearColor(0.07f, 0.07f, 0.13f, 1.f));

        UVerticalBoxSlot* RowSlot = RowContainer->AddChildToVerticalBox(RowBg);
        RowSlot->SetPadding(FMargin(0.f, 2.f));

        UHorizontalBox* HBox = WidgetTree->ConstructWidget<UHorizontalBox>();
        UBorderSlot* BSlot = Cast<UBorderSlot>(RowBg->AddChild(HBox));
        if (BSlot) BSlot->SetPadding(FMargin(8.f, 6.f));

        // Action name
        UTextBlock* ActionLbl = WidgetTree->ConstructWidget<UTextBlock>();
        ActionLbl->SetText(Bindings[i].DisplayName);
        FSlateFontInfo F = ActionLbl->GetFont();
        F.Size = 15;
        ActionLbl->SetFont(F);
        ActionLbl->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        UHorizontalBoxSlot* ActionSlot = HBox->AddChildToHorizontalBox(ActionLbl);
        ActionSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
        ActionSlot->SetVerticalAlignment(VAlign_Center);

        // Current key text
        UTextBlock* KeyTxt = WidgetTree->ConstructWidget<UTextBlock>();
        KeyTxt->SetColorAndOpacity(FSlateColor(FLinearColor(1.f, 0.85f, 0.3f, 1.f)));
        FSlateFontInfo KF = KeyTxt->GetFont();
        KF.Size = 15;
        KeyTxt->SetFont(KF);
        UHorizontalBoxSlot* KeySlot = HBox->AddChildToHorizontalBox(KeyTxt);
        KeySlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
        KeySlot->SetPadding(FMargin(16.f, 0.f));
        KeySlot->SetVerticalAlignment(VAlign_Center);

        // Change button
        UButton* ChangeBtn = WidgetTree->ConstructWidget<UButton>();
        UTextBlock* ChangeLbl = WidgetTree->ConstructWidget<UTextBlock>();
        FSlateFontInfo CF = ChangeLbl->GetFont();
        CF.Size = 12;
        ChangeLbl->SetFont(CF);
        ChangeLbl->SetText(FText::FromString("Changer"));
        ChangeLbl->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        ChangeBtn->AddChild(ChangeLbl);

        FSlateBrush BtnNormal;
        BtnNormal.TintColor = FSlateColor(FLinearColor(0.15f, 0.3f, 0.55f, 1.f));
        FButtonStyle BtnStyle;
        BtnStyle.SetNormal(BtnNormal);
        FSlateBrush BtnHover = BtnNormal;
        BtnHover.TintColor = FSlateColor(FLinearColor(0.25f, 0.45f, 0.75f, 1.f));
        BtnStyle.SetHovered(BtnHover);
        BtnStyle.SetPressed(BtnNormal);
        ChangeBtn->SetStyle(BtnStyle);

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
        Rows[Index].ChangeLbl->SetText(FText::FromString("Appuie une touche..."));
        Rows[Index].ChangeLbl->SetColorAndOpacity(
            FSlateColor(FLinearColor(1.f, 0.5f, 0.1f, 1.f)));
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
        Rows[ListeningIndex].ChangeLbl->SetText(FText::FromString("Changer"));
        Rows[ListeningIndex].ChangeLbl->SetColorAndOpacity(
            FSlateColor(FLinearColor::White));
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
    if (bListening) { StopListening(true); return; }
    OnBackClicked.Broadcast();
}
