// Copyright RevenantOps. All Rights Reserved.

#include "RevenantOpsHUD.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MercenairesGameState.h"
#include "RevenantOpsCharacter.h"
#include "WeaponBase.h"
#include "EnemyWaveSpawner.h"

void URevenantOpsHUD::NativeConstruct() {
  Super::NativeConstruct();

  // Cache character reference
  if (APawn *Pawn = GetOwningPlayerPawn()) {
    CachedCharacter = Cast<ARevenantOpsCharacter>(Pawn);

    if (CachedCharacter) {
      // Find health component on character
      CachedHealthComp =
          CachedCharacter->FindComponentByClass<UHealthComponent>();
    }
  }

  // Initialize hit marker as hidden
  if (HitMarkerImage) {
    HitMarkerImage->SetOpacity(0.f);
  }

  // Cache game state for timer/score
  CachedGameState = GetWorld()->GetGameState<AMercenairesGameState>();

  // Cache wave spawner for wave counter
  TArray<AActor *> Spawners;
  UGameplayStatics::GetAllActorsOfClass(
      GetWorld(), AEnemyWaveSpawner::StaticClass(), Spawners);
  if (Spawners.Num() > 0) {
    CachedWaveSpawner = Cast<AEnemyWaveSpawner>(Spawners[0]);
  }

  // Initialize new HUD elements as hidden
  if (ReloadBar) {
    ReloadBar->SetVisibility(ESlateVisibility::Collapsed);
  }
  if (DamageDirectionImage) {
    DamageDirectionImage->SetVisibility(ESlateVisibility::Collapsed);
  }
  if (KillNotificationText) {
    KillNotificationText->SetVisibility(ESlateVisibility::Collapsed);
  }
}

void URevenantOpsHUD::NativeTick(const FGeometry &MyGeometry,
                                  float InDeltaTime) {
  Super::NativeTick(MyGeometry, InDeltaTime);

  if (!CachedCharacter) {
    return;
  }

  UpdateHealthDisplay();
  UpdateStaminaDisplay();
  UpdateAmmoDisplay();
  UpdateCrosshair();
  UpdateLowHealthVignette();
  UpdateHitMarker(InDeltaTime);
  UpdateMercenairesDisplay();
  UpdateWaveDisplay();
  UpdateReloadBar();
  UpdateDamageDirection(InDeltaTime);
  UpdateKillNotification(InDeltaTime);
}

// =============================================================================
// HEALTH & SHIELD
// =============================================================================

void URevenantOpsHUD::UpdateHealthDisplay() {
  if (!CachedHealthComp) {
    return;
  }

  if (HealthBar) {
    HealthBar->SetPercent(CachedHealthComp->GetHealthPercent());
  }

  if (ShieldBar) {
    const float ShieldPct = CachedHealthComp->GetShieldPercent();
    ShieldBar->SetPercent(ShieldPct);
    // Hide shield bar if no shield system
    ShieldBar->SetVisibility(CachedHealthComp->GetCurrentShield() > 0.f ||
                                     CachedHealthComp->GetShieldPercent() > 0.f
                                 ? ESlateVisibility::Visible
                                 : ESlateVisibility::Collapsed);
  }
}

// =============================================================================
// STAMINA
// =============================================================================

void URevenantOpsHUD::UpdateStaminaDisplay() {
  if (StaminaBar && CachedCharacter) {
    StaminaBar->SetPercent(CachedCharacter->GetStaminaPercent());
  }
}

// =============================================================================
// AMMO
// =============================================================================

void URevenantOpsHUD::UpdateAmmoDisplay() {
  AWeaponBase *Weapon =
      CachedCharacter ? CachedCharacter->GetCurrentWeapon() : nullptr;

  if (!Weapon) {
    if (AmmoCurrentText) {
      AmmoCurrentText->SetText(FText::FromString(TEXT("--")));
    }
    if (AmmoReserveText) {
      AmmoReserveText->SetText(FText::FromString(TEXT("--")));
    }
    if (WeaponNameText) {
      WeaponNameText->SetText(FText::GetEmpty());
    }
    return;
  }

  if (AmmoCurrentText) {
    AmmoCurrentText->SetText(
        FText::FromString(FString::Printf(TEXT("%d"), Weapon->GetCurrentAmmo())));
  }

  if (AmmoReserveText) {
    AmmoReserveText->SetText(
        FText::FromString(FString::Printf(TEXT("%d"), Weapon->GetCurrentReserveAmmo())));
  }

  if (WeaponNameText) {
    WeaponNameText->SetText(Weapon->GetWeaponName());
  }
}

// =============================================================================
// CROSSHAIR
// =============================================================================

void URevenantOpsHUD::UpdateCrosshair() {
  if (!CrosshairImage || !CachedCharacter) {
    return;
  }

  AWeaponBase *Weapon = CachedCharacter->GetCurrentWeapon();
  float SpreadAlpha = 0.f;

  if (Weapon) {
    // Map weapon spread to crosshair size
    const float CurrentSpread = Weapon->GetCurrentSpread();
    SpreadAlpha = FMath::GetMappedRangeValueClamped(
        FVector2D(0.f, 10.f), FVector2D(0.f, 1.f), CurrentSpread);
  }

  const float TargetSize =
      FMath::Lerp(CrosshairBaseSize, CrosshairMaxSize, SpreadAlpha);

  // Scale the crosshair image
  CrosshairImage->SetDesiredSizeOverride(FVector2D(TargetSize, TargetSize));

  // Tint red if over an enemy (optional - can check via line trace)
  if (CachedCharacter->IsAiming()) {
    CrosshairImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.8f));
  } else {
    CrosshairImage->SetColorAndOpacity(FLinearColor(1.f, 1.f, 1.f, 0.5f));
  }
}

// =============================================================================
// LOW HEALTH VIGNETTE
// =============================================================================

void URevenantOpsHUD::UpdateLowHealthVignette() {
  if (!LowHealthVignette || !CachedHealthComp) {
    return;
  }

  const float HealthPct = CachedHealthComp->GetHealthPercent();

  if (HealthPct < LowHealthThreshold) {
    // Pulse effect: map health to opacity
    const float Alpha =
        FMath::GetMappedRangeValueClamped(
            FVector2D(0.f, LowHealthThreshold), FVector2D(0.8f, 0.f),
            HealthPct);

    // Add a pulsing effect
    const float Pulse =
        (FMath::Sin(GetWorld()->GetTimeSeconds() * 4.f) + 1.f) * 0.5f;
    const float FinalAlpha = Alpha * FMath::Lerp(0.6f, 1.0f, Pulse);

    LowHealthVignette->SetOpacity(FinalAlpha);
    LowHealthVignette->SetVisibility(ESlateVisibility::HitTestInvisible);
  } else {
    LowHealthVignette->SetVisibility(ESlateVisibility::Collapsed);
  }
}

// =============================================================================
// HIT MARKER
// =============================================================================

void URevenantOpsHUD::UpdateHitMarker(float DeltaTime) {
  if (!HitMarkerImage) {
    return;
  }

  if (HitMarkerTimer > 0.f) {
    HitMarkerTimer -= DeltaTime;
    const float Alpha = FMath::Clamp(HitMarkerTimer / HitMarkerDuration, 0.f, 1.f);
    HitMarkerImage->SetOpacity(Alpha);
  }
}

void URevenantOpsHUD::ShowHitMarker() {
  HitMarkerTimer = HitMarkerDuration;
  if (HitMarkerImage) {
    HitMarkerImage->SetOpacity(1.f);
  }
}

void URevenantOpsHUD::ShowDamageDirection(const FVector &DamageOrigin) {
  if (!CachedCharacter) {
    return;
  }

  const FVector CharacterLocation = CachedCharacter->GetActorLocation();
  const FVector DamageDir =
      (DamageOrigin - CharacterLocation).GetSafeNormal2D();
  const FVector CharForward =
      CachedCharacter->GetActorForwardVector().GetSafeNormal2D();

  // Calculate angle between forward and damage direction
  const float Dot = FVector::DotProduct(CharForward, DamageDir);
  const float Cross = FVector::CrossProduct(CharForward, DamageDir).Z;
  const float AngleRad = FMath::Atan2(Cross, Dot);

  DamageDirectionAngle = FMath::RadiansToDegrees(AngleRad);
  DamageDirectionTimer = DamageDirectionDuration;

  if (DamageDirectionImage) {
    DamageDirectionImage->SetRenderTransformAngle(DamageDirectionAngle);
    DamageDirectionImage->SetOpacity(1.f);
    DamageDirectionImage->SetVisibility(ESlateVisibility::HitTestInvisible);
  }
}

// =============================================================================
// WAVE DISPLAY
// =============================================================================

void URevenantOpsHUD::UpdateWaveDisplay() {
  if (!WaveText || !CachedWaveSpawner) {
    return;
  }

  const int32 Current = CachedWaveSpawner->GetCurrentWaveNumber();
  const int32 Total = CachedWaveSpawner->GetTotalWaves();

  if (Current > 0) {
    WaveText->SetText(FText::FromString(
        FString::Printf(TEXT("Wave %d/%d"), Current, Total)));
    WaveText->SetVisibility(ESlateVisibility::HitTestInvisible);
  } else {
    WaveText->SetVisibility(ESlateVisibility::Collapsed);
  }
}

// =============================================================================
// RELOAD BAR
// =============================================================================

void URevenantOpsHUD::UpdateReloadBar() {
  if (!ReloadBar || !CachedCharacter) {
    return;
  }

  AWeaponBase *Weapon = CachedCharacter->GetCurrentWeapon();
  if (Weapon && Weapon->GetCurrentState() == EWeaponState::Reloading) {
    ReloadBar->SetPercent(Weapon->GetReloadProgress());
    ReloadBar->SetVisibility(ESlateVisibility::HitTestInvisible);
  } else {
    ReloadBar->SetVisibility(ESlateVisibility::Collapsed);
  }
}

// =============================================================================
// DAMAGE DIRECTION
// =============================================================================

void URevenantOpsHUD::UpdateDamageDirection(float DeltaTime) {
  if (!DamageDirectionImage) {
    return;
  }

  if (DamageDirectionTimer > 0.f) {
    DamageDirectionTimer -= DeltaTime;
    const float Alpha =
        FMath::Clamp(DamageDirectionTimer / DamageDirectionDuration, 0.f, 1.f);
    DamageDirectionImage->SetOpacity(Alpha);
  } else {
    DamageDirectionImage->SetVisibility(ESlateVisibility::Collapsed);
  }
}

// =============================================================================
// KILL NOTIFICATION
// =============================================================================

void URevenantOpsHUD::UpdateKillNotification(float DeltaTime) {
  if (!KillNotificationText) {
    return;
  }

  if (KillNotificationTimer > 0.f) {
    KillNotificationTimer -= DeltaTime;
    const float Alpha = FMath::Clamp(KillNotificationTimer / KillNotificationDuration, 0.f, 1.f);
    KillNotificationText->SetOpacity(Alpha);
  } else if (KillNotificationText->GetVisibility() != ESlateVisibility::Collapsed) {
    KillNotificationText->SetVisibility(ESlateVisibility::Collapsed);
  }
}

void URevenantOpsHUD::ShowKillNotification(const FString &EnemyName,
                                            int32 Points) {
  if (KillNotificationText) {
    KillNotificationText->SetText(FText::FromString(
        FString::Printf(TEXT("+%d  %s"), Points, *EnemyName)));
    KillNotificationTimer = KillNotificationDuration;
    KillNotificationText->SetOpacity(1.f);
    KillNotificationText->SetVisibility(ESlateVisibility::HitTestInvisible);
  }
}

// =============================================================================
// MERCENAIRES DISPLAY (Timer, Score, Combo)
// =============================================================================

void URevenantOpsHUD::UpdateMercenairesDisplay() {
  if (!CachedGameState) {
    CachedGameState = GetWorld()->GetGameState<AMercenairesGameState>();
    if (!CachedGameState) {
      return;
    }
  }

  // Timer (MM:SS format)
  if (TimerText) {
    const float TimeLeft = CachedGameState->GetTimeRemaining();
    const int32 Minutes = FMath::FloorToInt(TimeLeft / 60.f);
    const int32 Seconds = FMath::FloorToInt(FMath::Fmod(TimeLeft, 60.f));
    TimerText->SetText(FText::FromString(
        FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));

    // Flash red when under 30 seconds
    if (TimeLeft < 30.f) {
      const float Pulse =
          (FMath::Sin(GetWorld()->GetTimeSeconds() * 6.f) + 1.f) * 0.5f;
      TimerText->SetColorAndOpacity(
          FSlateColor(FLinearColor::LerpUsingHSV(
              FLinearColor::Red, FLinearColor::White, Pulse)));
    } else {
      TimerText->SetColorAndOpacity(FSlateColor(FLinearColor::White));
    }
  }

  // Score
  if (ScoreText) {
    ScoreText->SetText(FText::FromString(
        FString::Printf(TEXT("%d"), CachedGameState->GetCurrentScore())));
  }

  // Combo multiplier
  if (ComboText) {
    const int32 Combo = CachedGameState->GetComboMultiplier();
    if (Combo > 1) {
      ComboText->SetText(FText::FromString(
          FString::Printf(TEXT("x%d"), Combo)));
      ComboText->SetVisibility(ESlateVisibility::HitTestInvisible);
    } else {
      ComboText->SetVisibility(ESlateVisibility::Collapsed);
    }
  }

  // Combo timer bar
  if (ComboTimerBar) {
    const float ComboTime = CachedGameState->GetComboTimeRemaining();
    if (ComboTime > 0.f) {
      ComboTimerBar->SetPercent(ComboTime / 5.0f); // 5s window
      ComboTimerBar->SetVisibility(ESlateVisibility::HitTestInvisible);
    } else {
      ComboTimerBar->SetVisibility(ESlateVisibility::Collapsed);
    }
  }
}
