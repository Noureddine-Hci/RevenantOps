# Phase 03 — Timer & Score : Résumé d'exécution

**Complété le :** 2026-03-23
**Durée :** ~30 min
**Fichiers modifiés/créés :** 6

## Ce qui a été fait

### Nouveaux fichiers C++

**MercenairesGameState.h/.cpp**
- Timer 5 minutes en décompte, `StartMatch()` / `EndMatch()`
- Score par kill selon type de zombie via `RegisterKill(AEnemyBase*)`
  - ZombieSlow=50, ZombieRunner=75, ZombieTank=200, ZombieSpitter=150, ZombieExploder=125
- Combo multiplicateur x1 à x20 avec decay 5 secondes sans kill
- `AddBonusTime(float Seconds)` pour les pickups
- Delegates : `FOnScoreChanged`, `FOnComboChanged`, `FOnTimerChanged`, `FOnMatchStateChanged`

**TimeBonusPickup.h/.cpp**
- Overlap pickup → appelle `MercenairesGameState->AddBonusTime(BonusSeconds)`
- BonusSeconds configurable (default 30)
- Animation bob/rotation, support respawn configurable

**AmmoBonusPickup.h/.cpp**
- Overlap pickup → appelle `CurrentWeapon->AddReserveAmmo(AmmoAmount)`
- AmmoAmount default 30, RespawnTime 45s

### Fichiers modifiés

**WeaponBase.h**
- Ajout enum `Melee` à `EWeaponCategory`
- Ajout getters : `GetBaseDamage()`, `GetFireRate()`, `GetMagazineSize()`, `GetReloadTime()`, `GetADSFOV()`
- Ajout `AddReserveAmmo(int32 Amount)` inline (clampé sur MaxReserveAmmo, broadcast OnAmmoChanged)

**WeaponMelee.cpp**
- Correction : `WeaponCategory = EWeaponCategory::Melee` (était Pistol)

**RevenantOpsHUD.h/.cpp**
- Ajout widgets : TimerText, ScoreText, ComboText, ComboTimerBar (UProgressBar)
- `UpdateMercenairesDisplay()` : format MM:SS, rouge sous 30s, combo affiché si >x1

## Décisions techniques

- `IsA<>` sur les sous-classes zombie pour déterminer les points (pas d'enum sur EnemyBase)
- Combo decay géré par FTimerHandle dans GameState, pas dans le HUD
- `AddReserveAmmo` inline dans WeaponBase.h pour éviter dispatch

## Blueprint créé

- BP_MercenairesGameState (assigné au GameMode dans ThirdPersonGameMode)
- BP_TimeBonusPickup_30s (BonusSeconds=30)
- BP_TimeBonusPickup_15s (BonusSeconds=15)
- BP_AmmoBonusPickup

## État

C++ compilé et chargé. Blueprints créés. Test en jeu requis.
