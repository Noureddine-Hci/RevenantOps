// Copyright RevenantOps. All Rights Reserved.
#include "UI/CharacterPreviewActor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/TextureRenderTarget2D.h"

ACharacterPreviewActor::ACharacterPreviewActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    RootComponent = SceneRoot;

    PreviewMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("PreviewMesh"));
    PreviewMesh->SetupAttachment(RootComponent);

    SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
    SceneCapture->SetupAttachment(RootComponent);
    SceneCapture->SetRelativeLocation(FVector(0.f, 200.f, 90.f));
    SceneCapture->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
}

void ACharacterPreviewActor::UpdateMesh(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimClass)
{
    if (!PreviewMesh) return;
    if (NewMesh)
        PreviewMesh->SetSkeletalMesh(NewMesh);
    if (NewAnimClass)
        PreviewMesh->SetAnimInstanceClass(NewAnimClass);
}

void ACharacterPreviewActor::InitCapture()
{
    if (!RenderTarget)
    {
        RenderTarget = NewObject<UTextureRenderTarget2D>(this);
        RenderTarget->InitAutoFormat(CaptureWidth, CaptureHeight);
        RenderTarget->UpdateResourceImmediate(true);
    }
    if (SceneCapture)
        SceneCapture->TextureTarget = RenderTarget;
}
