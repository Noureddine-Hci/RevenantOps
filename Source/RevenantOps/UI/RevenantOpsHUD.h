// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RevenantOpsHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class ARevenantOpsCharacter;
class AWeaponBase;
class UHealthComponent;
class AMercenairesGameState;

/**
 *  Main in-game HUD widget for RevenantOps.
 *  Displays: health, shield, stamina, ammo, crosshair, hit marker, damage direction.
 *  Create a Widget Blueprint that inherits from this class and bind the UMG elements.
 */
UCLASS(abstract)
class URevenantOpsHUD : public UUserWidget {
  GENERATED_BODY()

public:
  virtual void NativeConstruct() override;
  virtual void NativeTick(const FGeometry &MyGeometry,
                          float InDeltaTime) override;

protected:
  // ========== BOUND WIDGETS (set in Blueprint) ==========

  /** Health bar progress */
  UPROPERTY(meta = (BindWidgetOptional))
  UProgressBar *HealthBar;

  /** Shield bar progress */
  UPROPERTY(meta = (BindWidgetOptional))
  UProgressBar *ShieldBar;

  /** Stamina bar progress */
  UPROPERTY(meta = (BindWidgetOptional))
  UProgressBar *StaminaBar;

  /** Current ammo in magazine text */
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *AmmoCurrentText;

  /** Reserve ammo text */
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *AmmoReserveText;

  /** Weapon name text */
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *WeaponNameText;

  /** Crosshair image */
  UPROPERTY(meta = (BindWidgetOptional))
  UImage *CrosshairImage;

  /** Hit marker image (flash on hit) */
  UPROPERTY(meta = (BindWidgetOptional))
  UImage *HitMarkerImage;

  /** Low health vignette overlay */
  UPROPERTY(meta = (BindWidgetOptional))
  UImage *LowHealthVignette;

  // ========== MERCENAIRES HUD WIDGETS ==========

  /** Match timer text (MM:SS format) */
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *TimerText;

  /** Current score text */
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *ScoreText;

  /** Combo multiplier text (x2, x3...) */
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *ComboText;

  /** Combo timer progress bar (decays to zero) */
  UPROPERTY(meta = (BindWidgetOptional))
  UProgressBar *ComboTimerBar;

  // ========== CONFIGURATION ==========

  /** Health percentage below which the vignette appears */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Config",
            meta = (ClampMin = 0.0, ClampMax = 1.0))
  float LowHealthThreshold = 0.3f;

  /** Duration of the hit marker flash */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Config",
            meta = (ClampMin = 0.05, ClampMax = 1.0))
  float HitMarkerDuration = 0.15f;

  /** Crosshair base size (pixels) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Config",
            meta = (ClampMin = 1, ClampMax = 100))
  float CrosshairBaseSize = 20.f;

  /** Crosshair max spread size (pixels) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Config",
            meta = (ClampMin = 1, ClampMax = 200))
  float CrosshairMaxSize = 60.f;

  // ========== INTERNAL STATE ==========

  /** Timer for hit marker fade */
  float HitMarkerTimer = 0.f;

  /** Cached references */
  UPROPERTY()
  ARevenantOpsCharacter *CachedCharacter = nullptr;

  UPROPERTY()
  UHealthComponent *CachedHealthComp = nullptr;

  UPROPERTY()
  AMercenairesGameState *CachedGameState = nullptr;

  // ========== UPDATE FUNCTIONS ==========

  /** Updates health and shield bars */
  void UpdateHealthDisplay();

  /** Updates stamina bar */
  void UpdateStaminaDisplay();

  /** Updates ammo counter */
  void UpdateAmmoDisplay();

  /** Updates crosshair spread */
  void UpdateCrosshair();

  /** Updates low health vignette */
  void UpdateLowHealthVignette();

  /** Updates hit marker fade */
  void UpdateHitMarker(float DeltaTime);

  /** Updates Mercenaires timer, score, and combo display */
  void UpdateMercenairesDisplay();

public:
  /** Shows the hit marker (call from weapon hit event) */
  UFUNCTION(BlueprintCallable, Category = "HUD")
  void ShowHitMarker();

  /** Shows damage direction indicator */
  UFUNCTION(BlueprintCallable, Category = "HUD")
  void ShowDamageDirection(const FVector &DamageOrigin);
};
