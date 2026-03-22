---
phase: 01-zombies
plan: 03
subsystem: ai
tags: [zombie, projectile, ranged-ai, ue5, cpp]

# Dependency graph
requires:
  - phase: 01-zombies-01
    provides: "ZombieBase class with melee combat and PerformMeleeAttack virtual"
provides:
  - "AZombieProjectile actor with physics-based travel and damage"
  - "AZombieSpitter ranged zombie subclass with distance-keeping AI"
affects: [01-zombies, wave-spawner, arena]

# Tech tracking
tech-stack:
  added: [UProjectileMovementComponent, USphereComponent]
  patterns: [projectile-spawn-pattern, distance-keeping-ai, override-melee-to-ranged]

key-files:
  created:
    - Source/RevenantOps/AI/ZombieProjectile.h
    - Source/RevenantOps/AI/ZombieProjectile.cpp
    - Source/RevenantOps/AI/ZombieSpitter.h
    - Source/RevenantOps/AI/ZombieSpitter.cpp
  modified: []

key-decisions:
  - "ProjectileClass left as UPROPERTY to set in Blueprint — avoids ConstructorHelpers asset path coupling"
  - "ZombieSpitter reuses MeleeRange/MeleeAttackCooldown for ranged attack timing — avoids duplicating timer logic"

patterns-established:
  - "Projectile spawn pattern: override PerformMeleeAttack, SpawnActor with InitProjectile post-spawn"
  - "Distance-keeping AI: retreat when player within 60% of IdealEngagementRange"

requirements-completed: [ZOMB-04]

# Metrics
duration: 2min
completed: 2026-03-22
---

# Phase 01 Plan 03: ZombieSpitter Summary

**Ranged zombie (ZombieSpitter) that fires AZombieProjectile at 1500 speed with distance-keeping AI retreating at 600cm**

## Performance

- **Duration:** 2 min
- **Started:** 2026-03-22T15:59:16Z
- **Completed:** 2026-03-22T16:01:00Z
- **Tasks:** 2
- **Files modified:** 4

## Accomplishments
- AZombieProjectile actor with UProjectileMovementComponent (1500 speed, 0.1 gravity arc, 5s lifetime)
- AZombieSpitter ranged zombie that overrides melee to spawn projectiles at 1200cm range
- Distance-keeping AI: retreats when player is within 600cm (60% of 1000cm ideal range)
- Friendly fire prevention: projectile ignores actors tagged "Enemy"

## Task Commits

Each task was committed atomically:

1. **Task 1: Create AZombieProjectile actor** - `2791c95` (feat)
2. **Task 2: Create AZombieSpitter with ranged attack and distance-keeping** - `9d8e916` (feat)

## Files Created/Modified
- `Source/RevenantOps/AI/ZombieProjectile.h` - Projectile actor header with sphere collision, projectile movement, InitProjectile API
- `Source/RevenantOps/AI/ZombieProjectile.cpp` - Physics travel, damage on hit, enemy tag check, auto-destroy
- `Source/RevenantOps/AI/ZombieSpitter.h` - Ranged zombie subclass with ProjectileClass TSubclassOf
- `Source/RevenantOps/AI/ZombieSpitter.cpp` - Spawns projectile on attack, retreat logic, 250 speed, 100 HP

## Decisions Made
- ProjectileClass is a UPROPERTY that must be set in Blueprint (BP_ZombieProjectile) — avoids hardcoding asset paths with ConstructorHelpers
- Reused MeleeRange (set to 1200cm) and MeleeAttackCooldown (set to 2.5s) for ranged attack timing — leverages existing ZombieBase timer logic without duplication

## Deviations from Plan

None - plan executed exactly as written.

## Known Stubs

- **ProjectileClass default is nullptr** in ZombieSpitter — must be set in Blueprint. A Blueprint subclass of AZombieProjectile (BP_ZombieProjectile) needs to be created in the editor, and ZombieSpitter Blueprint must set ProjectileClass to it. This is intentional and will be resolved during Blueprint setup phase.

## Issues Encountered

None

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness
- ZombieSpitter and ZombieProjectile ready for Blueprint creation in editor
- Blueprint BP_ZombieProjectile must be created and assigned to ZombieSpitter's ProjectileClass
- ProjectileSpawnSocket ("head") should be verified against the Mannequin skeleton

---
*Phase: 01-zombies*
*Completed: 2026-03-22*
