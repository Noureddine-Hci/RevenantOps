// Copyright RevenantOps. All Rights Reserved.

#include "Gameplay/PhotoModeComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpectatorPawn.h"
#include "GameFramework/HUD.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

UPhotoModeComponent::UPhotoModeComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

APlayerController* UPhotoModeComponent::GetPC() const
{
    if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
        return PC;
    if (APawn* P = Cast<APawn>(GetOwner()))
        return Cast<APlayerController>(P->GetController());
    return nullptr;
}

void UPhotoModeComponent::TogglePhotoMode()
{
    if (bIsActive) ExitPhotoMode();
    else           EnterPhotoMode();
}

void UPhotoModeComponent::EnterPhotoMode()
{
    if (bIsActive) return;

    APlayerController* PC = GetPC();
    UWorld* World = GetWorld();
    if (!PC || !World) return;

    bIsActive = true;

    // Sauvegarde + freeze time
    SavedTimeDilation = UGameplayStatics::GetGlobalTimeDilation(World);
    UGameplayStatics::SetGlobalTimeDilation(World, 0.0001f);

    // Cache HUD
    if (AHUD* HUD = PC->GetHUD())
    {
        HUD->ShowHUD();  // toggle off si actuellement on
    }

    // Sauvegarde pawn et passe en spectator
    SavedPawn = PC->GetPawn();
    const FVector  StartLoc = SavedPawn ? SavedPawn->GetActorLocation() + FVector(0, 0, 100) : FVector::ZeroVector;
    const FRotator StartRot = SavedPawn ? SavedPawn->GetActorRotation() : FRotator::ZeroRotator;

    FActorSpawnParameters SP;
    SP.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    FreeCamPawn = World->SpawnActor<ASpectatorPawn>(ASpectatorPawn::StaticClass(), StartLoc, StartRot, SP);

    if (FreeCamPawn)
    {
        // Le spectator pawn doit ticker en temps réel — bypass le TimeDilation global
        FreeCamPawn->CustomTimeDilation = 1.f / 0.0001f;
        PC->Possess(FreeCamPawn);
    }

    UE_LOG(LogTemp, Log, TEXT("[PhotoMode] Activé"));
}

void UPhotoModeComponent::ExitPhotoMode()
{
    if (!bIsActive) return;

    APlayerController* PC = GetPC();
    UWorld* World = GetWorld();
    if (!PC || !World) return;

    bIsActive = false;

    // Restore time
    UGameplayStatics::SetGlobalTimeDilation(World, SavedTimeDilation);

    // Restore pawn
    if (SavedPawn)
    {
        PC->Possess(SavedPawn);
    }

    // Détruit le spectator
    if (FreeCamPawn)
    {
        FreeCamPawn->Destroy();
        FreeCamPawn = nullptr;
    }

    // Réaffiche HUD
    if (AHUD* HUD = PC->GetHUD())
    {
        HUD->ShowHUD();
    }

    UE_LOG(LogTemp, Log, TEXT("[PhotoMode] Désactivé"));
}

void UPhotoModeComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                        FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}
