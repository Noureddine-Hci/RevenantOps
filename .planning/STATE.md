---
gsd_state_version: 1.0
milestone: v3.0
milestone_name: Polish Visuel & Feel
status: ready_to_plan
last_updated: "2026-04-06T00:00:00.000Z"
progress:
  total_phases: 3
  completed_phases: 0
  total_plans: 0
  completed_plans: 0
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-04-06)

**Core value:** Le joueur peut lancer une partie, choisir son loadout, tuer des zombies en enchainant des combos sous pression du timer, et voir son score final.
**Current focus:** Phase 14 — Animations & Posture (next: `/gsd:plan-phase 14`)

## Current Position

Phase: 14 — Animations & Posture
Plan: Not started
Status: Roadmap defined — ready to plan Phase 14
Last activity: 2026-04-06 — v3.0 roadmap created (phases 14-16)

```
v3.0 Progress: [                    ] 0/3 phases
Phase 14: [ ] Animations & Posture
Phase 15: [ ] Arene Retravaillee
Phase 16: [ ] HUD & Menus Polish
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

## v2.0 — COMPLETE (2026-04-05)

| Phase | Statut | Date |
|-------|--------|------|
| 11. DataTables | Complete | 2026-03-29 |
| 12. Assets Armes | Complete | 2026-04-02 |
| 13. Audio & VFX Niagara | Complete | 2026-04-05 |

## v3.0 — EN COURS

| Phase | Statut | Date |
|-------|--------|------|
| 14. Animations & Posture | Not started | — |
| 15. Arene Retravaillee | Not started | — |
| 16. HUD & Menus Polish | Not started | — |

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
| RevenantOpsHUD.h/.cpp | 3+6+14 | OK (timer/score/combo + BuildDefaultUI + UpdateReloadBar/DamageDirection/ShowKillNotification) |
| LoadoutWidget.h/.cpp | 5 | OK |
| TitleScreenWidget.h/.cpp | 6 | OK |
| GameOverWidget.h/.cpp | 6 | OK |
| LeaderboardWidget.h/.cpp + LeaderboardSaveGame.h | 6 | OK |
| RevenantOpsCharacter.cpp | 7+14 | OK (SpringArm OTS RE4 + ADS zoom + bIsArmed) |
| RevenantOpsPlayerController.h/.cpp | 9+14 | OK (flow title→loadout→match→gameover→leaderboard + ReceivedPlayer fix) |
| CameraShakes.h/.cpp | 14 | OK (CS_WeaponFire + CS_TakeDamage) |

### Blueprints & Assets v2.0

| Asset | Chemin | Statut |
|-------|--------|--------|
| BP_ZombieSlow/Runner/Tank/Spitter/Exploder | /Game/Mercenaires/Zombies/ | Configure |
| BP_ZombieProjectile | /Game/Mercenaires/Zombies/ | Configure |
| BP_Pistol/AssaultRifle/SMG/Shotgun/Sniper/Melee | /Game/Mercenaires/Weapons/ | Kenney meshes assignes |
| BP_MercenairesGameState | /Game/Mercenaires/ | Configure |
| DT_WeaponStats / DT_EnemyStats | /Game/Mercenaires/ | Peuples |
| AM_Rifle_Fire/Reload/Equip | /Game/Mercenaires/Anims/Montages/ | Crees |
| AM_Pistol_Fire/Reload/Equip | /Game/Mercenaires/Anims/Montages/ | Crees |
| ABP_Mercenaire | /Game/Mercenaires/Anims/ | Configure (BS_Pistol_Walk_Run) |
| BS_Pistol_Walk_Run | /Game/Mercenaires/Anims/ | 4 samples OK |
| NS_MuzzleFlash / NS_Impact_Surface / NS_Impact_Blood | /Game/Mercenaires/VFX/ | Crees |
| WBP_Mercenaires_HUD | /Game/Mercenaires/UI/ | Cree (parent URevenantOpsHUD) |

### Map BLACKSITE (Phase 10 — Complete, retravaillee en Phase 15)

- 5 zones, 160 actors, NavMesh, 8 SpawnPoints (SP1-SP8)
- WaveSpawner configure (3 vagues : 7/8/9 ennemis), PIE valide
- 5 pickups places (TB1 30s, TB2 15s, TB3 15s, A1 Ammo, A2 Ammo)
- 13 PointLights (rouge/bleu/orange par zone)

## v3.0 — Ce Qui RESTE a Faire

### Phase 14: Animations & Posture (NEXT)

- BlendSpaces distincts par type d'arme dans ABP_Mercenaire (pistolet vs rifle vs shotgun)
- Etat "EmptyMag" dans ABP : detecter CurrentAmmo==0, jouer slide-lock/bolt-open anim
- AnimMontage reload visible en third-person (already partial — verifier visibility sur personnage)
- IK Two-Bone sur main gauche pour pointer vers grip de l'arme (IK Target socket sur weapon mesh)
- Transition Unarmed→Armed dans ABP avec blend time visible (pas instantane)

### Phase 15: Arene Retravaillee (apres Phase 14)

- Refaire geometry/layout BLACKSITE pour creer 3 zones distinctes lisibles
- Rework eclairage : zones d'ombre, couleurs d'ambiance, contraste fort
- Repositionner/ajouter SpawnPoints pour couvrir toutes les directions

### Phase 16: HUD & Menus Polish (apres Phase 14, parallelisable avec Phase 15)

- WBP_Mercenaires_HUD : barre de vie avec changement couleur (vert→jaune→rouge via material param ou color binding)
- Affichage munitions : style "12 | 48" avec distinction visuelle (taille, couleur, separateur)
- Combo counter : animation Blueprint sur le widget quand la valeur monte (scale pulse ou flash couleur)
- WBP_TitleScreen : remplacer fond noir par image atmospherique (screenshot BLACKSITE ou asset Kenney)
- Transitions entre ecrans : fade ou slide + hover/pressed states sur les boutons
- NOTE : edition widget tree obligatoirement manuelle dans UE5 (Python MCP ne peut pas modifier WidgetTree)

## Accumulated Context

### Decisions

- [Roadmap v1.0] Camera OTS deplacee en Phase 7 — la camera actuelle fonctionne, on peaufine plus tard
- [Roadmap v1.0] Zombies en Phase 1 : le core gameplay (ennemis) est la priorite
- [Roadmap v1.0] Phase 9 Integration sans nouveaux requirements — valide les 37 en conditions demo reelles
- [Roadmap v2.0] DataTables en Phase 11 avant les assets — fondation data-driven d'abord, les BPs armes liront les stats de la DT quand les meshes sont assignes
- [Roadmap v2.0] Assets armes en Phase 12 apres DataTables — evite de reconfigurer les BPs deux fois
- [Roadmap v2.0] Audio/VFX en Phase 13 en dernier — polish final, aucune autre phase n'en depend
- [Roadmap v3.0] Animations + Posture en Phase 14 en premier — le ressenti de combat depend directement des animations, les phases 15 et 16 beneficient d'animations fonctionnelles pour les tests
- [Roadmap v3.0] Phase 16 (HUD+Menus) depend de Phase 14 mais pas de Phase 15 — peut commencer en parallele avec Phase 15 si Phase 14 est complete
- [Roadmap v3.0] HUD et MENU groupes en Phase 16 — meme type de travail (widget Blueprint), meme contexte (editeur UE5 manuel), scope comparable
- [Phase 01-zombies] SetMaxHealth public setter sur HealthComponent pour customisation en sous-classes C++
- [Phase 01-zombies] ZombieExploder self-destructs via ApplyDamage(9999) pour preserver delegate OnEnemyDied
- [Phase 02-arsenal] Constructor-defaults-only pattern : stats dans constructor, comportement dans WeaponBase
- [Phase 02-arsenal] Melee override FireShot avec sphere trace — pas de Super call, SweepSingleByChannel
- [Phase 03-timer] Points : Slow=50, Runner=75, Tank=200, Spitter=150, Exploder=125
- [Phase 07-camera] SpringArm OTS : TargetArmLength=120, SocketOffset=(0,50,60), lag active
- [Phase 08-audio] Audio/VFX : UPROPERTY hooks dans WeaponBase et EnemyBase — valeurs nullptr en v1, assignees en v2.0 Phase 13
- [Phase 11-datatables] ApplyWeaponDataRow called BEFORE CurrentAmmo init so DT MaxAmmo takes effect at startup
- [Phase 11-datatables] ApplyEnemyDataRow called AFTER Super::BeginPlay so HealthComp events are bound before SetMaxHealth/ResetHealth

### Blockers Techniques Connus

- **MCP TCP bloque pendant PIE Play** — config uniquement en Editor/Simulate mode
- **Widget Blueprint widget tree** : impossible de creer/modifier des widgets UMG via Python MCP — edition manuelle obligatoire dans UE5 editor
- **ABP Python limitations** : impossible de creer/connecter des noeuds AnimGraph via Python (Nodes/Schema proteges) — reconfigurer manuellement ou via ScopedEditorTransaction sur proprietes existantes
- **LiveCoding** : ne peut PAS creer de nouvelles classes — full build via Build.bat obligatoire pour nouvelles classes C++

### Lecons Techniques Importantes

- LiveCoding ne peut PAS creer de nouvelles classes — full build via Build.bat obligatoire
- Apres full build, redemarrer UE5 pour que les BPs corrompus soient recharges proprement
- `hasattr(unreal, 'ClassName')` = methode fiable pour verifier qu'une classe C++ est chargee
- BPs crees pendant LiveCoding se corrompent (references LIVECODING_Xxx_0) → supprimer et recreer apres full build
- OFPA (One File Per Actor) = les External Actors sont stockes separement dans Content/__ExternalActors__/
- Camera Shake UE5.7 : GetRootShakePattern() dans constructeur, jamais ChangeRootShakePattern<T>() (NewObject illegal dans constructeur)
- ScopedEditorTransaction + modify() obligatoires pour que les valeurs survivent aux compilations BP

## Session Continuity

**Last session**: 2026-04-05/06 — v2.0 complete (phases 12-13 done, audio+VFX, armed locomotion, HitFlash)
**Current session**: 2026-04-06 — v3.0 roadmap cree (phases 14-16 definis)
**Next session**: Planifier Phase 14 via `/gsd:plan-phase 14`
