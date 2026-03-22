---
phase: 02-arsenal
plan: 02
subsystem: weapons
tags: [shotgun, sniper, pellets, spread, zoom, hitscan, ue5-cpp]

# Dependency graph
requires:
  - phase: 02-arsenal plan 01
    provides: WeaponBase with firing, recoil, spread, ADS, damage falloff systems
provides:
  - AWeaponShotgun with 8-pellet spread, short-range damage profile
  - AWeaponSniper with ADSFOV 30 zoom scope, long-range high-damage profile
affects: [02-arsenal-plan-03, loadout-selection, arena-weapon-pickups]

# Tech tracking
tech-stack:
  added: []
  patterns: [constructor-defaults-only weapon subclasses]

key-files:
  created:
    - Source/RevenantOps/Weapons/WeaponShotgun.h
    - Source/RevenantOps/Weapons/WeaponShotgun.cpp
    - Source/RevenantOps/Weapons/WeaponSniper.h
    - Source/RevenantOps/Weapons/WeaponSniper.cpp
  modified: []

key-decisions:
  - "Constructor-defaults-only pattern: no method overrides needed, all behavior handled by WeaponBase"

patterns-established:
  - "Weapon subclass pattern: header with UCLASS(Blueprintable) + constructor only, cpp sets all UPROPERTY defaults"

requirements-completed: [ARME-03, ARME-04]

# Metrics
duration: 1min
completed: 2026-03-22
---

# Phase 02 Plan 02: Shotgun & Sniper Summary

**Shotgun (8 pellets x 15 dmg, short-range spread) and Sniper (120 dmg, ADSFOV 30 zoom, 3x headshot) weapon subclasses**

## Performance

- **Duration:** 1 min
- **Started:** 2026-03-22T22:43:38Z
- **Completed:** 2026-03-22T22:44:25Z
- **Tasks:** 2
- **Files modified:** 4

## Accomplishments
- WeaponShotgun with 8-pellet multi-hitscan spread, 15 dmg per pellet, short-range falloff (500-1500cm)
- WeaponSniper with 120 damage, ADSFOV 30 zoom scope, 3x headshot multiplier, extreme range (50000cm)
- Both follow constructor-defaults-only pattern, no method overrides needed

## Task Commits

Each task was committed atomically:

1. **Task 1: Create WeaponShotgun** - `a5247b8` (feat)
2. **Task 2: Create WeaponSniper** - `0518e86` (feat)

## Files Created/Modified
- `Source/RevenantOps/Weapons/WeaponShotgun.h` - Shotgun class declaration inheriting AWeaponBase
- `Source/RevenantOps/Weapons/WeaponShotgun.cpp` - Shotgun constructor: 8 pellets, 15 dmg, SemiAuto 80 RPM, mag 6, reload 3s
- `Source/RevenantOps/Weapons/WeaponSniper.h` - Sniper class declaration inheriting AWeaponBase
- `Source/RevenantOps/Weapons/WeaponSniper.cpp` - Sniper constructor: 120 dmg, SemiAuto 40 RPM, ADSFOV 30, mag 5, 3x headshot

## Decisions Made
- Constructor-defaults-only pattern: all weapon behavior (firing, recoil, spread, ADS) is handled by WeaponBase; subclasses only set stat values

## Deviations from Plan
None - plan executed exactly as written.

## Issues Encountered
None

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Shotgun and Sniper ready for Blueprint setup (mesh, sounds, VFX)
- Plan 02-03 (remaining weapons or weapon integration) can proceed
- Loadout system will need references to all weapon subclasses

## Self-Check: PASSED

All 4 files created, both commits verified (a5247b8, 0518e86).

---
*Phase: 02-arsenal*
*Completed: 2026-03-22*
