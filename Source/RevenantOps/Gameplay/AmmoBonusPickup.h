// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/AmmoTypes.h"
#include "Gameplay/PickupInterface.h"
#include "Gameplay/ItemDefinition.h"
#include "AmmoBonusPickup.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UTexture2D;
class ARevenantOpsCharacter;

/**
 *  Ammo pickup — ramassage manuel (touche E) avec popup icone.
 *  Implementer IPickupInterface permet de recuperer l'icone automatiquement
 *  partout (drops ennemis, caisses) sans renseigner DropIcon manuellement.
 *
 *  Configuration dans le BP :
 *    - ItemIcon        : texture a assigner UNE SEULE FOIS dans le BP
 *    - DisplayName     : ex "Munitions Pistolet"
 *    - AmmoAmount      : quantite par defaut
 *    - TargetAmmoType  : type de l'arme cible
 */
UCLASS(Blueprintable)
class AAmmoBonusPickup : public AActor, public IPickupInterface
{
  GENERATED_BODY()

public:
  AAmmoBonusPickup();

protected:
  virtual void BeginPlay() override;
  virtual void Tick(float DeltaTime) override;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  UStaticMeshComponent* PickupMesh;

  UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
  USphereComponent* CollisionSphere;

  /** Joueur en zone — raw ptr sans UPROPERTY pour eviter CDO crash */
  ARevenantOpsCharacter* PendingPlayer = nullptr;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Visual")
  bool bBobbing = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Visual")
  float BobAmplitude = 8.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Visual")
  float BobSpeed = 2.f;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Visual")
  bool bRotating = true;

  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Visual")
  float RotationSpeed = 90.f;

  float InitialZ = 0.f;
  FTimerHandle RespawnTimer;
  FTimerHandle LifetimeTimer;

  UFUNCTION()
  void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
                      bool bFromSweep, const FHitResult& SweepResult);

  UFUNCTION()
  void OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

  void HidePickup();
  void RespawnPickup();
  void OnLifetimeExpired();
  void ScanNearbyAmmoPickups(ARevenantOpsCharacter* Player);

public:
  // ── IPickupInterface ─────────────────────────────────────────────────────

  virtual UTexture2D*  GetPickupIcon_Implementation()        const override
    { return (ItemDefinition && ItemDefinition->ItemIcon) ? ItemDefinition->ItemIcon.Get() : ItemIcon.Get(); }
  virtual FText        GetPickupDisplayName_Implementation() const override
    { return (ItemDefinition && !ItemDefinition->DisplayName.IsEmpty()) ? ItemDefinition->DisplayName : DisplayName; }
  virtual int32        GetPickupDisplayAmount_Implementation() const override { return AmmoAmount; }
  virtual void         TryPickupInteract_Implementation(ARevenantOpsCharacter* Player) override;

  // ── API publique ─────────────────────────────────────────────────────────

  /** Appele par le personnage quand il appuie sur E */
  void TryPickup(ARevenantOpsCharacter* Player);

  /** Demarre le timer de vie du drop ennemi apres assignation des proprietes. */
  void StartLifetimeTimer();

  // ── Proprietes configurables (dans le BP ou au spawn) ───────────────────

  /** Son joue quand le joueur ramasse les munitions */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Audio")
  USoundBase* PickupSound = nullptr;

  /**
   *  DataAsset source de vérité (DA_Item_Ammo_Pistol, etc.).
   *  Si assigné : TargetAmmoType, ItemIcon et DisplayName viennent du DA.
   *  Si null    : fallback sur les champs manuels ci-dessous.
   */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup")
  TObjectPtr<UItemDefinition> ItemDefinition = nullptr;

  /** Icone affichee dans le popup (ignorée si ItemDefinition assigné) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup",
            meta = (EditCondition = "ItemDefinition == nullptr", EditConditionHides))
  TObjectPtr<UTexture2D> ItemIcon = nullptr;

  /** Nom affiche dans le popup (ignoré si ItemDefinition assigné) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup",
            meta = (EditCondition = "ItemDefinition == nullptr", EditConditionHides))
  FText DisplayName = FText::FromString("Munitions");

  /** Quantite de munitions donnee */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup",
            meta = (ClampMin = 1, ClampMax = 999))
  int32 AmmoAmount = 30;

  /** Temps de respawn (0 = pas de respawn) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup",
            meta = (ClampMin = 0, ClampMax = 300))
  float RespawnTime = 45.f;

  /** Type d'arme ciblee (ignoré si ItemDefinition assigné) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Drop",
            meta = (EditCondition = "ItemDefinition == nullptr", EditConditionHides))
  EAmmoType TargetAmmoType = EAmmoType::None;

  /** Duree de vie du drop ennemi (0 = pickup statique permanent) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AmmoPickup|Drop",
            meta = (ClampMin = 0.f, ClampMax = 60.f))
  float DropLifetime = 0.f;

  UFUNCTION(BlueprintImplementableEvent, Category = "AmmoPickup",
            meta = (DisplayName = "On Ammo Picked Up"))
  void BP_OnPickedUp(APawn* PickedUpBy, int32 AmmoAdded);
};
