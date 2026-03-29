---
gsd_state_version: 1.0
milestone: v2.0
milestone_name: Finition Mode Mercenaires
status: in_progress
stopped_at: Phase 11 — DataTables (not started, ready to plan)
last_updated: "2026-03-29T00:00:00Z"
progress:
  total_phases: 3
  completed_phases: 0
  total_plans: 0
  completed_plans: 0
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-03-29)

**Core value:** Le joueur peut lancer une partie, choisir son loadout, tuer des zombies en enchainant des combos sous pression du timer, et voir son score final.
**Current focus:** Milestone v2.0 — Finition Mode Mercenaires

## Current Position

Phase: 11 — DataTables (not started)
Plan: —
Status: Roadmap v2.0 cree, pret a planifier Phase 11
Last activity: 2026-03-29 — Roadmap v2.0 etabli (3 phases : 11 DataTables, 12 Assets Armes, 13 Audio/VFX Niagara)

```
v2.0 Progress: [          ] 0/3 phases
Phase 11: [ ] DataTables
Phase 12: [ ] Assets Armes
Phase 13: [ ] Audio & VFX Niagara
```

## v1.0 — COMPLETE (2026-03-29)

Toutes les phases 1-10 terminees. PIE valide. Demo partageable operationnelle.

| Phase | Statut | Date |
|-------|--------|------|
| 1. Zombies | Complete | 2026-03-22 |
| 2. Arsenal | Complete | 2026-03-22 |
| 3. Timer & Score | Complete | 2026-03-23 |
| 4. Arene | Complete | 2026-03-23 |
| 5. Loadout | Complete | 2026-03-23 |
| 6. UI & Menus | Complete | 2026-03-23 |
| 7. Camera OTS | Complete | 2026-03-23 |
| 8. Audio & VFX | Complete | 2026-03-23 |
| 9. Integration | Complete | 2026-03-23 |
| 10. Editor Setup & Playtest | Complete | 2026-03-29 |

## Ce Qui Est FAIT (C++ + Blueprints)

### C++ — 100% compile et charge

| Fichier | Phase | Statut |
|---------|-------|--------|
| EnemyBase.h/.cpp | 1 | OK |
| ZombieBase.h/.cpp | 1 | OK |
| ZombieSlow/Runner/Tank/Spitter/Exploder | 1 | OK |
| WeaponBase.h/.cpp | 2 | OK (+ Melee enum, getters, AddReserveAmmo) |
| WeaponPistol/AssaultRifle/SMG/Shotgun/Sniper/Melee | 2 | OK |
| MercenairesGameState.h/.cpp | 3 | OK |
| TimeBonusPickup.h/.cpp | 3 | OK |
| AmmoBonusPickup.h/.cpp | 3 | OK |
| RevenantOpsHUD.h/.cpp | 3+6 | OK (timer/score/combo display) |
| LoadoutWidget.h/.cpp | 5 | OK |
| TitleScreenWidget.h/.cpp | 6 | OK |
| GameOverWidget.h/.cpp | 6 | OK |
| LeaderboardWidget.h/.cpp + LeaderboardSaveGame.h | 6 | OK |
| RevenantOpsCharacter.cpp | 7 | OK (SpringArm OTS RE4 style + ADS zoom) |
| RevenantOpsPlayerController.h/.cpp | 9 | OK (flow title→loadout→match→gameover→leaderboard) |

### UPROPERTY Hooks existants (base pour v2.0)

| Hook | Classe | Phase v2.0 |
|------|--------|-----------|
| FireSound (USoundBase*) | WeaponBase | Phase 13 |
| MuzzleFlashVFX (UNiagaraSystem*) | WeaponBase | Phase 13 |
| DeathSound (USoundBase*) | EnemyBase | Phase 13 |
| HitSound (USoundBase*) | EnemyBase | Phase 13 |
| WeaponMeshSM (UStaticMeshComponent*) | WeaponBase | Phase 12 |
| Actuellement : SM_ChamferCube par defaut | 6 BPs armes | Remplace Phase 12 |

### Blueprints crees dans UE5

| Blueprint | Chemin | Statut |
|-----------|--------|--------|
| BP_ZombieSlow/Runner/Tank/Spitter/Exploder | /Game/Mercenaires/Zombies/ | Configure |
| BP_ZombieProjectile | /Game/Mercenaires/Zombies/ | Configure |
| BP_Pistol/AssaultRifle/SMG/Shotgun/Sniper/Melee | /Game/Mercenaires/Weapons/ | Configure (SM_ChamferCube) |
| BP_MercenairesGameState | /Game/Mercenaires/ | Configure |
| BP_TimeBonusPickup_30s / _15s | /Game/Mercenaires/ | Configure |
| BP_AmmoBonusPickup | /Game/Mercenaires/ | Configure |
| WBP_TitleScreen/Loadout/GameOver/Leaderboard | /Game/Mercenaires/UI/ | Fonctionnels |

### Map BLACKSITE (Phase 10 — Complete)

- 5 zones, 160 actors, NavMesh, 8 SpawnPoints (SP1-SP8)
- WaveSpawner configure (3 vagues : 7/8/9 ennemis), PIE valide (14 ennemis observes)
- 5 pickups places (TB1 30s, TB2 15s, TB3 15s, A1 Ammo, A2 Ammo)
- 13 PointLights (rouge/bleu/orange par zone)

## v2.0 — Ce Qui RESTE a Faire

### Phase 11: DataTables (NEXT)

Prerequis technique :
- Creer `FWeaponTableRow : public FTableRowBase` en C++ avec champs : Damage, FireRate, MaxAmmo, Range
- Creer `FEnemyTableRow : public FTableRowBase` avec champs : MaxHP, Damage, MovementSpeed
- Modifier WeaponBase::BeginPlay() pour lire stats depuis DT_WeaponStats via FDataTableRowHandle
- Modifier EnemyBase::BeginPlay() pour lire stats depuis DT_EnemyStats
- Creer DT_WeaponStats et DT_EnemyStats dans l'editeur UE5, remplir les lignes

### Phase 12: Assets Armes (apres Phase 11)

- Importer 6 meshes armes depuis Fab.com (SM_ pour chaque arme)
- Assigner dans Details panel de chaque BP arme : WeaponMeshSM = nouveau mesh
- Verifier positionnement/rotation sur socket hand_r en PIE

### Phase 13: Audio & VFX Niagara (apres Phase 12)

- Importer/creer assets audio : 6 sons de tir, 3 sons zombie (idle/attaque/mort)
- Creer/importer 3 systemes Niagara : NS_MuzzleFlash, NS_BloodImpact, NS_ZombieExplosion
- Assigner dans Details panel de chaque BP arme et BP zombie

## Accumulated Context

### Decisions

- [Roadmap v1.0] Camera OTS deplacee en Phase 7 — la camera actuelle fonctionne, on peaufine plus tard
- [Roadmap v1.0] Zombies en Phase 1 : le core gameplay (ennemis) est la priorite
- [Roadmap v1.0] Phase 9 Integration sans nouveaux requirements — valide les 37 en conditions demo reelles
- [Roadmap v2.0] DataTables en Phase 11 avant les assets — fondation data-driven d'abord, les BPs armes liront les stats de la DT quand les meshes sont assignes
- [Roadmap v2.0] Assets armes en Phase 12 apres DataTables — evite de reconfigurer les BPs deux fois
- [Roadmap v2.0] Audio/VFX en Phase 13 en dernier — polish final, aucune autre phase n'en depend
- [Phase 01-zombies] SetMaxHealth public setter sur HealthComponent pour customisation en sous-classes C++
- [Phase 01-zombies] ZombieExploder self-destructs via ApplyDamage(9999) pour preserver delegate OnEnemyDied
- [Phase 02-arsenal] Constructor-defaults-only pattern : stats dans constructor, comportement dans WeaponBase
- [Phase 02-arsenal] Melee override FireShot avec sphere trace — pas de Super call, SweepSingleByChannel
- [Phase 03-timer] Points : Slow=50, Runner=75, Tank=200, Spitter=150, Exploder=125
- [Phase 07-camera] SpringArm OTS : TargetArmLength=120, SocketOffset=(0,50,60), lag active
- [Phase 08-audio] Audio/VFX : UPROPERTY hooks dans WeaponBase et EnemyBase — valeurs nullptr en v1, assignees en v2.0 Phase 13

### Blockers Techniques Connus

- **MCP TCP bloque pendant PIE Play** — config uniquement en Editor/Simulate mode
- **DataTables UE5** : necessitent FTableRowBase C++ struct + recompilation avant creation des DT assets dans l'editeur
- **Niagara** : Plugin Niagara doit etre active dans le projet (verifier Plugins avant Phase 13)

### Lecons Techniques Importantes

- LiveCoding ne peut PAS creer de nouvelles classes — full build via Build.bat obligatoire
- Apres full build, redemarrer UE5 pour que les BPs corrompus soient recharges proprement
- `hasattr(unreal, 'ClassName')` = methode fiable pour verifier qu'une classe C++ est chargee
- BPs crees pendant LiveCoding se corrompent (references LIVECODING_Xxx_0) → supprimer et recreer apres full build
- OFPA (One File Per Actor) = les External Actors sont stockes separement dans Content/__ExternalActors__/

## Session Continuity

**Last session**: 2026-03-29 — v1.0 complete (Phase 10 done, PIE valide, weapon mesh OK)
**Current session**: 2026-03-29 — Roadmap v2.0 etabli (phases 11-13)
**Next session**: Planifier Phase 11 (DataTables) via `/gsd:plan-phase 11`
