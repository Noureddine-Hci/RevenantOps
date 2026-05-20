// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PhotoModeComponent.generated.h"

class APlayerController;
class UUserWidget;

/**
 *  Photo Mode — touche F11 par défaut.
 *
 *  Comportement :
 *    - Gèle le temps (TimeDilation = 0)
 *    - Cache le HUD
 *    - Détache la caméra du personnage (caméra libre WASD + souris)
 *    - F11 à nouveau ou Échap pour quitter
 *
 *  Attach ce composant au PlayerController ou au Character.
 *  L'input "PhotoMode" doit être bindé dans l'Enhanced Input mapping.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class REVENANTOPS_API UPhotoModeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UPhotoModeComponent();

    /** Active/désactive le mode photo. Toggle si pas de paramètre */
    UFUNCTION(BlueprintCallable, Category = "Photo Mode")
    void TogglePhotoMode();

    UFUNCTION(BlueprintCallable, Category = "Photo Mode")
    void EnterPhotoMode();

    UFUNCTION(BlueprintCallable, Category = "Photo Mode")
    void ExitPhotoMode();

    UFUNCTION(BlueprintCallable, Category = "Photo Mode")
    bool IsPhotoModeActive() const { return bIsActive; }

    /** Vitesse de déplacement de la caméra libre (cm/s) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo Mode")
    float FreeCamSpeed = 800.f;

    /** Sensibilité de rotation de la caméra libre */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Photo Mode")
    float FreeCamSensitivity = 1.0f;

protected:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    bool bIsActive = false;

    /** TimeDilation sauvegardé pour le restaurer en sortie */
    float SavedTimeDilation = 1.f;

    /** Pawn possédé sauvegardé (on repossède en sortie) */
    UPROPERTY()
    APawn* SavedPawn = nullptr;

    /** Caméra spectator créée pour le mode */
    UPROPERTY()
    APawn* FreeCamPawn = nullptr;

    APlayerController* GetPC() const;
};
