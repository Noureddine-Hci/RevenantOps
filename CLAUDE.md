# RevenantOps — Regles Claude Code

## Projet
- **RevenantOps** : Third-Person Shooter (mode Mercenaires) en Unreal Engine 5.7, C++
- Developpeurs : Noureddine Houichi + Jilani (depuis 2026-04-06)
- Langue de communication : **francais**

---

## AUTO-INVOCATION RULES — RÈGLES IMPÉRATIVES POUR CLAUDE

**Les devs ne connaissent pas par cœur les skills/agents disponibles. C'est à TOI, Claude, de détecter les patterns et d'invoquer les bons outils AUTOMATIQUEMENT. Jamais attendre que le dev tape `/gate-check` — invoque de toi-même.**

### Règle d'or
Avant de répondre à une demande, **scan la demande** pour détecter un pattern ci-dessous. Si match → invoque l'outil SANS demander. Annonce brièvement ce que tu fais (« Je lance /gate-check 17 avant de confirmer »).

### Skills à auto-invoquer

| Pattern dans la demande du dev | Skill à invoquer |
|---|---|
| "phase X terminée", "on peut passer à la suite", "c'est fini", "on valide la phase" | `gate-check [phase]` |
| "je teste en PIE", "lance le jeu", "je vais faire un test", "avant de merger" | `smoke-check` |
| "on attaque la phase X", "prochaine phase", "planifie la suite", "qu'est-ce qu'on fait après" | `sprint-plan [phase]` |
| "combat feel", "polish combat", "camera shake + X", "tir + VFX + son", problème qui touche plusieurs systèmes gameplay | `team-combat` (lance plusieurs agents en parallèle) |
| "équilibré ?", "trop fort", "DataTable", "DT_WeaponStats", "balance" | `balance-check` |
| "review mon code", "j'ai fini de coder", "avant de committer", nouveau C++ écrit | `code-review [fichier]` |
| "bug", "ça crash", "ça marche pas", "problème avec", "erreur" | `bug-triage` |
| "ça lag", "FPS bas", "c'est lent", "optimiser", "perf" | `perf-profile` |

### Agents à auto-déléguer (via Task)

| Pattern | Agent |
|---|---|
| Question C++ UE5, architecture générale, plugin, Build.cs | `unreal-specialist` |
| Problème Blueprint, graph spaghetti, BP/C++ boundary, review BP | `ue-blueprint-specialist` |
| HUD, inventaire, widgets, WBP, crosshair, Slate | `ue-umg-specialist` |
| GAS, abilities, gameplay effects, attribute sets, tags | `ue-gas-specialist` |

### Règles de lancement parallèle

Quand une tâche touche **plusieurs domaines** (C++ + BP + UI + VFX), **lance les agents en parallèle** dans un seul message (pas séquentiel) :
- Exemple : "Ajoute un système de réchargement visuel" → lancer `unreal-specialist` (C++), `ue-umg-specialist` (HUD bar), `ue-blueprint-specialist` (BP weapon) EN PARALLÈLE

### Format d'annonce (court, pas de blabla)

Avant d'invoquer :
> « Pattern détecté : [X]. Je lance `/[skill]` pour [raison]. »

Puis tu exécutes. Tu ne demandes PAS la permission pour des skills/agents — tu les utilises comme tes propres outils.

### Exceptions — NE PAS auto-invoquer si :
- La demande est triviale (ex: "c'est quoi UE5 ?")
- Le dev est clairement en mode conversation/brainstorm
- On est en plein milieu d'une autre tâche déjà lancée

---

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

## Etat Phase 19 — Pickups adaptatifs + Caisses destructibles (COMPLETE 2026-04-26)

### Systeme de pickups unifie
- `IPickupInterface` : interface UE5 (GetPickupIcon, GetPickupDisplayName, GetPickupDisplayAmount, TryPickupInteract)
  - Icone lue automatiquement depuis le BP — plus de DropIcon a renseigner manuellement
- `AHealthPickup` : soins en % de vie max (HealPercent = 0.25/0.5/1.0), prompt E + icone
- `AAmmoBonusPickup` : refacto public API, StartLifetimeTimer() separe de BeginPlay
- `RevenantOpsCharacter` :
  - `PendingInteractable` (AActor*) remplace PendingPickup + PendingHealthPickup
  - `ClearPendingPickup()` evite l'ambiguite nullptr sur les surcharges
  - `GetWeaponInventory()` expose WeaponInventory en lecture seule

### Drops ennemis adaptatifs
- `FAmmoDropEntry` : DropIcon/DropDisplayName supprimes (viennent du BP via IPickupInterface)
- `EnemyBase::HandleDeath` : filtre par EAmmoType selon armes du joueur

### Caisses destructibles ameliorees
- `FCrateLootEntry` : DropChance + Weight + AmmoTypeFilter par entree
- `ELootMode::Independent` : chaque entree tiree selon DropChance (soins/munitions)
- `ELootMode::PickOne` : 1 seule entree choisie par tirage pondere (Weight) — caisses armes
- Etat endommage : DamagedMaterial + BP_OnDamagedState quand HP < DamagedThreshold
- Filtre adaptatif AmmoTypeFilter sur chaque entree

### Talents
- `UTalentDefinition` : UPrimaryDataAsset (ReloadSpeed, DamageResistance, AmmoCapacity, MoveSpeed, MaxHealth, Stamina)
- `ApplyTalents()` sur ARevenantOpsCharacter

### Portrait 3D CharacterSelect
- `ACharacterPreviewActor` : SceneCapture2D + RT 512x910
- `CharacterSelectWidget` : grille inventaire 3x3 + talents + portrait live

### Patterns importants appris
- `SetPendingPickup(nullptr)` = AMBIGUITE sur surcharges → toujours `ClearPendingPickup()`
- Proprietes post-spawn (DropLifetime) : doivent etre settees avant StartLifetimeTimer(), pas dans BeginPlay
- Membres protected → public si accessed depuis EnemyBase/DestructibleObject
- Dernier commit : affb9a0 (merge main)

---

## Etat Phase 17 — Inventaire RE5 + Viseur CS (COMPLETE 2026-04-13)
- `FInventoryItem` struct : type, DisplayName, Description, Quantity, WeaponClass, HealAmount, TimeBonusSeconds, ItemIcon
- `UInventoryWidget` : 9 slots 3x3, navigation ZQSD/flèches, Tab ouvre/ferme, E utilise
  - `RebuildWidget` avec guard `IsDesignTime()` — évite corruption WBP dans l'éditeur
  - `FInputModeUIOnly` quand ouvert (bloque tir + mouvement)
  - Time dilation 0.3x quand inventaire ouvert
- `WeaponBase` : champ `WeaponIcon` (UTexture2D*) visible dans Details → Weapon|Identity
- `RevenantOpsCharacter` : `Inventory[9]`, `AddItemToInventory`, `UseInventoryItem`
  - Slots 0-1 auto-remplis avec les armes du loadout via `SpawnDefaultWeapons`
- `RevenantOpsPlayerController` : `ToggleInventory()` sur Tab, `OnInventoryItemUsed`, `bInventoryOpen`
- Viseur CS : 4 traits `CrosshairTop/Bottom/Left/Right` (UImage brush solide)
  - S'écartent dynamiquement selon `Weapon->GetCurrentSpread()`
  - Se resserrent en ADS, interpolation fluide (`CrosshairInterpSpeed`)
  - Créés dans `NativeConstruct` si le WBP a déjà un RootWidget
- Icônes importées : AK74, M92F, SHOTGUN, VZ61 dans `Content/Mercenaires/UI/Icons/`
- Dernier commit : fc27952

### PATTERNS IMPORTANTS — Live Coding
- **Changements structurels** (nouvelles classes, nouveaux UPROPERTY) → Live Coding boucle → **redémarrer UE5**
- **WBP corrompu** ("Impossible de charger WidgetTree") → supprimer + recréer le WBP
- `RebuildWidget` modifie le WidgetTree en éditeur → toujours ajouter `if (!IsDesignTime())`
- `NativeConstruct` est appelé APRÈS que Slate est construit → pas adapté pour créer le RootWidget

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

## Etat Phase 16 — Demo Jouable (2026-04-05, NON COMMITÉ)

### C++ FAIT (compilé UBT ✅)
- `RevenantOpsHUD` : BuildDefaultUI() crée 18 widgets programmatiquement si WBP vide
- `RevenantOpsPlayerController` : ReceivedPlayer() remplace BeginPlay pour CreateWidget
  (fix crash "CreateWidget sans joueur joint")

### MCP FAIT (assets sauvegardés ✅)
- BP_ThirdPersonGameMode → GameStateClass = BP_MercenairesGameState
- BP_ThirdPersonPlayerController : 5 widget classes + 5 AvailableWeaponClasses
- BP_ZombieSpitter → ProjectileClass = BP_ZombieProjectile
- BP_EnemyWaveSpawner créé + placé dans Lvl_ThirdPerson
  (3 vagues : 5 Slow / 4Slow+3Runner / 4Runner+2Tank, 28 SpawnPoints, MaxAliveEnemies=8)

### ABP_Mercenaire — ÉTAT CORRIGÉ (2026-04-05)
- Graph[16] Transition Locomotion→Armed : can_enter_transition = **False**
  (était True → forçait MF_Rifle_Idle_ADS en permanence)
- Graph[2] Walk/Run state : BlendSpace = **BS_IdleRun** (était BS_Pistol_Walk_Run)
  → fix crash sprint "Array index 3 out of bounds"
- BS_Pistol_Walk_Run : 4 samples (0/150/375/950) avec pistol anims corrects
- ATTENTION : Graph[15] Armed seq = MF_Pistol_Idle_ADS (si transition réactivée)

### CRASH CONNU RÉSOLU
- "Array index 3 into array of size 3" lors du sprint
- Cause : BS_Pistol_Walk_Run mal calibré (vitesse max=375 < SprintSpeed=900)
- Fix : BS_IdleRun (pré-existant, stable toutes vitesses)

### À COMMITTER (après validation PIE)
```
feat(16): demo jouable — HUD programmatique + ReceivedPlayer fix + WaveSpawner + animations
```

## PROCHAINE ETAPE (Jilani — branche J)
1. Tester en PIE : Tab → inventaire s'ouvre, ZQSD navigue, E utilise
2. Tester viseur : 4 traits visibles au centre, s'écartent en tirant, se resserrent en ADS
3. Assigner WeaponIcon dans BP_SMG, BP_Shotgun, BP_Sniper (BP_Pistol + BP_AssaultRifle déjà faits)
4. À décider : Phase 18 (sons Kenney, animations upper body armed, polish arena)

---

## COLLABORATION — Noureddine & Jilani (depuis 2026-04-06)

### Identification du dev actif
Au début de chaque session Claude, le dev se présente : "Je suis Noureddine" ou "Je suis Jilani".
Si non précisé, Claude doit demander : "Tu es Noureddine ou Jilani ?"

### Git — Branches & Convention
```
main              → stable, buildable, validé en PIE. JAMAIS travailler directement dessus.
noureddine/phase-XX  → branche de travail Noureddine
jilani/phase-XX      → branche de travail Jilani
```
- Commits : format `[N] feat(scope): desc` ou `[J] feat(scope): desc`
  ex: `[J] feat(15): arena zone B eclairage`
- Workflow merge : push ta branche → PR → review par l'autre → merge dans main → les deux font `git pull`

### Règle d'or — Ownership des assets binaires
Les `.uasset` UE5 sont des **binaires non-mergeables**. Un seul dev les touche à la fois.
Avant de modifier un Blueprint ou le level :
- Annoncer à l'autre (Discord/WhatsApp) : "je prends BP_ZombieSpitter"
- L'autre ne touche pas cet asset jusqu'au merge

Ownership par défaut :
- `Lvl_ThirdPerson.umap` → UN seul à la fois, annoncer avant
- `ABP_Mercenaire` → UN seul à la fois, annoncer avant
- BPs gameplay → pareil, jamais les deux en même temps

### Division du travail v3.0
```
Phase 14 (Animations posture)   → Noureddine (déjà en cours)
Phase 15 (Arena retravaillée)   → Jilani
Phase 16 (HUD & Menus polish)   → à décider après phases 14+15
```

### Tuto — Commencer une session
1. `git pull origin main` — récupérer la dernière version stable
2. `git checkout ta-branche` (ex: `jilani/phase-15`)
3. Annoncer à l'autre ce que tu vas toucher
4. Travailler → committer sur ta branche avec le préfixe `[N]` ou `[J]`

### Tuto — Merger vers main
1. `git push origin ta-branche`
2. `gh pr create` ou PR GitHub
3. L'autre dev review et approuve
4. Merge dans main
5. Les deux font `git pull origin main`

### Onboarding Jilani (à faire UNE FOIS sur sa machine)
```
1. Cloner le repo : git clone <url-du-repo> RevenantOps
2. Créer ~/.claude/primer.md avec le template ci-dessous
3. Créer sa branche : git checkout -b jilani/phase-15
4. Lire CLAUDE.md avant toute session
5. Ne jamais committer sur main directement
```

Template `~/.claude/primer.md` pour Jilani :
```markdown
# Active Project: RevenantOps (UE5 TPS — Mode Mercenaires)
## Dev: JILANI
## Location: [chemin local sur ta machine vers le repo]
## Branche active: jilani/phase-15
## Etat: v3.0 en cours — Phase 14 par Noureddine, Phase 15 (Arena) par toi
## Prochaine étape: [à remplir après ta première session]

## Contexte rapide
- UE5.7, C++ + Blueprints
- Gameplay loop complet (title → loadout → match → gameover → leaderboard)
- 5 types de zombies, 6 armes, timer/score/combo
- MCP TCP port 12029 : python Scripts/mcp_run.py script.py timeout
- JAMAIS ouvrir level via MCP | JAMAIS MCP pendant PIE
- Live Coding : Ctrl+Alt+F11
```
