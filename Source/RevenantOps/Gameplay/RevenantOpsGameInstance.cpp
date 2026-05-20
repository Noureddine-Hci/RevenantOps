// Copyright RevenantOps. All Rights Reserved.
#include "Gameplay/RevenantOpsGameInstance.h"
#include "Gameplay/SettingsSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundClass.h"
#include "Sound/SoundMix.h"
#include "AudioDevice.h"
#include "GameFramework/GameUserSettings.h"

const FString URevenantOpsGameInstance::SettingsSlotName = TEXT("Settings");

void URevenantOpsGameInstance::Init()
{
    Super::Init();

    // Charger ou créer les settings par défaut
    if (UGameplayStatics::DoesSaveGameExist(SettingsSlotName, 0))
    {
        Settings = Cast<USettingsSaveGame>(
            UGameplayStatics::LoadGameFromSlot(SettingsSlotName, 0));
    }

    if (!Settings)
    {
        Settings = Cast<USettingsSaveGame>(
            UGameplayStatics::CreateSaveGameObject(USettingsSaveGame::StaticClass()));
    }

    ApplySettings();
}

void URevenantOpsGameInstance::Shutdown()
{
    SaveSettings();
    Super::Shutdown();
}

void URevenantOpsGameInstance::SaveSettings()
{
    if (!Settings) return;
    UGameplayStatics::SaveGameToSlot(Settings, SettingsSlotName, 0);
}

void URevenantOpsGameInstance::ApplySettings()
{
    if (!Settings) return;

    // Audio : volume master via SetGlobalSoundMixVolume nécessite un SoundMix.
    // Approche simple ici : ajuster le volume global du device audio.
    if (FAudioDevice* AudioDevice = GetWorld() ? GetWorld()->GetAudioDeviceRaw() : nullptr)
    {
        AudioDevice->SetTransientPrimaryVolume(Settings->MasterVolume);
    }

    // Qualité graphique
    if (UGameUserSettings* GUS = UGameUserSettings::GetGameUserSettings())
    {
        const int32 Q = FMath::Clamp(Settings->GraphicsQuality, 0, 3);
        GUS->SetOverallScalabilityLevel(Q);

        switch (Settings->WindowMode)
        {
            case 0: GUS->SetFullscreenMode(EWindowMode::Fullscreen); break;
            case 1: GUS->SetFullscreenMode(EWindowMode::WindowedFullscreen); break;
            case 2: GUS->SetFullscreenMode(EWindowMode::Windowed); break;
            default: break;
        }
        GUS->ApplySettings(false);
    }
}
