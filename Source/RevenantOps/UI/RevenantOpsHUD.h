// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanelSlot.h"
#include "InputAction.h"
#include "RevenantOpsHUD.generated.h"

class UProgressBar;
class UTextBlock;
class UImage;
class UBorder;
class UTexture2D;
class ARevenantOpsCharacter;
class AWeaponBase;
class UHealthComponent;
class AMercenairesGameState;
class AEnemyWaveSpawner;

/**
 *  Main in-game HUD widget for RevenantOps.
 *  Displays: health, shield, stamina, ammo, crosshair, hit marker, damage direction.
 *  Create a Widget Blueprint that inherits from this class and bind the UMG elements.
 */
UCLASS(abstract)
class URevenantOpsHUD : public UUserWidget {
  GENERATED_BODY()

public:
  virtual TSharedRef<SWidget> RebuildWidget() override;
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

  /** Crosshair image (legacy — remplace par les 4 traits ci-dessous) */
  UPROPERTY(meta = (BindWidgetOptional))
  UImage *CrosshairImage;

  // 4 traits du viseur style CS
  UPROPERTY() UImage *CrosshairTop    = nullptr;
  UPROPERTY() UImage *CrosshairBottom = nullptr;
  UPROPERTY() UImage *CrosshairLeft   = nullptr;
  UPROPERTY() UImage *CrosshairRight  = nullptr;

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

  /** Wave counter text (e.g., "Wave 2/3") */
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *WaveText;

  /** Reload progress bar (visible only during reload) */
  UPROPERTY(meta = (BindWidgetOptional))
  UProgressBar *ReloadBar;

  /** Damage direction indicator arrow (rotated toward damage source) */
  UPROPERTY(meta = (BindWidgetOptional))
  UImage *DamageDirectionImage;

  /** Kill notification text ("+100 Zombie") */
  UPROPERTY(meta = (BindWidgetOptional))
  UTextBlock *KillNotificationText;

  // ========== CONFIGURATION ==========

  /** Health percentage below which the vignette appears */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Config",
            meta = (ClampMin = 0.0, ClampMax = 1.0))
  float LowHealthThreshold = 0.3f;

  /** Duration of the hit marker flash */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Config",
            meta = (ClampMin = 0.05, ClampMax = 1.0))
  float HitMarkerDuration = 0.15f;

  /** Longueur de chaque trait du viseur (pixels) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Crosshair",
            meta = (ClampMin = 2, ClampMax = 40))
  float CrosshairLineLength = 10.f;

  /** Epaisseur de chaque trait (pixels) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Crosshair",
            meta = (ClampMin = 1, ClampMax = 8))
  float CrosshairLineThickness = 2.f;

  /** Ecartement minimum entre les traits (au repos) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Crosshair",
            meta = (ClampMin = 0, ClampMax = 30))
  float CrosshairGapMin = 4.f;

  /** Ecartement maximum (dispersion max) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Crosshair",
            meta = (ClampMin = 10, ClampMax = 80))
  float CrosshairGapMax = 36.f;

  /** Vitesse d'interpolation du viseur (plus haut = plus réactif) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Crosshair",
            meta = (ClampMin = 1, ClampMax = 30))
  float CrosshairInterpSpeed = 12.f;

  // Gap courant interpolé
  float CrosshairCurrentGap = 4.f;

  // Deprecated — garde pour BindWidgetOptional
  float CrosshairBaseSize = 20.f;
  float CrosshairMaxSize  = 60.f;

  // ========== INTERNAL STATE ==========

  /** Timer for hit marker fade */
  float HitMarkerTimer = 0.f;

  /** Damage direction state */
  float DamageDirectionTimer = 0.f;
  float DamageDirectionAngle = 0.f;
  static constexpr float DamageDirectionDuration = 1.5f;

  /** Kill notification state */
  float KillNotificationTimer = 0.f;
  static constexpr float KillNotificationDuration = 1.0f;

  /** Cached references */
  UPROPERTY()
  ARevenantOpsCharacter *CachedCharacter = nullptr;

  UPROPERTY()
  UHealthComponent *CachedHealthComp = nullptr;

  UPROPERTY()
  AMercenairesGameState *CachedGameState = nullptr;

  UPROPERTY()
  AEnemyWaveSpawner *CachedWaveSpawner = nullptr;

  // ========== PROGRAMMATIC UI BUILDER ==========

  /** Creates all HUD widgets programmatically when the WBP WidgetTree is empty. */
  void BuildDefaultUI();

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

  /** Updates wave counter display */
  void UpdateWaveDisplay();

  /** Updates reload progress bar */
  void UpdateReloadBar();

  /** Updates damage direction indicator fade */
  void UpdateDamageDirection(float DeltaTime);

  /** Updates kill notification fade */
  void UpdateKillNotification(float DeltaTime);

  // ========== PICKUP PROMPT RE5 ==========

  /** Conteneur du popup — HitTestInvisible, ne bloque jamais l'input */
  UPROPERTY() UBorder*    PickupPromptBG      = nullptr;
  UPROPERTY() UImage*     PickupPromptIcon    = nullptr;
  UPROPERTY() UTextBlock* PickupPromptName    = nullptr;
  UPROPERTY() UTextBlock* PickupPromptKeyHint = nullptr; // "[E]  Prendre" — mis à jour dynamiquement

  /** Action "interagir / ramasser" — assigner IA_Interact dans le WBP Class Defaults */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD|Input")
  TObjectPtr<UInputAction> InteractAction;

  // ========== FINISHER PROMPT ==========

  UPROPERTY() UBorder*    FinisherPromptBG   = nullptr;
  UPROPERTY() UTextBlock* FinisherPromptText = nullptr;

public:
  /** Shows the hit marker (call from weapon hit event) */
  UFUNCTION(BlueprintCallable, Category = "HUD")
  void ShowHitMarker();

  /** Shows damage direction indicator */
  UFUNCTION(BlueprintCallable, Category = "HUD")
  void ShowDamageDirection(const FVector &DamageOrigin);

  /** Shows kill notification popup */
  UFUNCTION(BlueprintCallable, Category = "HUD")
  void ShowKillNotification(const FString &EnemyName, int32 Points);

  /** Affiche le popup RE5 (icone + [E] Prendre + nom) — HitTestInvisible */
  void ShowPickupPrompt(UTexture2D* Icon, const FText& Name, int32 Qty);

  /** Cache le popup RE5 */
  void HidePickupPrompt();

  /** Affiche le prompt de finisher corps-à-corps */
  void ShowFinisherPrompt();

  /** Cache le prompt de finisher */
  void HideFinisherPrompt();
};
