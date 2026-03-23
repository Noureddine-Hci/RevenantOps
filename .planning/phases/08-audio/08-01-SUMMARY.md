# Phase 08 — Audio & VFX : Résumé d'exécution

**Complété le :** 2026-03-23
**Statut :** Hooks C++ OK — assets audio/VFX à assigner dans les Blueprints

## Ce qui a été fait

### WeaponBase.h — Properties audio/VFX

```cpp
UPROPERTY(EditDefaultsOnly, Category="Audio")
USoundBase* FireSound;

UPROPERTY(EditDefaultsOnly, Category="VFX")
UNiagaraSystem* MuzzleFlashVFX;
```

Utilisés dans `FireShot()` — `UGameplayStatics::PlaySoundAtLocation` + `UNiagaraFunctionLibrary::SpawnSystemAtLocation`

### EnemyBase.h — Properties audio

```cpp
UPROPERTY(EditDefaultsOnly, Category="Audio")
USoundBase* DeathSound;

UPROPERTY(EditDefaultsOnly, Category="Audio")
USoundBase* HitSound;
```

Appelés dans `Die()` et `TakeDamage()` respectivement.

### ZombieBase.h (hérité)

```cpp
UPROPERTY(EditDefaultsOnly, Category="Audio")
USoundBase* GrowlSound;     // Looping ambiance
USoundBase* AttackSound;    // Sur frappe joueur
```

### MercenairesGameState.h

```cpp
UPROPERTY(EditDefaultsOnly, Category="Audio")
USoundBase* ComboSound;     // Sur montée de combo
USoundBase* LowTimeSound;   // Quand timer < 30s
```

## Ce qui reste

### Assets à assigner dans les Blueprints (UMG/Details Panel)

Chaque BP_Pistol, BP_AssaultRifle, etc. doit avoir ses UPROPERTY remplies.

**Options pour les assets :**

1. **StarterContent** (déjà dans le projet UE5) :
   - `/Game/StarterContent/Audio/` — impact sounds, ambient
   - Utiliser comme placeholder

2. **Engine built-in** :
   - `/Engine/EditorSounds/` — quelques sons utiles

3. **Sons distincts par arme** :
   - BP_Pistol.FireSound = son sec court
   - BP_AssaultRifle.FireSound = son automatique
   - BP_SMG.FireSound = son rapide léger
   - BP_Shotgun.FireSound = son grave puissant
   - BP_Sniper.FireSound = son fort avec réverbération

### NiagaraSystem
- Muzzle flash : utiliser NS_Sparks ou NS_MuzzleFlash de StarterContent si disponible
- Impact : NS_BloodSplat à créer ou placeholder

## Note

Les UPROPERTY sont `EditDefaultsOnly` → elles sont modifiables dans les Blueprint Defaults, pas en instance.
