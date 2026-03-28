---
gsd_state_version: 1.0
milestone: v1.0
milestone_name: Demo Partageable Mercenaires
status: in_progress
stopped_at: Phase 10 — C++ phases 1-9 complètes. 4 blocages critiques identifiés avant playtest (socket, WaveSpawner, Waves array, weapon meshes). Prêt pour fixes C++ + config MCP + playtest.
last_updated: "2026-03-24T00:00:00Z"
progress:
  total_phases: 10
  completed_phases: 9
  total_plans: 13
  completed_plans: 13
---

# Project State

## Project Reference

See: .planning/PROJECT.md (updated 2026-03-22)

**Core value:** Le joueur peut lancer une partie, choisir son loadout, tuer des zombies en enchainant des combos sous pression du timer, et voir son score final.
**Current focus:** Phase 4 — complétion éditeur + validation complète en jeu

## Current Position

Phase: 09 (integration) — C++ COMPLETE, Blueprint setup en cours
Plan: Tous les plans C++ sont done. Reste : setup éditeur (Arena complétion, widgets layout, PlayerController flow).

## Ce Qui Est FAIT (C++ + Blueprints)

### C++ — 100% compilé et chargé

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

### Blueprints créés dans UE5

| Blueprint | Chemin | Statut |
|-----------|--------|--------|
| BP_ZombieSlow | /Game/Mercenaires/Zombies/ | Créé + configuré |
| BP_ZombieRunner | /Game/Mercenaires/Zombies/ | Créé + configuré |
| BP_ZombieTank | /Game/Mercenaires/Zombies/ | Créé + configuré |
| BP_ZombieSpitter | /Game/Mercenaires/Zombies/ | Créé + configuré (ProjectileClass=BP_ZombieProjectile) |
| BP_ZombieExploder | /Game/Mercenaires/Zombies/ | Créé + configuré |
| BP_ZombieProjectile | /Game/Mercenaires/Zombies/ | Créé |
| BP_Pistol | /Game/Mercenaires/Weapons/ | Créé |
| BP_AssaultRifle | /Game/Mercenaires/Weapons/ | Créé |
| BP_SMG | /Game/Mercenaires/Weapons/ | Créé |
| BP_Shotgun | /Game/Mercenaires/Weapons/ | Créé |
| BP_Sniper | /Game/Mercenaires/Weapons/ | Créé |
| BP_Melee | /Game/Mercenaires/Weapons/ | Créé |
| BP_MercenairesGameState | /Game/Mercenaires/ | Créé + assigné au GameMode |
| BP_TimeBonusPickup_30s | /Game/Mercenaires/ | Créé (BonusSeconds=30) |
| BP_TimeBonusPickup_15s | /Game/Mercenaires/ | Créé (BonusSeconds=15) |
| BP_AmmoBonusPickup | /Game/Mercenaires/ | Créé |
| WBP_TitleScreen | /Game/Mercenaires/UI/ | Créé (layout à faire dans éditeur) |
| WBP_Loadout | /Game/Mercenaires/UI/ | Créé (layout à faire dans éditeur) |
| WBP_GameOver | /Game/Mercenaires/UI/ | Créé (layout à faire dans éditeur) |
| WBP_Leaderboard | /Game/Mercenaires/UI/ | Créé (layout à faire dans éditeur) |

### Arène (Phase 4) — Géométrie en place, setup éditeur en cours

| Élément | Statut |
|---------|--------|
| Floor 5000x5000cm | Spawné |
| 4 murs périmètre | Spawné |
| Couloir gauche (2 murs + toit) | Spawné |
| Plateforme élevée droite + rampe | Spawné |
| 5 caisses de couverture centre | Spawné |
| 4 piliers | Spawné |
| Petite salle coin haut-droit | Spawné |
| **Spawn points zombies** | **MANQUANT** |
| **Pickups placés dans l'arène** | **MANQUANT** |
| **NavMesh bounds** | **MANQUANT** |
| **Éclairage/atmosphère** | **MANQUANT** |

## Ce Qui RESTE à Faire (4 blocages critiques)

### **PRIORITÉ 1 — Fixes C++ (5 min, BLOCKING)**

#### Blocage 1: Arme invisible
- [ ] **Fichier**: `Source/RevenantOps/RevenantOpsCharacter.cpp` (constructeur)
- [ ] **Changement**: L40 `FName("WeaponSocket")` → `FName("hand_r")`
- [ ] **Raison**: `WeaponSocket` n'existe pas, `hand_r` = socket natif Mannequin UE5

#### Blocage 2: Ennemis ne spawnent jamais
- [ ] **Fichier**: `Source/RevenantOps/RevenantOpsPlayerController.cpp`
- [ ] **Lieu**: dans `StartMercenairesMatch()`, après `GS->StartMatch();`
- [ ] **Ajout**: Boucle WaveSpawner auto-start (voir CLAUDE.md Fix 2 pour code exact)
- [ ] **Raison**: `StartEncounter()` n'est jamais appelé autrement

#### Blocage 3: Waves vides
- [ ] **Acteur**: `MercenairesWaveSpawner` dans `Lvl_ThirdPerson`
- [ ] **Propriété**: Array `Waves`
- [ ] **Après recompil C++**: Via MCP `util_execute_python`, remplir 3 vagues (voir CLAUDE.md Fix 3)
- [ ] **Structure**: chaque wave = FEnemyWaveEntry(EnemyClass, Count)

#### Blocage 4: Meshes armes vides (OPTIONNEL)
- [ ] **BPs**: `BP_Pistol`, `BP_AssaultRifle`, etc. dans `/Game/Mercenaires/Weapons/`
- [ ] **Optionnel**: Assigner StaticMesh placeholder via MCP ou Details panel
- [ ] **Note**: Les tirs fonctionnent sans mesh — c'est cosmétique

---

### **PRIORITÉ 2 — Après fixes C++ (avant playtest)**
- [ ] Recompiler (VS 2022 ou Compile Within Editor)
- [ ] MCP: Remplir Waves array
- [ ] Ouvrir UE5 Editor, vérifier MercenairesWaveSpawner en place

---

### **PRIORITÉ 3 — Playtest complet (titre → loadout → match)**
- [ ] Lancer PIE (Simulate ou Play)
- [ ] Title Screen "REVENANTOPS" → cliquer JOUER
- [ ] Loadout: sélectionner arme → CONFIRMER
- [ ] **Vérifications** (tous doivent être ✅):
  - [ ] Arme visible au spawn?
  - [ ] Ennemis spawn wave 1?
  - [ ] HUD affiche score/temps/combo?
  - [ ] Coups de feu tuent ennemis?
  - [ ] GameOver après 5min?
  - [ ] Leaderboard save/affiche?

---

### Anciennes tâches validées ✅
- [x] Arène géométrie (22 pièces) — en place
- [x] Spawn points zombies (10 points) — en place
- [x] Pickups placés — en place
- [x] NavMesh — en place
- [x] Éclairage — en place
- [x] Widgets layout (Title/Loadout/GameOver/Leaderboard) — fonctionnels end-to-end
- [x] PlayerController flow — testé et validé
- [x] Widget classe assignation dans BP_ThirdPersonPlayerController — FAIT (vérifier Details panel)

## Accumulated Context

### Decisions

- [Roadmap] Camera OTS deplacee en Phase 7 — la camera actuelle fonctionne, on peaufine plus tard
- [Roadmap] Zombies en Phase 1 : le core gameplay (ennemis) est la priorite
- [Roadmap] Zombies avant Arsenal : les armes doivent etre calibrees contre les types d'ennemis existants
- [Roadmap] Timer & Score depend des Zombies (les kills doivent scorer) mais pas de l'Arsenal (independant)
- [Roadmap] Arene en Phase 4 apres le systeme de score pour placer les pickups de bonus temps correctement
- [Roadmap] Loadout en Phase 5 apres Arsenal : pas de selection possible sans armes implementees
- [Roadmap] Phase 9 Integration sans nouveaux requirements — valide les 37 en conditions demo reelles
- [Phase 01-zombies] SetMaxHealth public setter sur HealthComponent pour customisation en sous-classes C++
- [Phase 01-zombies] ZombieBase uses separate UpdateZombieCombat instead of overriding non-virtual EnemyBase methods
- [Phase 01-zombies] ZombieExploder self-destructs via ApplyDamage(9999) pour préserver delegate OnEnemyDied
- [Phase 01-zombies] ProjectileClass UPROPERTY assigné en BP — pas de ConstructorHelpers pour éviter couplage asset path
- [Phase 01-zombies] MaxAliveEnemies=15 pour support wave 10 ; wave config via BlueprintFunctionLibrary
- [Phase 02-arsenal] Constructor-defaults-only pattern : stats dans constructor, comportement dans WeaponBase
- [Phase 02-arsenal] Melee override FireShot avec sphere trace — pas de Super call, SweepSingleByChannel
- [Phase 02-arsenal] Melee WeaponCategory corrigé : Pistol → Melee (enum Melee ajouté à EWeaponCategory)
- [Phase 03-timer] MercenairesGameState : 5 min, points par type zombie, combo x1-x20, decay 5s sans kill
- [Phase 03-timer] Points : Slow=50, Runner=75, Tank=200, Spitter=150, Exploder=125
- [Phase 04-arena] Géométrie via MCP Python : 22 pièces /Engine/BasicShapes/Cube scalées
- [Phase 05-loadout] LoadoutWidget : UWidget::Slot conflict → paramètre renommé SlotIndex
- [Phase 06-ui] PlayerController flow : title→loadout→match→gameover→leaderboard
- [Phase 07-camera] SpringArm OTS : TargetArmLength=120, SocketOffset=(0,50,60), lag activé
- [Phase 07-camera] ADS zoom : ADSFOV depuis GetADSFOV() CDO, offset (0,40,55), ArmLength=80
- [Phase 08-audio] Audio/VFX : UPROPERTY hooks dans WeaponBase (FireSound, MuzzleFlashVFX) et EnemyBase (DeathSound, HitSound)
- [Phase 09-integration] PlayerController properties non exposées en BP — fix requis manuellement

### Blockers Critiques (Session 2026-03-24)

| Blocker | Cause | Fix | Priorité |
|---------|-------|-----|----------|
| **Arme invisible** | Socket "WeaponSocket" n'existe pas | C++ → hand_r | CRITIQUE |
| **Pas d'ennemis** | StartEncounter() jamais appelé | C++ auto-start dans StartMercenairesMatch() | CRITIQUE |
| **Waves vides** | Array non configuré en éditeur | MCP config après recompil | CRITIQUE |
| **Meshes armes** | BPs sans StaticMesh assigné | Optionnel: assigner placeholder | OPTIONNEL |

### Blockers Techniques Connus
- **MCP TCP bloqué pendant PIE Play** — config uniquement en Editor/Simulate mode
- **BlueprintGeneratedClass properties** : Les UPROPERTY de PlayerController sont assignables en Details panel (✅ vérifié comme FAIT)
- **Widget layouts** : Les WBP_* layouts sont fonctionnels end-to-end (Title→Loadout→Match flow validé)
- **Audio assets** : Placeholders non assignés → optionnel pour démo basique

### Roadmap Evolution

- Phase 10 added: Editor Setup & Playtest — configuration éditeur UE5 (arena, widgets, PlayerController, audio) + validation partie complète

### Leçons Techniques Importantes

- LiveCoding ne peut PAS créer de nouvelles classes — full build via Build.bat obligatoire
- Après full build, redémarrer UE5 pour que les BPs corrompus soient rechargés proprement
- `hasattr(unreal, 'ClassName')` = méthode fiable pour vérifier qu'une classe C++ est chargée
- BPs créés pendant LiveCoding se corrompent (références LIVECODING_Xxx_0) → supprimer et recréer après full build
- OFPA (One File Per Actor) = les External Actors sont stockés séparément dans Content/__ExternalActors__/

## Session Continuity

**Last session**: 2026-03-23 — C++ Phases 1-9 complètes, 20 BPs créés, flow end-to-end validé
**Current session**: 2026-03-24 — Identification des 4 blocages critiques restants
**Next session**: 2026-03-25 — Appliquer 2 fixes C++ + recompiler + MCP config + playtest PIE complet

### Resume Points for 2026-03-25
1. Apply Fix 1 (hand_r socket) + Fix 2 (WaveSpawner auto-start) in C++
2. Recompile (Visual Studio 2022 or Compile Within Editor)
3. MCP: Configure Waves array (3 vagues, 11 ennemis total wave 1)
4. PIE test: Title→Loadout→Match, verify weapon visible + enemies spawn + HUD shows
5. If all OK: declare Phase 10 DONE, start Phase 11 (Polish/Export)
