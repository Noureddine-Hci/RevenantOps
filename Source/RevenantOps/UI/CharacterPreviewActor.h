// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CharacterPreviewActor.generated.h"

class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class USkeletalMeshComponent;
class USceneComponent;

/**
 *  Actor de prévisualisation 3D pour le CharacterSelectWidget.
 *  Capture le personnage dans une RenderTarget via SceneCapture2D.
 */
UCLASS(Blueprintable)
class REVENANTOPS_API ACharacterPreviewActor : public AActor
{
    GENERATED_BODY()

public:
    ACharacterPreviewActor();

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* SceneRoot;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* PreviewMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneCaptureComponent2D* SceneCapture;

    /** RenderTarget dans laquelle le portrait est capturé */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview")
    UTextureRenderTarget2D* RenderTarget = nullptr;

    /** Taille de la RenderTarget (largeur) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview",
              meta = (ClampMin = 64, ClampMax = 1024))
    int32 CaptureWidth = 512;

    /** Taille de la RenderTarget (hauteur) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Preview",
              meta = (ClampMin = 64, ClampMax = 1024))
    int32 CaptureHeight = 910;

    /** Initialise la RenderTarget et démarre la capture */
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void InitCapture();

    /** Retourne la RenderTarget du portrait */
    UFUNCTION(BlueprintCallable, Category = "Preview")
    UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget; }

    /** Met à jour le mesh et l'AnimClass du preview */
    UFUNCTION(BlueprintCallable, Category = "Preview")
    void UpdateMesh(USkeletalMesh* NewMesh, TSubclassOf<UAnimInstance> NewAnimClass);
};
