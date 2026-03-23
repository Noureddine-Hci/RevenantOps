# Phase 10: Editor Setup & Playtest — Research

**Researched:** 2026-03-23
**Domain:** Unreal Engine 5.7 — configuration éditeur, UMG, NavMesh, Blueprint wiring, audio placeholders, validation PIE
**Confidence:** HIGH (basé sur l'état réel du projet via lecture directe des sources C++ et summaries)

---

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| AREN-01 | 1 arène jouable avec zones variées (ouverte, couloirs, hauteurs) | Géométrie déjà en place (22 pièces), aucun travail restant sur la structure |
| AREN-02 | Points de spawn zombies répartis dans l'arène | EnemyWaveSpawner BP à placer + configurer wave data + SpawnPoints array |
| AREN-03 | Pickups de bonus temps placés dans l'arène | BP_TimeBonusPickup_30s / _15s à placer dans la level |
| AREN-04 | Pickups de munitions dans l'arène | BP_AmmoBonusPickup à placer dans la level |
| AREN-05 | Éclairage et ambiance sombre/tendue | Directional Light affaibli + Point Lights + Atmospheric Fog |
| UI-01 | Écran titre avec "Jouer" et "Quitter" | WBP_TitleScreen — layout UMG + BindWidgetOptional (PlayButton, QuitButton) |
| UI-02 | HUD en jeu (vie, munitions, arme, timer, score, combo) | WBP_HUD — layout UMG + BindWidgetOptional (tous nommés dans RevenantOpsHUD.h) |
| UI-03 | Écran de fin de partie (score, kills, meilleur combo, rejouer) | WBP_GameOver — layout UMG + BindWidgetOptional |
| UI-04 | Leaderboard local (top 10 scores) | WBP_Leaderboard — layout UMG + BP_OnScoresUpdated implementé |
| FX-01 | Sons de tir pour chaque arme | Assigner SoundBase dans BP_Pistol/AssaultRifle/SMG/Shotgun/Sniper (FireSound) |
| FX-02 | Sons d'impact (balle sur zombie, melee) | Assets StarterContent /Game/StarterContent/Audio/ |
| FX-03 | Sons de zombie (grognements, attaque, mort) | Assigner dans BP_ZombieSlow etc. (DeathSound, HitSound, GrowlSound, AttackSound) |
| FX-04 | Musique d'ambiance tension/action | Assigner dans BP_MercenairesGameState (BackgroundMusic UPROPERTY) |
| FX-05 | VFX de base (muzzle flash, impact sang, explosion) | Assigner NiagaraSystem dans BP armes (MuzzleFlashVFX) |
</phase_requirements>

---

## Summary

Phase 10 est une phase entièrement "éditeur" : tout le C++ est compilé et fonctionnel. Il ne reste aucun travail de code à écrire — uniquement de la configuration dans l'éditeur UE5.7 (UMG layouts, Blueprint Details Panel, placement d'acteurs dans la level, NavMesh, éclairage, assets audio/VFX).

Le défi principal est la contrainte MCP : le TCP MCP est bloqué pendant PIE Play, donc tous les tests de validation sont manuels. En revanche, MCP peut être utilisé en mode Editor pour automatiser les tâches répétitives (placement d'acteurs, set_property sur BPs, Blueprint Graph wiring pour les events).

Le second défi est le wiring UMG : les 4 WBPs sont créés mais vides. Les widgets C++ utilisent `BindWidgetOptional` ce qui signifie que les éléments UMG doivent avoir des noms qui correspondent exactement aux noms de UPROPERTY C++. Une erreur de nommage ne crash pas, mais le widget ne se met pas à jour.

**Recommandation principale :** Travailler en 4 blocs séquentiels — (1) Arène complète + NavMesh, (2) Widgets UMG layouts, (3) PlayerController wiring + Audio BPs, (4) Test PIE bout-en-bout.

---

## Standard Stack

### Outils disponibles dans ce projet

| Outil | Usage | Disponibilité |
|-------|-------|--------------|
| unreal-mcp (MCP TCP) | Placement acteurs, set_property, Blueprint graph | Editor mode UNIQUEMENT — bloqué en PIE |
| UMG Editor (manuel) | Layout des 4 WBP widgets | Manuel obligatoire pour le layout visuel |
| StarterContent audio | Sons placeholders | Déjà dans /Game/StarterContent/Audio/ si activé |
| Niagara StarterContent | VFX placeholders | /Game/StarterContent/FX/ si disponible |
| Build Navigation | Génération NavMesh | Via menu "Build > Build Navigation" dans l'éditeur |

### Assets audio disponibles UE5 StarterContent

| Path | Utilisation suggérée |
|------|---------------------|
| `/Game/StarterContent/Audio/Collapse01` | FireSound générique |
| `/Game/StarterContent/Audio/Explosion01` | ZombieExploder DeathSound |
| `/Game/StarterContent/Audio/Fire01` | BackgroundMusic placeholder |
| `/Engine/EditorSounds/` | Sons d'interface UI |

**Confidence:** MEDIUM — StarterContent présence dépend de si le projet a été créé avec StarterContent activé. À vérifier en ouvrant le Content Browser.

---

## Architecture Patterns

### Pattern 1 : BindWidgetOptional — Contrat de nommage strict

**Ce que c'est :** `meta = (BindWidgetOptional)` en C++ lie automatiquement une UPROPERTY UButton/UTextBlock à un élément UMG du même nom exact. Si le nom ne correspond pas, le pointeur reste nullptr (pas de crash, mais pas de fonctionnalité).

**Règle critique :** Le nom du widget dans le panneau UMG Designer doit être **identique** à la UPROPERTY C++ en respectant la casse.

**Mapping complet des noms requis :**

Pour `WBP_TitleScreen` (parent : UTitleScreenWidget) :
```
UButton    nom : "PlayButton"
UButton    nom : "QuitButton"
UTextBlock (libre — pas de BindWidget pour le titre)
```

Pour `WBP_GameOver` (parent : UGameOverWidget) :
```
UTextBlock nom : "FinalScoreText"
UTextBlock nom : "TotalKillsText"
UTextBlock nom : "BestComboText"
UButton    nom : "ReplayButton"
UButton    nom : "QuitButton"
```

Pour `WBP_Leaderboard` (parent : ULeaderboardWidget) :
```
UButton    nom : "BackButton"  (si lié — vérifier si BindWidgetOptional dans .h)
UVerticalBox pour la liste dynamique
```

**Note WBP_Loadout :** LoadoutWidget.h n'expose PAS de BindWidgetOptional. Le widget utilise des BlueprintImplementableEvent (BP_OnSelectionChanged, BP_OnLoadoutConfirmed). Le layout est libre, mais les boutons doivent appeler `SelectWeapon()` et `ConfirmLoadout()` via Blueprint nodes.

**Note WBP_HUD :** Un Blueprint `WBP_HUD` héritant de `URevenantOpsHUD` n'est PAS encore créé. Il faut le créer et assigner dans le Details Panel du PlayerController (HUDWidgetClass).

### Pattern 2 : PlayerController Details Panel — Assignation manuelle obligatoire

**Le problème connu :** Les UPROPERTY du PlayerController n'ont pas été trouvées via MCP Python lors de la phase 09 (erreur BlueprintGeneratedClass). Le fix documenté dans 09-01-SUMMARY.md est :

1. Ouvrir `BP_ThirdPersonPlayerController` dans l'éditeur
2. Cliquer "Compile" dans la toolbar
3. Les propriétés apparaissent dans le Details Panel sous la catégorie "UI|Mercenaires"
4. Assigner manuellement :
   - TitleScreenClass → WBP_TitleScreen
   - LoadoutWidgetClass → WBP_Loadout
   - GameOverWidgetClass → WBP_GameOver
   - LeaderboardWidgetClass → WBP_Leaderboard
   - HUDWidgetClass → WBP_HUD (à créer)
   - AvailableWeaponClasses → array [BP_Pistol, BP_AssaultRifle, BP_SMG, BP_Shotgun, BP_Sniper]

**Les UPROPERTY sont `EditAnywhere`** — elles seront visibles dans le Details Panel une fois le BP recompilé.

### Pattern 3 : EnemyWaveSpawner — Configuration requise

Le spawner `AEnemyWaveSpawner` a deux modes de trigger :
- `bTriggerOnOverlap = true` : déclenche quand le joueur entre dans le TriggerVolume
- Manuel via `StartEncounter()` Blueprint call

Pour le mode Mercenaires (timer fixe de 5 min), le comportement recommandé est :
- `bInfiniteMode = true` sur le spawner principal — les zombies continuent de spawn jusqu'à la fin du timer
- `bTriggerOnOverlap = false` + appel `StartEncounter()` depuis MercenairesGameState::StartMatch() ou depuis le PlayerController après le loadout

**SpawnPoints array :** Le spawner utilise un `TArray<AActor*> SpawnPoints`. Il faut placer des `Target Point` actors dans la level et les référencer dans le Details Panel du EnemyWaveSpawner (ou utiliser MCP set_property).

### Pattern 4 : NavMesh Build

**Étapes :**
1. Placer un `Nav Mesh Bounds Volume` dans la level via Place Actors panel
2. Dimensions : couverture de toute l'arène — au minimum 5200x5200x500 (arène 5000x5000 + marge)
3. Menu `Build > Build Navigation` (ou Ctrl+Alt+F pour rebuild dans la viewport)
4. Vérification : activer l'affichage NavMesh via la touche P en viewport (paths s'affichent en vert)

**Note OFPA :** Avec One File Per Actor actif, les acteurs placés dans la level sont dans `Content/__ExternalActors__/`. Cela n'affecte pas le workflow, mais les External Actors ne sont pas lisibles via grep.

### Pattern 5 : Audio UPROPERTY assignation

Les UPROPERTY audio sont `EditDefaultsOnly` — modifications dans les **Blueprint Defaults** (ouvrir le BP, pas une instance). Via MCP, utiliser `set_property` sur le Blueprint asset (pas un acteur spawné).

**Mapping BP → UPROPERTY à assigner :**

| Blueprint | UPROPERTY | Asset suggéré |
|-----------|-----------|---------------|
| BP_Pistol | FireSound | StarterContent/Audio/Collapse01 ou équivalent |
| BP_AssaultRifle | FireSound | StarterContent/Audio/Collapse01 |
| BP_SMG | FireSound | StarterContent/Audio/Collapse01 |
| BP_Shotgun | FireSound | StarterContent/Audio/Explosion01 |
| BP_Sniper | FireSound | StarterContent/Audio/Collapse01 |
| BP_ZombieSlow | DeathSound, HitSound, GrowlSound, AttackSound | Placeholders |
| BP_MercenairesGameState | BackgroundMusic | StarterContent/Audio/Fire01 ou équivalent |

### Anti-Patterns à éviter

- **Créer un nouveau niveau via MCP "Open" command** : crash UE5 — le niveau est `Lvl_ThirdPerson` existant, déjà ouvert dans l'éditeur
- **Envoyer des commandes MCP pendant PIE Play** : TCP bloqué — toujours quitter PIE avant d'utiliser MCP
- **Nommer les widgets UMG sans respecter la casse** : BindWidgetOptional est case-sensitive (`PlayButton` ≠ `playButton`)
- **Assigner les UPROPERTY sur des instances plutôt que les Defaults** : les UPROPERTY `EditDefaultsOnly` ne sont pas visibles sur les instances placées dans la level
- **LiveCoding pour créer de nouvelles classes** : impossible — tout le C++ est déjà compilé, aucune nouvelle classe requise pour cette phase

---

## Don't Hand-Roll

| Problème | Ne pas construire | Utiliser à la place | Pourquoi |
|----------|------------------|---------------------|---------|
| Persistence du leaderboard | Custom file I/O | UGameplayStatics::SaveGameToSlot (déjà implémenté) | LeaderboardWidget.cpp gère déjà la sauvegarde |
| Sons de tir | New SoundCue assets | StarterContent/Audio/ existants | Placeholders fonctionnels immédiatement |
| NavMesh custom | Path finding manuel | Nav Mesh Bounds Volume + Build Navigation | Built-in UE5, fonctionne avec les zombies AI existants |
| Spawn distribution | Custom spawner logic | EnemyWaveSpawner.SpawnPoints[] existant | Le C++ du spawner gère déjà la distribution aléatoire |

---

## Common Pitfalls

### Pitfall 1 : BlueprintGeneratedClass properties non visibles

**Ce qui se passe :** Les UPROPERTY C++ n'apparaissent pas dans le Details Panel du Blueprint.
**Pourquoi :** Le Blueprint a été créé avant le build qui a ajouté les propriétés, ou n'a pas été recompilé depuis.
**Comment éviter :** Ouvrir le BP dans l'éditeur → cliquer "Compile" dans la toolbar (pas juste Save). Les propriétés apparaissent après recompilation.
**Signes d'alerte :** Details Panel vide dans la catégorie "UI|Mercenaires" sur BP_ThirdPersonPlayerController.

### Pitfall 2 : WBP_HUD manquant

**Ce qui se passe :** Le flow démarre mais le HUD en jeu n'affiche rien — timer, score, combo absents.
**Pourquoi :** `WBP_HUD` (Blueprint héritant de `URevenantOpsHUD`) n'est pas dans la liste des Blueprints créés. Le PlayerController a une propriété `HUDWidgetClass` qui doit pointer vers ce Blueprint.
**Comment éviter :** Créer `WBP_HUD` dans `/Game/Mercenaires/UI/` avec parent `URevenantOpsHUD` avant de tester.
**Signes d'alerte :** Pas d'affichage HUD pendant la partie.

### Pitfall 3 : EnemyWaveSpawner sans SpawnPoints → spawn à l'origine (0,0,0)

**Ce qui se passe :** Les zombies apparaissent tous au centre de la map ou dans le sol.
**Pourquoi :** `GetRandomSpawnTransform()` utilise `SpawnPoints[]` — si vide, retourne FTransform::Identity.
**Comment éviter :** Placer au minimum 4 `Target Point` actors dans la level, les référencer dans `SpawnPoints` array du EnemyWaveSpawner Details Panel.
**Signes d'alerte :** Zombies qui apparaissent au même point, souvent dans les murs.

### Pitfall 4 : NavMesh non construit → zombies immobiles

**Ce qui se passe :** Les zombies spawner normalement mais ne bougent pas / restent figés.
**Pourquoi :** L'IA utilise le NavMesh pour pathfinder. Sans Nav Mesh Bounds Volume et navigation buildée, les zombies ne trouvent pas de path.
**Comment éviter :** Toujours build navigation avant le premier test PIE après avoir ajouté la géométrie.
**Signes d'alerte :** Zombies debout immobiles, pas d'animation de marche.

### Pitfall 5 : TitleScreenWidget BP_OnPlayPressed non implémenté

**Ce qui se passe :** Le bouton "JOUER" ne fait rien ou crash.
**Pourquoi :** `OnPlayClicked()` en C++ appelle `BP_OnPlayPressed()` — un BlueprintImplementableEvent qui doit être implémenté dans WBP_TitleScreen Event Graph. Le PlayerController `ShowLoadoutScreen()` est appelé via cet event.
**Comment éviter :** Dans WBP_TitleScreen Event Graph, implémenter l'event "On Play Pressed" avec un appel au PlayerController `ShowLoadoutScreen()` ou laisser le PlayerController gérer via son delegate.

**Note architecturale :** Le PlayerController lie `OnPlayPressed` → `ShowLoadoutScreen()` dans son flow. La connexion exacte dépend de l'implémentation dans `RevenantOpsPlayerController.cpp` — à vérifier si c'est via delegate ou via cast dans l'event graph.

### Pitfall 6 : AvailableWeaponClasses vide → Loadout sans armes

**Ce qui se passe :** L'écran de sélection de loadout apparaît mais sans armes à sélectionner.
**Pourquoi :** `AvailableWeaponClasses` (TArray dans le PlayerController) est assigné manuellement dans le Details Panel.
**Comment éviter :** Assigner les 5 classes dans le Details Panel : BP_Pistol, BP_AssaultRifle, BP_SMG, BP_Shotgun, BP_Sniper.

---

## Code Examples

### Vérification BindWidget en éditeur (Python MCP)

```python
# Vérifier que le BP est compilé et les propriétés visibles
import unreal

bp = unreal.load_asset('/Game/Mercenaires/BP_ThirdPersonPlayerController')
obj = unreal.get_default_object(bp.generated_class())
# Si get_default_object retourne None, le BP doit être recompilé manuellement
```

### Placement Target Points pour spawn points (MCP Python)

```python
import unreal

# Placer 4 Target Points aux 4 coins de l'arène
spawn_positions = [
    unreal.Vector(-2000, -2000, 100),  # Coin SW
    unreal.Vector(-2000,  2000, 100),  # Coin NW
    unreal.Vector( 2000, -2000, 100),  # Coin SE
    unreal.Vector( 2000,  2000, 100),  # Coin NE
    unreal.Vector(-1500,     0, 100),  # Couloir gauche
    unreal.Vector(    0, -1500, 100),  # Centre-sud
]
for i, pos in enumerate(spawn_positions):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.TargetPoint,
        pos,
        unreal.Rotator(0, 0, 0)
    )
    actor.set_actor_label(f'SpawnPoint_{i+1}')
```

### Build Navigation via Python MCP (alternative éditeur)

```python
import unreal
# Déclencher le build navigation depuis Python
subsystem = unreal.get_editor_subsystem(unreal.NavigationSystemEditorSubsystem)
subsystem.build()
# Note : méthode disponible en UE 5.3+ via EditorSubsystem
```

### Flow PlayerController — Connexion TitleScreen → Loadout

Le flow C++ dans `RevenantOpsPlayerController.cpp` est :
- `BeginPlay()` → `ShowTitleScreen()` → crée widget → addToViewport → SetInputMode UI Only
- Le widget `UTitleScreenWidget::OnPlayClicked()` appelle `BP_OnPlayPressed()`
- Dans WBP_TitleScreen Event Graph : implémenter "On Play Pressed" → `Cast to ARevenantOpsPlayerController` → `ShowLoadoutScreen()`

Alternative simplifiée (si cast compliqué) : dans WBP_TitleScreen, "On Play Pressed" → `Get Player Controller` → `Show Loadout Screen` (fonction exposée en BlueprintCallable).

---

## Validation Architecture

### Test Framework

| Property | Value |
|----------|-------|
| Framework | Tests manuels PIE — pas de framework automatisé applicable (UE5 PIE, MCP bloqué pendant PIE) |
| Config file | N/A |
| Quick run command | Lancer PIE manuellement dans l'éditeur UE5 |
| Full suite command | Parcours complet : Title → Loadout → Match 5min → GameOver → Leaderboard |

### Phase Requirements → Test Map

| Req ID | Comportement | Type | Commande automatisée | Fichier test |
|--------|-------------|------|---------------------|--------------|
| AREN-02 | Zombies spawn depuis points distribués | Visuel PIE | Manuel — observer spawn en PIE | ❌ Manuel only |
| AREN-03/04 | Pickups apparaissent et disparaissent à la ramasse | Visuel PIE | Manuel | ❌ Manuel only |
| AREN-05 | Ambiance sombre visible | Visuel | Manuel | ❌ Manuel only |
| UI-01 | TitleScreen affiche + boutons répondent | Fonctionnel PIE | Manuel | ❌ Manuel only |
| UI-02 | HUD affiche timer/score/combo | Fonctionnel PIE | Manuel | ❌ Manuel only |
| UI-03 | GameOver affiche score final | Fonctionnel PIE | Manuel | ❌ Manuel only |
| UI-04 | Leaderboard liste top 10 | Fonctionnel PIE | Manuel | ❌ Manuel only |
| FX-01-05 | Sons audibles + VFX visibles | Audio/Visuel PIE | Manuel | ❌ Manuel only |

**Justification manuel-only :** MCP TCP est bloqué pendant PIE Play. Aucun framework de test automatisé compatible avec la configuration actuelle (UE5.7, C++ pur, pas de Automation Tests écrits pour les widgets). La validation est exclusivement visuelle/auditive par le développeur.

### Sampling Rate
- **Par tâche :** Lancer PIE et vérifier visuellement
- **Avant validation finale :** Parcours complet du bout en bout sans interruption

### Wave 0 Gaps
Aucun fichier de test à créer — cette phase est 100% éditeur visuel.

---

## Open Questions

1. **WBP_HUD existe-t-il déjà ?**
   - Ce qu'on sait : Les Blueprints listés dans STATE.md ne mentionnent pas `WBP_HUD`. Seuls WBP_TitleScreen, WBP_Loadout, WBP_GameOver, WBP_Leaderboard sont listés.
   - Ce qui est flou : Le PlayerController a `HUDWidgetClass` (TSubclassOf<URevenantOpsHUD>) — si non assigné, le HUD en jeu n'existe pas.
   - Recommandation : Créer `WBP_HUD` dans `/Game/Mercenaires/UI/` avec parent `URevenantOpsHUD` comme première action de cette phase.

2. **StarterContent est-il présent dans le projet ?**
   - Ce qu'on sait : La phase 08 le mentionne comme option pour les sons, mais aucune confirmation qu'il a été importé.
   - Ce qui est flou : Si absent, les placeholders audio doivent venir d'une autre source (Engine/EditorSounds, création de SoundWave vide).
   - Recommandation : Vérifier dans Content Browser au début. Si absent, utiliser `/Engine/EditorSounds/` ou créer des SoundWave assets vides.

3. **Le PlayerController compilé expose-t-il bien ses UPROPERTY ?**
   - Ce qu'on sait : La phase 09 a documenté l'échec MCP pour assigner les widget classes. Le fix est de recompiler le BP dans l'éditeur.
   - Ce qui est flou : Est-ce que le BP a été recompilé depuis ce problème ?
   - Recommandation : Ouvrir BP_ThirdPersonPlayerController → Compile → vérifier catégorie "UI|Mercenaires" dans Details.

4. **L'Event Graph de TitleScreenWidget doit-il implémenter BP_OnPlayPressed ?**
   - Ce qu'on sait : `OnPlayClicked()` appelle `BP_OnPlayPressed()`. Le PlayerController lie `ShowTitleScreen()` mais le code exact de `RevenantOpsPlayerController.cpp` n'a pas été lu pour cette phase.
   - Recommandation : Lire `RevenantOpsPlayerController.cpp` pour vérifier si le flow Title→Loadout est géré par delegate ou par l'Event Graph du widget.

---

## Sources

### Primary (HIGH confidence)
- Lecture directe de `RevenantOpsPlayerController.h` — UPROPERTY EditAnywhere confirmées, catégories vérifiées
- Lecture directe de `TitleScreenWidget.h`, `GameOverWidget.h`, `LeaderboardWidget.h`, `LoadoutWidget.h` — tous les noms BindWidgetOptional documentés
- Lecture directe de `EnemyWaveSpawner.h` — interface SpawnPoints[], bInfiniteMode, bTriggerOnOverlap
- Lecture directe de `MercenairesGameState.h` — BackgroundMusic UPROPERTY confirmé
- Lecture directe de `RevenantOpsHUD.h` — tous les BindWidgetOptional nommés
- `09-01-SUMMARY.md` — problème BlueprintGeneratedClass documenté + fix connu
- `04-01-SUMMARY.md` — éléments arène manquants documentés
- `06-01-SUMMARY.md` — layout widgets requis documentés
- `08-01-SUMMARY.md` — UPROPERTY audio confirmées + StarterContent comme option

### Secondary (MEDIUM confidence)
- `STATE.md` — liste complète des Blueprints créés vs manquants
- README unreal-mcp — capacités MCP (Actor Manipulation, Blueprint Graph, set_property)

### Tertiary (LOW confidence)
- Disponibilité StarterContent dans ce projet — non vérifiée directement (nécessite ouverture Content Browser)
- Méthode Python `NavigationSystemEditorSubsystem.build()` — disponibilité en UE 5.7 non vérifiée via docs officielles

---

## Metadata

**Confidence breakdown :**
- État du projet (C++ compilé, Blueprints existants) : HIGH — lu directement dans les fichiers source
- Noms BindWidgetOptional requis : HIGH — lus directement dans les .h
- Fix BlueprintGeneratedClass : HIGH — documenté dans 09-01-SUMMARY avec cause et solution
- Disponibilité StarterContent : MEDIUM — mentionné dans 08-01-SUMMARY mais non confirmé
- API Python NavMesh build : LOW — syntaxe probable mais non vérifiée sur UE 5.7

**Research date :** 2026-03-23
**Valid until :** Indéfini — basé sur l'état actuel du projet, pas sur des libs externes
