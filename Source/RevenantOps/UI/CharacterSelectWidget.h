// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "UI/MenuWidgetBase.h"
#include "UI/MenuTypes.h"
#include "Gameplay/TalentDefinition.h"
#include "CharacterSelectWidget.generated.h"

class ACharacterPreviewActor;
class UButton;
class UTextBlock;
class UVerticalBox;
class UHorizontalBox;
class UBorder;
class UImage;
class UUniformGridPanel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterChosen, FCharacterInfo, CharacterInfo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCharacterSelectBack);

UCLASS(Blueprintable)
class REVENANTOPS_API UCharacterSelectWidget : public UMenuWidgetBase
{
    GENERATED_BODY()

public:
    UPROPERTY(BlueprintAssignable, Category = "Menu")
    FOnCharacterChosen OnCharacterChosen;

    UPROPERTY(BlueprintAssignable, Category = "Menu")
    FOnCharacterSelectBack OnBackClicked;

    void PopulateCharacters(const TArray<FCharacterInfo>& Characters);

    virtual TSharedRef<SWidget> RebuildWidget() override;
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

private:
    void BuildDefaultUI();
    void SelectCharacter(int32 Index);
    void RefreshCarousel();
    void RefreshInfo();

    UFUNCTION() void HandleBack();
    UFUNCTION() void HandlePrev();
    UFUNCTION() void HandleNext();
    UFUNCTION() void HandleConfirm();

    TArray<FCharacterInfo> CachedCharacters;
    int32 SelectedIndex = 0;

    UPROPERTY() UImage*         PortraitImage   = nullptr;
    UPROPERTY() UHorizontalBox* CarouselBox     = nullptr;
    UPROPERTY() UTextBlock*     CharNameText    = nullptr;
    UPROPERTY() UButton*        BtnPrev         = nullptr;
    UPROPERTY() UButton*        BtnNext         = nullptr;
    UPROPERTY() UButton*        BtnConfirm      = nullptr;
    UPROPERTY() UButton*        BtnBack         = nullptr;
    UPROPERTY() UVerticalBox*   TalentsVBox     = nullptr;
    UPROPERTY() UUniformGridPanel* InventoryGrid = nullptr;

    bool bUIBuilt = false;

    /** Classe du preview actor — assigne BP_CharacterPreviewActor dans le WBP */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview",
              meta = (AllowPrivateAccess = "true"))
    TSubclassOf<ACharacterPreviewActor> PreviewActorClass;

    /** Acteur de preview 3D — spawné/détruit avec le widget */
    UPROPERTY() ACharacterPreviewActor* PreviewActor = nullptr;
};
