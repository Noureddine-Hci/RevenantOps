---
gsd_state_version: 1.0
milestone: v1.0
milestone_name: Demo Partageable Mercenaires
status: in_progress
stopped_at: Phase 4 Arena — geometrie spawned (22 pieces), manque spawn points + pickups + NavMesh + lighting. Widgets BP crees (WBP_TitleScreen/Loadout/GameOver/Leaderboard) mais PlayerController properties non mappees.
last_updated: "2026-03-23T00:00:00Z"
progress:
  total_phases: 9
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

## Ce Qui RESTE à Faire (avant test final)

### Priorité 1 — Éditeur UE5 obligatoire
- [ ] **Arène** : Placer EnemyWaveSpawner BP dans la level (spawn points zombies x4-6)
- [ ] **Arène** : Placer BP_TimeBonusPickup et BP_AmmoBonusPickup dans la level
- [ ] **Arène** : Volume NavMesh + Build Nav
- [ ] **Arène** : Directional Light sombre + Point Lights ambiance

- [ ] **Widgets** : Ouvrir WBP_TitleScreen → ajouter Text "REVENANTOPS", Bouton "JOUER", Bouton "QUITTER" → lier au C++ (BindWidget)
- [ ] **Widgets** : WBP_Loadout → layout grille armes + stats + bouton Confirmer
- [ ] **Widgets** : WBP_GameOver → afficher score/kills/combo + bouton Rejouer + bouton Leaderboard
- [ ] **Widgets** : WBP_Leaderboard → liste top 10 + bouton Retour

- [ ] **PlayerController** : Vérifier/exposer les UPROPERTY pour TitleScreenClass, LoadoutWidgetClass, GameOverWidgetClass, LeaderboardWidgetClass — les properties n'ont pas été trouvées via MCP (BlueprintGeneratedClass issue)
- [ ] **PlayerController** : Assigner les widget classes dans le BP_ThirdPersonPlayerController details panel manuellement

### Priorité 2 — Audio/VFX placeholders
- [ ] Assigner des SoundCue placeholders aux BP armes (FireSound property)
- [ ] Assigner des NiagaraSystem placeholders aux BP armes (MuzzleFlashVFX)

### Priorité 3 — Test final
- [ ] Lancer une partie complète (title → loadout → match 5min → gameover → leaderboard)
- [ ] Vérifier timer, score, combo sur kills zombies
- [ ] Vérifier switch armes, rechargement, melee

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

### Blockers Connus

- **BlueprintGeneratedClass properties** : Les UPROPERTY de PlayerController (TitleScreenClass, etc.) ne sont pas trouvées via MCP. Probablement besoin de les assigner manuellement dans l'éditeur, ou vérifier que les UPROPERTY sont bien déclarées avec EditAnywhere.
- **MCP TCP bloqué pendant PIE Play** — tests manuels nécessaires pour valider en jeu
- **Widget layouts** : Les WBP_* sont créés mais vides — les layouts doivent être faits dans UMG Editor
- **Audio assets** : Pas d'assets son dans le projet → utiliser StarterContent ou placer des Sound Wave placeholders

### Leçons Techniques Importantes

- LiveCoding ne peut PAS créer de nouvelles classes — full build via Build.bat obligatoire
- Après full build, redémarrer UE5 pour que les BPs corrompus soient rechargés proprement
- `hasattr(unreal, 'ClassName')` = méthode fiable pour vérifier qu'une classe C++ est chargée
- BPs créés pendant LiveCoding se corrompent (références LIVECODING_Xxx_0) → supprimer et recréer après full build
- OFPA (One File Per Actor) = les External Actors sont stockés séparément dans Content/__ExternalActors__/

## Session Continuity

Last session: 2026-03-23
Stopped at: Création des 4 Widget BPs (WBP_TitleScreen/Loadout/GameOver/Leaderboard) — PlayerController properties non mappées via MCP
Resume: Compléter setup éditeur (Arena + Widgets layout + PlayerController manual assignment)
