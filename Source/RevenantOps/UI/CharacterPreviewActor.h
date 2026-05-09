// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterPreviewActor.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class USkeletalMeshComponent;
class USpringArmComponent;

/**
 *  Actor de prévisualisation 3D pour CharacterSelectWidget.
 *
 *  Spawn hors du level (Z=50000) — invisible au joueur.
 *  Capture le mesh dans un RenderTarget via SceneCaptureComponent2D.
 *
 *  Réglage caméra sans recompiler : modifier CameraDistance/CameraHeight/CameraFOV
 *  dans le Blueprint hérité (BP_CharacterPreviewActor).
 */
UCLASS(Blueprintable)
class REVENANTOPS_API ACharacterPreviewActor : public AActor
{
    GENERATED_BODY()

public:
    ACharacterPreviewActor();

    virtual void BeginPlay() override;

    // ── Composants ────────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview|Components")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview|Components")
    TObjectPtr<USkeletalMeshComponent> PreviewMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview|Components")
    TObjectPtr<USceneCaptureComponent2D> SceneCapture;

    /**
     *  Rotation du mesh autour de Z pour le faire face à la caméra.
     *  0   = mesh face +X (mannequin UE5 par défaut)
     *  -90 = mesh face +Y (ex : imports Mixamo/Blender souvent dans cet axe)
     *  Modifiable dans le BP sans recompiler.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview|Camera",
              meta = (ClampMin = -180.f, ClampMax = 180.f))
    float MeshYawOffset = -90.f;

    // ── RenderTarget ──────────────────────────────────────────────────────────

    /** RenderTarget produite — lire depuis le widget pour afficher le portrait */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Preview|Output")
    TObjectPtr<UTextureRenderTarget2D> RenderTarget;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview|Output",
              meta = (ClampMin = 64, ClampMax = 2048))
    int32 RenderTargetWidth = 512;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview|Output",
              meta = (ClampMin = 64, ClampMax = 2048))
    int32 RenderTargetHeight = 900;

    // ── Réglages caméra ───────────────────────────────────────────────────────
    // La POSITION et ROTATION de la caméra se définissent directement dans
    // le Blueprint en déplaçant le composant SceneCapture dans le viewport.

    /** Champ de vision de la capture (degrés) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview|Camera",
              meta = (ClampMin = 20.f, ClampMax = 120.f))
    float CameraFOV = 50.f;

    /**
     *  Compensation d'exposition (EV).
     *  0 = exposition neutre. Augmente si le perso est trop sombre, baisse si trop brillant.
     *  Modifiable dans le BP sans recompiler.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview|Camera",
              meta = (ClampMin = -4.f, ClampMax = 4.f))
    float ExposureBias = 1.0f;

    // ── API publique ──────────────────────────────────────────────────────────

    /**
     *  Crée le RenderTarget et démarre la capture.
     *  Appelé automatiquement dans BeginPlay ; appeler de nouveau si tu changes
     *  RenderTargetWidth/Height en runtime.
     */
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void InitCapture();

    /** Applique les réglages caméra actuels (Distance/Height/FOV…) au composant */
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void ApplyCameraSettings();

    /** Change le mesh et l'AnimInstance affichés dans le portrait */
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void UpdateMesh(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimClass);
};
