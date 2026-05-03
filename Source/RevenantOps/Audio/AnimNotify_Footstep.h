// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_Footstep.generated.h"

/**
 *  AnimNotify_Footstep
 *
 *  Placer sur chaque keyframe de pas dans les AnimSequences (Idle, Walk, Run, Sprint).
 *  Joue FootstepSound en 3D à la position du pawn.
 *
 *  Usage :
 *    1. Ouvrir une AnimSequence dans UE5
 *    2. Ajouter une piste Notify → choisir "Footstep"
 *    3. Assigner FootstepSound dans les détails de la notify
 */
UCLASS(meta = (DisplayName = "Footstep"))
class REVENANTOPS_API UAnimNotify_Footstep : public UAnimNotify
{
    GENERATED_BODY()

public:
    /** Son joué à chaque pas. Assigner un SoundCue ou SoundWave dans l'AnimSequence. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep")
    USoundBase* FootstepSound = nullptr;

    /** Volume relatif (1.0 = volume normal du son asset) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep",
              meta = (ClampMin = 0.f, ClampMax = 2.f))
    float VolumeMultiplier = 1.f;

    /** Pitch relatif */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Footstep",
              meta = (ClampMin = 0.5f, ClampMax = 2.f))
    float PitchMultiplier = 1.f;

    virtual void Notify(USkeletalMeshComponent* MeshComp,
                        UAnimSequenceBase* Animation,
                        const FAnimNotifyEventReference& EventReference) override;

    virtual FString GetNotifyName_Implementation() const override
    {
        return FootstepSound
            ? FString::Printf(TEXT("Footstep [%s]"), *FootstepSound->GetName())
            : TEXT("Footstep [no sound]");
    }
};
