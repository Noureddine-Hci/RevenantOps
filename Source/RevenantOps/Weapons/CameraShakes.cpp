// Copyright RevenantOps. All Rights Reserved.

#include "CameraShakes.h"
#include "Shakes/PerlinNoiseCameraShakePattern.h"

// UDefaultCameraShakeBase already creates a UPerlinNoiseCameraShakePattern via
// CreateDefaultSubobject in its own constructor. We must NOT call
// ChangeRootShakePattern (which uses NewObject — illegal in constructors).
// Instead we cast the existing pattern and configure it.

UCS_WeaponFire::UCS_WeaponFire(const FObjectInitializer &ObjInit)
    : Super(ObjInit) {
  if (UPerlinNoiseCameraShakePattern *Pattern =
          Cast<UPerlinNoiseCameraShakePattern>(GetRootShakePattern())) {
    Pattern->Duration = 0.1f;
    Pattern->BlendInTime = 0.01f;
    Pattern->BlendOutTime = 0.08f;
    Pattern->RotationAmplitudeMultiplier = 1.f;
    Pattern->Pitch.Amplitude = 0.15f;
    Pattern->Pitch.Frequency = 25.f;
    Pattern->Yaw.Amplitude = 0.06f;
    Pattern->Yaw.Frequency = 18.f;
    Pattern->Roll.Amplitude = 0.f;
    Pattern->LocationAmplitudeMultiplier = 0.f;
  }
}

UCS_TakeDamage::UCS_TakeDamage(const FObjectInitializer &ObjInit)
    : Super(ObjInit) {
  if (UPerlinNoiseCameraShakePattern *Pattern =
          Cast<UPerlinNoiseCameraShakePattern>(GetRootShakePattern())) {
    Pattern->Duration = 0.25f;
    Pattern->BlendInTime = 0.02f;
    Pattern->BlendOutTime = 0.15f;
    Pattern->RotationAmplitudeMultiplier = 1.f;
    Pattern->Pitch.Amplitude = 1.5f;
    Pattern->Pitch.Frequency = 20.f;
    Pattern->Yaw.Amplitude = 1.0f;
    Pattern->Yaw.Frequency = 15.f;
    Pattern->Roll.Amplitude = 0.5f;
    Pattern->Roll.Frequency = 12.f;
    Pattern->LocationAmplitudeMultiplier = 0.f;
  }
}
