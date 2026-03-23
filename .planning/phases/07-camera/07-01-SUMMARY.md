# Phase 07 — Camera OTS : Résumé d'exécution

**Complété le :** 2026-03-23
**Fichiers modifiés :** 1 (RevenantOpsCharacter.cpp)

## Ce qui a été fait

### Setup SpringArm (constructeur)

```cpp
CameraBoom->TargetArmLength = 120.0f;             // Proche du personnage
CameraBoom->SocketOffset = FVector(0.f, 50.f, 60.f); // Épaule droite + hauteur
CameraBoom->bEnableCameraLag = true;
CameraBoom->CameraLagSpeed = 15.f;
CameraBoom->bEnableCameraRotationLag = true;
CameraBoom->CameraRotationLagSpeed = 20.f;
```

### UpdateCameraFOV (ADS zoom)

- Normal : SocketOffset=(0,50,60), ArmLength=120, FOV normal
- ADS : SocketOffset=(0,40,55), ArmLength=80, FOV = `Weapon->GetADSFOV()` depuis CDO
- Interpolation douce via `FMath::FInterpTo` chaque tick
- Fallback FOV 70° si aucune arme équipée

## Résultat visuel

- Vue épaule droite serrée style RE4 par défaut
- Zoom supplémentaire en ADS avec resserrement de l'offset
- Camera lag donne un feeling fluide sans saccade
- Le retour à la vue normale après relâchement ADS est smooth

## Décision

`GetADSFOV()` lit depuis le CDO de l'arme active — pas besoin d'instance spawned pour obtenir la valeur.
