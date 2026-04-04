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

## Workflow Autonome C++
Boucle obligatoire pour toute modification C++ :
1. Ecrire/modifier le code .h/.cpp
2. Compiler (VS Build ou Live Coding Ctrl+Alt+F11)
3. Si erreur : lire le message, corriger UNIQUEMENT le probleme, recompiler
4. Tester en PIE ou via tests unitaires
5. Si echec : analyser le log, corriger la logique, retester
6. Valider : zero erreur, zero warning critique, conventions UE5 respectees

Ne pas finaliser une tache sans preuve que ca compile et fonctionne.

## Checklist Validation (avant de declarer une tache complete)
- Code compile sans erreur (MSVC / VS 2026)
- Tests passent ou PIE validee
- Pas de warnings critiques
- Conventions UE5 (CamelCase, prefixes U/A/F/S/E)
- Pas de duplication inutile
- Pas de memory leaks (UPROPERTY pour GC, pas de raw new sans raison)

## Fichiers d'etat — REGLE OBLIGATOIRE
- `primer.md` : etat courant du projet — **mettre a jour en DEBUT et FIN de chaque session**
- `CLAUDE.md` : regles + etat technique — **mettre a jour en DEBUT et FIN de chaque session**
- Memoire persistante dans `.claude/projects/.../memory/`

## Style commits
- Format : `type(scope): description`
- Types : feat, fix, docs, refactor, chore
- Scope = numero de phase ou nom du module
- Regrouper les changements par feature logique (pas un commit par fichier)

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

## DataTable BP CDO — Workflow Python TCP (session 2026-03-29)
```python
# Pattern valide pour setter une propriete C++ heritee sur un BP CDO via TCP
bp = unreal.load_asset(path)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)   # compile AVANT
cdo = unreal.get_default_object(bp.generated_class())
handle = unreal.DataTableRowHandle(data_table=dt, row_name='RowName')
with unreal.ScopedEditorTransaction('Set prop') as t:
    bp.modify()
    cdo.modify()
    cdo.set_editor_property('prop_name', handle)
unreal.BlueprintEditorLibrary.compile_blueprint(bp)   # compile APRES
unreal.EditorAssetLibrary.save_asset(path)
# IMPORTANT: Clean+Build VS requis quand UE5 est ferme avant ouverture
# Sans ScopedEditorTransaction + modify() : la valeur ne survit pas aux compilations
```

---

## Etat Phase 11 — DataTables (COMPLETE 2026-03-29)
- FWeaponTableRow + FEnemyTableRow structs C++ OK
- WeaponBase::ApplyWeaponDataRow() + ZombieBase::ApplyEnemyDataRow() OK
- DT_WeaponStats (6 lignes) + DT_EnemyStats (5 lignes) crees et peuples
- 11 BPs configures (WeaponDataRow / EnemyDataRow pointe vers DTs)
- PIE validee : Pistol = 12 balles (DT), Tank = 400 HP
- Dernier commit : 17cb395

## Etat Phase 12 — A FAIRE
- ASSET-01 : Trouver et importer meshes armes depuis Fab.com
- ASSET-02 : Assigner meshes dans BP_Pistol, BP_AssaultRifle, etc.

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

---

## Etat Phase 12 — Weapon Meshes (COMPLETE 2026-04-02)
- 6 meshes Kenney importes : blaster-a/c/e/g/m + grenade-a
- Assignes aux BPs : Pistol=blaster-a, AssaultRifle=blaster-c, SMG=blaster-e, Shotgun=blaster-g, Sniper=blaster-m, Melee=grenade-a
- Rotation socket : (Pitch=0, Yaw=90, Roll=90) pour modeles Y-forward Kenney sur hand_r
- Camera : HipArmLength=350, ADSArmLength=250 (fix UpdateCameraFOV hardcode)
- bDoCollisionTest=False sur SpringArm (evite zoom intempestif)
- Dernier commit : 1d0cba2

## Etat Phase 13 — Weapon Animation System (COMPLETE 2026-04-02)
- AnimMontages crees depuis AnimSequences existants :
  AM_Rifle_Fire/Reload/Equip + AM_Pistol_Fire/Reload/Equip (dans /Mercenaires/Anims/Montages/)
- CharacterFireMontage/ReloadMontage/EquipMontage assignes sur les 6 weapon BPs
- bIsArmed (BlueprintReadOnly) ajoute a ARevenantOpsCharacter — set dans SpawnDefaultWeapons()
- ABP_Mercenaire cree (duplique ABP_Unarmed, meme skeleton)
- BP_ThirdPersonCharacter AnimClass = ABP_Mercenaire
- Transition Locomotion->Armed : can_enter_transition=True (le perso a toujours une arme)
- MF_Rifle_Idle_ADS = asset override null-sequence dans Armed state
- ATTENTION : bIsArmed C++ necess. Live Coding (Ctrl+Alt+F11) avant test PIE

## ABP Mercenaire — API Python utile
```python
# Lire les graphes d'animation
graphs = abp.get_animation_graphs()  # retourne AnimationGraph/StateGraph/TransitionGraph
g = graphs[15]  # etat Armed
nodes = g.get_graph_nodes_of_class(unreal.AnimGraphNode_SequenceEvaluator)

# Configurer la regle de transition (TransitionResult)
tr = g16.get_graph_nodes_of_class(unreal.AnimGraphNode_TransitionResult)[0]
new_node = unreal.AnimNode_TransitionResult()
new_node.can_enter_transition = True
with unreal.ScopedEditorTransaction('...') as _:
    tr.modify()
    tr.set_editor_property('Node', new_node)

# Override d'asset (null -> anim)
abp.add_node_asset_override(None, anim_asset)
```

## Etat Phase 14 — Demo Polish / Combat Feel (EN COURS 2026-04-04)

### C++ FAIT (compile VS, PAS commit)
- `CameraShakes.h/.cpp` : UCS_WeaponFire (0.15s subtil) + UCS_TakeDamage (0.25s fort)
- `Build.cs` : module `EngineCameras` ajoute (requis pour UDefaultCameraShakeBase)
- `WeaponBase` : hit marker (ShowHitMarker via PC->HUD), camera shake au tir, ReloadStartTime, GetReloadProgress()
- `RevenantOpsHUD` : WaveText, ReloadBar, DamageDirectionImage, KillNotificationText (BindWidgetOptional), UpdateWaveDisplay/UpdateReloadBar/UpdateDamageDirection/ShowKillNotification
- `EnemyBase` : HitFlashMaterials (MID), HitFlashTimer, flash blanc 0.15s sur hit
- `RevenantOpsCharacter` : cache HealthComp, bind OnDamageReceived → shake TakeDamage + ShowDamageDirection

### ASSETS MCP FAITS
- 3 Niagara : NS_MuzzleFlash, NS_Impact_Surface, NS_Impact_Blood (dans /Mercenaires/VFX/)
- VFX assignes aux 6 weapon BPs (CDO via ScopedEditorTransaction)
- WBP_Mercenaires_HUD cree (parent URevenantOpsHUD) + assigne sur BP_ThirdPersonPlayerController

### RESTE A FAIRE (prochaine session)
1. **MANUEL UE5** : ouvrir WBP_Mercenaires_HUD, ajouter les widgets avec noms EXACTS :
   HealthBar, StaminaBar, TimerText, ScoreText, WaveText, ReloadBar,
   HitMarkerImage, KillNotificationText, AmmoCurrentText, AmmoReserveText, WeaponNameText
2. Commit : `feat(14): combat feel — camera shake + hit flash + VFX + HUD counters`
3. Sons Kenney CC0 : importer + assigner FireSound/ReloadSound/EmptySound/HitSound/DeathSound
4. Materiau ennemi : ajouter param scalaire "HitFlash" (emissive blanc) pour l'effet flash

## Camera Shake UE5.7 — PATTERN VALIDE
```cpp
// JAMAIS ChangeRootShakePattern<T>() dans le constructeur (NewObject illegal)
// CORRECT : recuperer le pattern par defaut et le configurer
UCS_WeaponFire::UCS_WeaponFire(const FObjectInitializer &ObjInit) : Super(ObjInit) {
    if (UPerlinNoiseCameraShakePattern *P =
            Cast<UPerlinNoiseCameraShakePattern>(GetRootShakePattern())) {
        P->Duration = 0.15f;
        P->Pitch.Amplitude = 0.4f;
        P->Pitch.Frequency = 25.f;
    }
}
// Module Build.cs : "EngineCameras" (pas GameplayCameras)
// Include : "Shakes/DefaultCameraShakeBase.h" + "Shakes/PerlinNoiseCameraShakePattern.h"
// Parent class : UDefaultCameraShakeBase
```

## Widget Blueprint Python — LIMITATION CONNUE
- `widget_blueprint.widget_tree` → AttributeError (pas expose Python)
- `get_editor_property('WidgetTree')` → "property is protected"
- **Impossible de creer/modifier des widgets UMG via Python MCP**
- Solution : creer le WBP avec le bon parent class via Python, mais ajouter les widgets manuellement dans UE5

## PROCHAINE ETAPE
Phase 14 completion : widgets UMG manuels + sons + commit
