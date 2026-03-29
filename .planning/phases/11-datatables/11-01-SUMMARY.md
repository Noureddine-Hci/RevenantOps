---
phase: 11-datatables
plan: 01
subsystem: data-tables
tags: [datatables, weapons, enemies, cpp, ue5]
dependency_graph:
  requires: []
  provides: [FWeaponTableRow, FEnemyTableRow, WeaponBase.WeaponDataRow, ZombieBase.EnemyDataRow]
  affects: [WeaponBase, ZombieBase, all zombie subclasses, all weapon BPs]
tech_stack:
  added: [FTableRowBase, FDataTableRowHandle, Engine/DataTable.h]
  patterns: [data-driven stats via FDataTableRowHandle in BeginPlay, UE_LOG fallback on invalid row]
key_files:
  created:
    - Source/RevenantOps/Weapons/WeaponTableRow.h
    - Source/RevenantOps/AI/EnemyTableRow.h
  modified:
    - Source/RevenantOps/Weapons/WeaponBase.h
    - Source/RevenantOps/Weapons/WeaponBase.cpp
    - Source/RevenantOps/AI/ZombieBase.h
    - Source/RevenantOps/AI/ZombieBase.cpp
key_decisions:
  - "ApplyWeaponDataRow called BEFORE CurrentAmmo = MagazineSize in BeginPlay so DT MaxAmmo takes effect at init"
  - "ApplyEnemyDataRow called AFTER Super::BeginPlay() so HealthComp events are already bound before SetMaxHealth"
  - "IsNull() guard preserves constructor defaults when no DT is assigned — no crash, no forced DT requirement"
metrics:
  duration_seconds: 481
  completed_date: "2026-03-29"
  tasks_completed: 2
  tasks_manual: 1
  files_created: 2
  files_modified: 4
  build_result: "Succeeded — 9 actions, 0 errors, 51.82s"
---

# Phase 11 Plan 01: DataTable Row Structs + BeginPlay Integration Summary

**One-liner:** FWeaponTableRow and FEnemyTableRow USTRUCTs wired into WeaponBase and ZombieBase via FDataTableRowHandle with UE_LOG fallback on invalid rows.

## Tasks Completed

| # | Name | Commit | Files |
|---|------|--------|-------|
| 1 | Create FWeaponTableRow and FEnemyTableRow structs | d4b6c9c | WeaponTableRow.h, EnemyTableRow.h |
| 2 | Integrate FDataTableRowHandle + BeginPlay loading | 788f6d2 | WeaponBase.h/cpp, ZombieBase.h/cpp |

## Task 3 — MANUAL CHECKPOINT (Human Required)

**Task 3: Create DataTable assets in UE5 editor and configure Blueprint defaults**

This task cannot be automated. It requires the UE5 editor to be relaunched after the C++ build so the new USTRUCT types are available in the struct picker.

**Prerequisites:**
- C++ build must succeed (it did — 0 errors at commit 788f6d2)
- Close and relaunch UE5 editor to load the new UHTGenerated types

**Steps:**

1. In UE5 Content Browser, navigate to `/Game/Mercenaires/` and create a new folder named `Data`.

2. Right-click in `/Game/Mercenaires/Data/` → Miscellaneous → Data Table, row struct = `WeaponTableRow`. Name it `DT_WeaponStats`. Add 6 rows:

   | Row Name     | Damage | FireRate | MaxAmmo | Range  |
   |--------------|--------|----------|---------|--------|
   | Pistol       | 20     | 300      | 12      | 8000   |
   | AssaultRifle | 15     | 600      | 30      | 12000  |
   | SMG          | 12     | 900      | 40      | 8000   |
   | Shotgun      | 50     | 60       | 8       | 3000   |
   | Sniper       | 120    | 45       | 5       | 50000  |
   | Melee        | 35     | 0        | 0       | 150    |

   Save with Ctrl+S.

3. Right-click in `/Game/Mercenaires/Data/` → Data Table, row struct = `EnemyTableRow`. Name it `DT_EnemyStats`. Add 5 rows:

   | Row Name | MaxHP | MeleeDamage | MovementSpeed |
   |----------|-------|-------------|---------------|
   | Slow     | 100   | 10          | 200           |
   | Runner   | 60    | 12          | 550           |
   | Tank     | 400   | 25          | 180           |
   | Spitter  | 150   | 15          | 300           |
   | Exploder | 80    | 80          | 350           |

   Save with Ctrl+S.

4. Open each weapon BP in `/Game/Mercenaires/Weapons/` → Class Defaults → set `Weapon Data Row`:

   | Blueprint       | DataTable      | Row Name     |
   |-----------------|----------------|--------------|
   | BP_Pistol       | DT_WeaponStats | Pistol       |
   | BP_AssaultRifle | DT_WeaponStats | AssaultRifle |
   | BP_SMG          | DT_WeaponStats | SMG          |
   | BP_Shotgun      | DT_WeaponStats | Shotgun      |
   | BP_Sniper       | DT_WeaponStats | Sniper       |
   | BP_Melee        | DT_WeaponStats | Melee        |

   Compile and save each BP.

5. Open each zombie BP in `/Game/Mercenaires/Zombies/` → Class Defaults → set `Enemy Data Row`:

   | Blueprint         | DataTable     | Row Name |
   |-------------------|---------------|----------|
   | BP_ZombieSlow     | DT_EnemyStats | Slow     |
   | BP_ZombieRunner   | DT_EnemyStats | Runner   |
   | BP_ZombieTank     | DT_EnemyStats | Tank     |
   | BP_ZombieSpitter  | DT_EnemyStats | Spitter  |
   | BP_ZombieExploder | DT_EnemyStats | Exploder |

   Compile and save each BP.

6. PIE validation: launch a Mercenaires match, verify Tank requires more shots than Slow. Change Pistol Damage to 50 in DT_WeaponStats, relaunch PIE — Pistol should kill faster. Revert to 20 after test.

**Acceptance criteria:**
- DT_WeaponStats at /Game/Mercenaires/Data/ with 6 rows
- DT_EnemyStats at /Game/Mercenaires/Data/ with 5 rows
- BP_Pistol WeaponDataRow = {DT_WeaponStats, "Pistol"}
- BP_ZombieTank EnemyDataRow = {DT_EnemyStats, "Tank"}
- PIE launches without DataTable-related errors in Output Log
- Changing DT values produces observable behavior change without recompile

## Deviations from Plan

None — plan executed exactly as written. Task 3 is marked manual per plan specification.

## Known Stubs

None. The C++ integration is complete and correct. Task 3 (editor-side DataTable creation) is intentionally deferred to human action per plan design — it is not a stub in the code but a required manual editor step before the full data-driven flow is operational.

## Self-Check: PASSED
