---
status: partial
phase: 11-datatables
source: [11-VERIFICATION.md]
started: 2026-03-29T00:00:00Z
updated: 2026-03-29T00:00:00Z
---

## Current Test

[awaiting human testing — Task 3 editor steps]

## Tests

### 1. Créer DT_WeaponStats dans l'éditeur UE5
expected: Asset /Game/Mercenaires/Data/DT_WeaponStats avec 6 lignes (Pistol/AssaultRifle/SMG/Shotgun/Sniper/Melee), row struct FWeaponTableRow, valeurs per PLAN Task 3
result: [pending]

### 2. Créer DT_EnemyStats dans l'éditeur UE5
expected: Asset /Game/Mercenaires/Data/DT_EnemyStats avec 5 lignes (Slow/Runner/Tank/Spitter/Exploder), row struct FEnemyTableRow, valeurs per PLAN Task 3
result: [pending]

### 3. Configurer les 6 BPs armes (WeaponDataRow)
expected: Chaque BP_Pistol/AssaultRifle/SMG/Shotgun/Sniper/Melee a WeaponDataRow = {DT_WeaponStats, rowName} dans Class Defaults
result: [pending]

### 4. Configurer les 5 BPs zombies (EnemyDataRow)
expected: Chaque BP_ZombieSlow/Runner/Tank/Spitter/Exploder a EnemyDataRow = {DT_EnemyStats, rowName} dans Class Defaults
result: [pending]

### 5. Validation PIE — stats chargées depuis DT
expected: Tank nécessite plus de balles que Slow (400 HP vs 100). Modifier Pistol Damage 20→50 sans recompiler = kills plus rapides en PIE.
result: [pending]

## Summary

total: 5
passed: 0
issues: 0
pending: 5
skipped: 0
blocked: 0

## Gaps
