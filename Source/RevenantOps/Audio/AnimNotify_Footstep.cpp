// Copyright RevenantOps. All Rights Reserved.

#include "Audio/AnimNotify_Footstep.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"

void UAnimNotify_Footstep::Notify(USkeletalMeshComponent* MeshComp,
                                   UAnimSequenceBase* Animation,
                                   const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!FootstepSound || !MeshComp || !MeshComp->GetOwner()) return;

    // Position du pied au sol — on prend la position de l'acteur (capsule base)
    const FVector Location = MeshComp->GetOwner()->GetActorLocation();

    UGameplayStatics::PlaySoundAtLocation(
        MeshComp->GetOwner(),
        FootstepSound,
        Location,
        VolumeMultiplier,
        PitchMultiplier);
}
