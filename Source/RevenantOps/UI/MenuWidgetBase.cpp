// Copyright RevenantOps. All Rights Reserved.
#include "UI/MenuWidgetBase.h"
#include "UI/UIHelpers.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"

UUITheme* UMenuWidgetBase::GetTheme() const
{
    if (ThemeOverride)
        return ThemeOverride;
    return UUIHelpers::GetDefaultTheme();
}

void UMenuWidgetBase::BindButtonSounds(UButton* Btn)
{
    if (!Btn) return;
    Btn->OnHovered.AddDynamic(this, &UMenuWidgetBase::HandleHover);
}

void UMenuWidgetBase::PlayClickSound()
{
    if (SoundClick)
        UGameplayStatics::PlaySound2D(this, SoundClick);
}

void UMenuWidgetBase::HandleHover()
{
    if (SoundHover)
        UGameplayStatics::PlaySound2D(this, SoundHover);
}
