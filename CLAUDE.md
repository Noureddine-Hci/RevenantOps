# RevenantOps — Regles Claude Code

## Projet
- **RevenantOps** : Third-Person Shooter (mode Mercenaires) en Unreal Engine 5.7, C++
- Developpeur solo : Noureddine Houichi
- Langue de communication : **francais**

## Conventions C++
- Unreal Engine 5.7 — toujours utiliser les API a jour (pas de deprecated)
- `SetCrouchedHalfHeight()` au lieu de `CrouchedHalfHeight`
- Cast `AController` -> `APlayerController` pour `AddPitchInput`/`AddYawInput`
- Enhanced Input System (jamais legacy Input)
- Classes abstraites (Character, WeaponBase, HUD, EnemyBase) -> BP enfants requis
- Build.cs modules : Engine, InputCore, EnhancedInput, AIModule, NavigationSystem, StateTreeModule, UMG, Slate

## Interdictions
- **JAMAIS** changer de niveau via MCP TCP (`Open` command) — crash UE5
- **JAMAIS** envoyer de commandes MCP pendant PIE Play — le TCP est bloque
- **JAMAIS** utiliser des API depreciees UE 5.7 sans verifier la doc
- Pas de mocks dans les tests — on teste en conditions reelles

## Workflow MCP
- Utiliser **Simulate** ou **Editor mode** pour les operations MCP
- Configurer tout en mode Editor avant de demander de tester en PIE
- Demander a l'utilisateur d'ouvrir les niveaux manuellement

## Fichiers d'etat — REGLE OBLIGATOIRE
- `primer.md` : etat courant du projet — **mettre a jour en DEBUT et FIN de chaque session**
- `CLAUDE.md` : regles + etat technique — **mettre a jour en DEBUT et FIN de chaque session**
- Memoire persistante dans `.claude/projects/.../memory/`

## Style commits
- Format : `type(scope): description`
- Types : feat, fix, docs, refactor, chore
- Scope = numero de phase ou nom du module

## Screenshot Editor (workflow fiable)
```python
# Methode fiable pour screenshots viewport Editor :
unreal.AutomationLibrary.take_high_res_screenshot(1920, 1080, "nom_fichier")
# -> Saved/Screenshots/WindowsEditor/nom_fichier.png

# Deplacer la camera :
unreal.EditorLevelLibrary.set_level_viewport_camera_info(
    unreal.Vector(x, y, z),
    unreal.Rotator(pitch=pitch, yaw=yaw, roll=0)
)
# ATTENTION: Rotator constructor positionnel = (roll, pitch, yaw) — utiliser kwargs !
```

## Formule Z pour le level design (SM_Cube = 100u centre)
- Sol top = Z = -37.5 (floor centre a Z=-50, scale_z=0.25)
- Objet sur le sol : `center_z = -37.5 + scale_z * 50`
- Objet sureleve de H : `center_z = -37.5 + H + scale_z * 50`

---

## Etat Phase 10 — Blueprint Setup & Playtest

### CE QUI EST FAIT (session 2026-03-25)
- **C++ phases 1-9** : compilees, zero erreur
- **Fix 1** : socket `hand_r` (RevenantOpsCharacter.h, EnemyBase.h, WeaponBase.h)
- **Fix 2** : WaveSpawner auto-start dans StartMercenairesMatch()
- **Fix 3** : Waves configurees (3 vagues : 7/8/9 ennemis)
- **Nouvelle map "The Compound"** : 73 meshes, 16 lumieres, 5 zones
- **Widgets** : assignes dans BP_ThirdPersonPlayerController
- **Inputs** : Enhanced Input (WASD, Espace, Shift, Souris)
- **NavMesh + Spawn points** : 27 gameplay actors

### MEGA AUDIT + FIXES (session 2026-03-26)
- **4 agents paralleles** : C++ quality, architecture, game flow, config
- **Score global** : C++ 8/10, Archi 6.5/10, Flow 7/10, Config 9/10
- **9 fixes appliques, build 30/30 OK (60s)** :
  1. FScoreEntry extrait dans ScoreEntry.h (circular include casse)
  2. `bIsDead` guard dans EnemyBase::HandleDeath (double-death)
  3. `bLoadoutConfirmed` guard dans PlayerController (double-confirm)
  4. Null check OwnerPawn dans WeaponBase::FireShot (crash prevention)
  5. Timer clamp AVANT broadcast dans MercenairesGameState::Tick
  6. SpawnTimers clear dans EnemyWaveSpawner::StartEncounter
  7. `virtual GetKillPoints()` sur EnemyBase + overrides 5 zombies (decouplage)
  8. MercenairesGameState decouple des includes zombie (5 includes supprimes)
  9. RemoveDynamic avant AddDynamic sur OnMatchEnded (anti double-bind replay)

### LEVEL BLACKSITE — SESSION 2026-03-27 (TERMINÉ)
Via MCP TCP port 12029 (unreal-mcpython) :
- Map 5 zones construite (Scripts/build_blacksite.py)
- NavMeshBoundsVolume (0,250,300) scale 82x52x10 ✅
- 8 TargetPoints SP1-SP8 placés comme spawn ennemis ✅
- 5 Pickups : TB1 30s (-750,1900,700) | TB2 15s (3000,0,100) | TB3 15s (-3600,2200,100) | A1 Ammo (1500,0,100) | A2 Ammo (-2000,-800,100) ✅
- 13 PointLights (rouge/bleu/orange par zone) ✅
- NOTE : BP_TimeBonusPickup_20s inexistant → TB2 = 15s

### PROCHAINE ÉTAPE OBLIGATOIRE
1. UE5 Ctrl+S (sauvegarder niveau)
2. Ouvrir BP_EnemyWaveSpawner dans level, assigner les 8 TargetPoints dans Details > SpawnPoints[]
3. Vérifier GameMode → BP_MercenairesGameState
4. Commit : feat(10): blacksite level + spawners + pickups + lights

### A TESTER (PIE)
1. Title Screen -> JOUER
2. Loadout -> select arme -> CONFIRMER (guard double-click OK)
3. Match -> arme visible ? Ennemis spawn depuis les 8 TargetPoints ? HUD fonctionne ?
4. Kill = score correct (50/75/125/150/200 via GetKillPoints) ?
5. Timer countdown -> GameOver -> Leaderboard save ?
6. Replay depuis GameOver -> flow propre sans double-bind ?

### OPTIONNEL
- Meshes armes invisibles (BP_Pistol etc. sans StaticMesh) — cosmetique
