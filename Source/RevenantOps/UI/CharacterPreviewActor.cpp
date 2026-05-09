// Copyright RevenantOps. All Rights Reserved.
#include "UI/CharacterPreviewActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/TextureRenderTarget2D.h"

ACharacterPreviewActor::ACharacterPreviewActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // ── Racine neutre ─────────────────────────────────────────────────────────
    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    // ── Mesh de prévisualisation (attaché à la racine) ────────────────────────
    PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
    PreviewMesh->SetupAttachment(SceneRoot);
    PreviewMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    PreviewMesh->bCastDynamicShadow  = false;
    PreviewMesh->CastShadow          = false;

    // ── SceneCapture2D (attaché à la racine, pas au mesh) ────────────────────
    SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
    SceneCapture->SetupAttachment(SceneRoot);

    // Position par défaut : devant le personnage (+X), regardant vers -X.
    // Cette valeur sert de point de départ dans le BP — tu peux la modifier
    // directement dans le viewport du BP sans recompiler.
    SceneCapture->SetRelativeLocation(FVector(250.f, 0.f, 100.f));
    SceneCapture->SetRelativeRotation(FRotator(-5.f, 180.f, 0.f));

    // Désactiver la capture automatique chaque frame (on capture en continu,
    // mais pas besoin du Every Frame si le perso ne bouge pas en menu).
    SceneCapture->bCaptureEveryFrame       = true;
    SceneCapture->bCaptureOnMovement       = true;
    SceneCapture->bAlwaysPersistRenderingState = true;

    SceneCapture->CaptureSource = ESceneCaptureSource::SCS_FinalColorLDR;

    // Désactiver les effets inutiles pour un portrait de menu
    SceneCapture->ShowFlags.SetAtmosphere(false);
    SceneCapture->ShowFlags.SetFog(false);
    SceneCapture->ShowFlags.SetBloom(false);
    SceneCapture->ShowFlags.SetAmbientOcclusion(false);
    SceneCapture->ShowFlags.SetMotionBlur(false);
    SceneCapture->ShowFlags.SetEyeAdaptation(false); // désactive l'auto-exposition — cause principale du perso très sombre

    // Exposition manuelle fixe : le perso sera correctement éclairé
    // Ajustable via ExposureBias dans le BP (valeur par défaut 1.0 = normal)
    SceneCapture->PostProcessSettings.bOverride_AutoExposureMethod = true;
    SceneCapture->PostProcessSettings.AutoExposureMethod           = AEM_Manual;
    SceneCapture->PostProcessSettings.bOverride_AutoExposureBias   = true;
    SceneCapture->PostProcessSettings.AutoExposureBias             = ExposureBias;

    // Appliquer les valeurs par défaut de caméra (position / FOV)
    ApplyCameraSettings();
}

void ACharacterPreviewActor::BeginPlay()
{
    Super::BeginPlay();
    InitCapture();
}

// ─────────────────────────────────────────────────────────────────────────────

void ACharacterPreviewActor::ApplyCameraSettings()
{
    if (!SceneCapture) return;

    // NE PAS toucher à la position/rotation du SceneCapture ici —
    // elles sont contrôlées directement depuis le Blueprint (viewport Components).
    // On applique uniquement les réglages post-process non éditables visuellement.

    SceneCapture->FOVAngle = CameraFOV;

    SceneCapture->PostProcessSettings.bOverride_AutoExposureMethod = true;
    SceneCapture->PostProcessSettings.AutoExposureMethod           = AEM_Manual;
    SceneCapture->PostProcessSettings.bOverride_AutoExposureBias   = true;
    SceneCapture->PostProcessSettings.AutoExposureBias             = ExposureBias;

    // Rotation du mesh pour faire face à la caméra (configurable dans Class Defaults)
    if (PreviewMesh)
        PreviewMesh->SetRelativeRotation(FRotator(0.f, MeshYawOffset, 0.f));
}

void ACharacterPreviewActor::InitCapture()
{
    if (!SceneCapture) return;

    // Créer (ou recréer) le RenderTarget aux dimensions voulues
    RenderTarget = NewObject<UTextureRenderTarget2D>(this, TEXT("PreviewRT"));
    RenderTarget->RenderTargetFormat = RTF_RGBA8;
    RenderTarget->InitAutoFormat(RenderTargetWidth, RenderTargetHeight);
    RenderTarget->UpdateResourceImmediate(true);

    SceneCapture->TextureTarget = RenderTarget;

    // Appliquer les réglages caméra au cas où BeginPlay les a changés
    ApplyCameraSettings();
}

void ACharacterPreviewActor::UpdateMesh(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimClass)
{
    if (!PreviewMesh) return;
    if (NewMesh)       PreviewMesh->SetSkeletalMesh(NewMesh);
    if (NewAnimClass)  PreviewMesh->SetAnimInstanceClass(NewAnimClass);
}
