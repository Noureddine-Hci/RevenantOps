// Copyright RevenantOps. All Rights Reserved.

#include "RevenantOpsHUD.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "HealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "RevenantOpsCharacter.h"
#include "WeaponBase.h"

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
    // We need public access - get via the broadcast or we read from BlueprintReadOnly
    // For now, use a simple approach through the weapon's public interface
    AmmoCurrentText->SetText(
        FText::FromString(FString::Printf(TEXT("%d"), 0))); // Will be updated via delegate
  }

  if (WeaponNameText) {
    WeaponNameText->SetText(FText::GetEmpty());
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
  // Damage direction indicator - rotation of an arrow image towards the damage source
  // This is best handled in Blueprint with a directional widget
  // The C++ just provides the angle
  if (!CachedCharacter) {
    return;
  }

  const FVector CharacterLocation = CachedCharacter->GetActorLocation();
  const FVector DamageDirection = (DamageOrigin - CharacterLocation).GetSafeNormal2D();
  const FVector CharacterForward = CachedCharacter->GetActorForwardVector().GetSafeNormal2D();

  // Calculate angle between forward and damage direction
  const float DotProduct = FVector::DotProduct(CharacterForward, DamageDirection);
  const float CrossProduct = FVector::CrossProduct(CharacterForward, DamageDirection).Z;
  const float AngleRad = FMath::Atan2(CrossProduct, DotProduct);
  const float AngleDeg = FMath::RadiansToDegrees(AngleRad);

  // Subclasses can override this or use BP to display a directional indicator at AngleDeg
}
