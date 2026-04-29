# ROADMAP.md — RevenantOps v4.0

> 12 semaines de sprint GSD, du **2026-04-28** au **2026-07-21**.
> Chaque tâche est < 1 jour. Cocher `[x]` quand fait. Ajouter date de complétion `(YYYY-MM-DD)`.
> Mise à jour à chaque session début + fin (règle CLAUDE.md).
>
> **Note** : l'ancien ROADMAP théorique (phases 1-X) est archivé dans l'historique git. Cette version GSD remplace.

---

## SPRINT 1 — REFONTE IDENTITÉ MENUS (sem 1-4)

**Objectif** : éliminer le look "basique UE5", établir l'identité RE Grit, nettoyer le code mort.
**Démo livrable** : vidéo 60s "menu tour" (Splash → Title → Level → Char → Loadout → Match → GameOver).

### Semaine 1 (28 avril - 4 mai) — Fondations theme

**Jilani** :
- [ ] J1.1 Créer `UUITheme` UDataAsset (`Source/RevenantOps/UI/UITheme.h/.cpp`) avec palette validée
- [ ] J1.2 Créer `UIHelpers.h/.cpp` (helpers `MakePanelBrush`, `MakeButtonStyle`, `MakeStencilBorder`)
- [ ] J1.3 Supprimer `MainMenuWidget.h/.cpp` + toutes les refs (audit confirmé doublon mort)
- [ ] J1.4 Refacto `TitleScreenWidget` pour consommer `UUITheme` au lieu des couleurs hardcodées
- [ ] J1.5 Importer fonts Stencil + Roboto Condensed + Mono dans `Content/Mercenaires/UI/Fonts/`
- [ ] J1.6 Créer DA_Theme_Default (instance `UUITheme`) et l'assigner globalement
- [ ] J1.7 Update `STATE.md` fin semaine

**Nourredine** :
- [ ] N1.1 Setup Blender 4.x + suivre tuto Blender Guru "Beginner Donut" (4h)
- [ ] N1.2 Installer Audacity + tuto "Normalize and Export"
- [ ] N1.3 Normaliser tous les SFX existants (`SW_Fire_*`, `SW_Reload_*`) à -3dB peak
- [ ] N1.4 Livrer pack `Audio_Normalized_v1.zip` dans Drive `incoming/`

**Validation fin semaine** : `git grep MainMenuWidget` retourne 0 résultats. TitleScreen affiche fonts custom.

---

### Semaine 2 (5 - 11 mai) — Refonte écrans principaux

**Jilani** :
- [ ] J2.1 Refonte `LevelSelectWidget` — cards style "dossier mission" (frame métallique)
- [ ] J2.2 Refonte `CharacterSelectWidget` — fiche personnel militaire (portrait + grade)
- [ ] J2.3 Refonte `LoadoutWidget` — layout "armurerie" (armes sur établi, stats bars)
- [ ] J2.4 Vérifier que tous les widgets héritent `UMenuWidgetBase` (sauf HUD)
- [ ] J2.5 Tests PIE : naviguer Title→Level→Char→Loadout sans bug visuel

**Nourredine** :
- [ ] N2.1 Modéliser caisse munitions (low poly, refs photos fournies par Jilani)
- [ ] N2.2 Modéliser barrel rouillé
- [ ] N2.3 Modéliser panneau "RESTRICTED" (panneau métal + texte)
- [ ] N2.4 Modéliser lampe cassée (suspendue, détails visibles)
- [ ] N2.5 Modéliser douille (cartouche au sol)
- [ ] N2.6 Export FBX → Drive `incoming/Props_v1/`
- [ ] N2.7 Trouver 10 sons CC0 footsteps (béton, métal, gravier) sur freesound.org

**Validation** : démo statique screenshots des 4 écrans refondus partagée Discord.

---

### Semaine 3 (12 - 18 mai) — Animations & transitions

**Jilani** :
- [ ] J3.1 Créer helper `PlayFadeInTransition` (UWidgetAnimation programmatique 300ms)
- [ ] J3.2 Brancher fade-in dans `NativeOnInitialized` de chaque widget menu
- [ ] J3.3 Helper `PlaySlideTransition` (slide latéral entre écrans)
- [ ] J3.4 Boutons hover : pulse rouge + scale 1.05 au survol
- [ ] J3.5 Refondre splash → TitleScreen : fade cinématique 1.5s avec audio whoosh
- [ ] J3.6 Importer sons UI whoosh + impact (livrés par Nourredine sem 2 ou freesound CC0)

**Nourredine** :
- [ ] N3.1 Tuto Mixamo : importer mannequin UE5 dans Blender (workflow YouTube fourni)
- [ ] N3.2 Comprendre weight painting (pas d'animation à produire encore)
- [ ] N3.3 Screenshot hierarchy bones validée → Discord
- [ ] N3.4 Créer ambiance level Blacksite (vent + bourdon élec + radio statique) loop 60s Audacity
- [ ] N3.5 Livrer `Ambience_Blacksite_Loop.wav`

**Validation** : enregistrement transition Splash→Title→Level visible et fluide.

---

### Semaine 4 (19 - 25 mai) — HUD polish + Options + GameOver

**Jilani** :
- [ ] J4.1 HUD : bordures stencil sur Health/Stamina/Ammo panels
- [ ] J4.2 HUD : font Mono pour ammo/timer/score (utilise `UUITheme::FontMono`)
- [ ] J4.3 OptionsWidget : refonte layout fiche briefing (sections)
- [ ] J4.4 GameOverWidget : tier visuel S/A/B/C/D placeholder (logique tier en sprint 3)
- [ ] J4.5 InventoryWidget : frame "caisse militaire" + étiquettes laiton
- [ ] J4.6 Enregistrer démo vidéo 60s "menu tour" — livrable Sprint 1

**Nourredine** :
- [ ] N4.1 Modéliser silencer (attachment arme)
- [ ] N4.2 Modéliser red dot sight
- [ ] N4.3 Modéliser tactical light
- [ ] N4.4 Export FBX → Drive `incoming/WeaponAttachments_v1/`
- [ ] N4.5 Audio : split `OST_1` en 3 phases (intro 8s / loop 60s / outro 6s)
- [ ] N4.6 Livrer `Music_Combat_Intro/Loop/Outro.wav`

**Validation Sprint 1** :
- [ ] Démo vidéo 60s validée par les 2 devs sur Discord
- [ ] Aucun usage `MainMenuWidget` (grep clean)
- [ ] Tous les widgets utilisent `UUITheme`
- [ ] Update `CLAUDE.md` + `primer.md` + `STATE.md`
- [ ] Commit + merge sur `main` + tag `sprint-1-done`

---

## SPRINT 2 — SOUND DESIGN + ANIMATIONS (sem 5-8)

**Objectif** : tous les sons audibles, footsteps fonctionnels, musique combat dynamique, anims joueur de base.
**Démo livrable** : vidéo 30s gameplay avec audio audible (sprint, footsteps, hit sounds, musique combat).

### Semaine 5 (26 mai - 1 juin) — Architecture audio

**Jilani** :
- [ ] J5.1 Créer SoundClass tree (Master, Music, SFX_Weapons, SFX_Enemies, SFX_UI, Ambience, Footsteps) dans `Content/Mercenaires/Audio/Classes/`
- [ ] J5.2 Créer SoundMix "Combat" (duck Music -6dB, boost SFX_Weapons +2dB)
- [ ] J5.3 Assigner SoundClass à chaque USoundBase existant (manuel via éditeur)
- [ ] J5.4 Brancher `EnemyBase::HitSound` dans `HandleDamage` (PlaySoundAtLocation)
- [ ] J5.5 Brancher `EnemyBase::DeathSound` dans `HandleDeath`
- [ ] J5.6 Brancher `EnemyBase::AmbientSound` via timer dans `Tick` (avec `AmbientSoundInterval`)

**Nourredine** :
- [ ] N5.1 Animation Pickup (se baisser, prendre, relever) — 1.5s ★☆☆
- [ ] N5.2 Animation Player Death (chute en avant) — 2s ★☆☆
- [ ] N5.3 Tuto fourni "FBX export Mannequin UE5"
- [ ] N5.4 Livrer FBX dans `incoming/Anims_Player_v1/`

**Validation** : tirer sur ennemi → entendre HitSound. Tuer ennemi → DeathSound.

---

### Semaine 6 (2 - 8 juin) — Footsteps + crouch

**Jilani** :
- [ ] J6.1 Créer `Source/RevenantOps/Audio/FootstepNotify.h/.cpp` (UAnimNotify)
- [ ] J6.2 Ajouter notifies sur `BS_IdleRun` aux frames de pied (left/right)
- [ ] J6.3 `RevenantOpsCharacter` : `OnFootstep()` joue un son aléatoire selon surface (line trace ECC_Visibility)
- [ ] J6.4 Importer footsteps Nourredine sem 2 (béton/métal/gravier)
- [ ] J6.5 Importer animations Pickup + Death de Nourredine sem 5 → créer montages
- [ ] J6.6 Brancher `FinisherMontagePickup` (déclaration sur `RevenantOpsCharacter`)
- [ ] J6.7 Tester PIE : sprinter → footsteps audibles

**Nourredine** :
- [ ] N6.1 Animation Crouch Idle loop — 2s ★★☆
- [ ] N6.2 Animation Crouch Walk loop — 1s ★★☆
- [ ] N6.3 Refs RE5 fournies par Jilani
- [ ] N6.4 Livrer FBX → Discord pour review

**Validation** : footsteps audibles avec variation surface. Animation pickup visible PIE.

---

### Semaine 7 (9 - 15 juin) — Sprint + musique combat

**Jilani** :
- [ ] J7.1 Importer animations Crouch Nourredine sem 6 → ABP_Mercenaire state Crouch
- [ ] J7.2 Logique C++ : `ARevenantOpsCharacter::CrouchPressed` joue le bon montage
- [ ] J7.3 `MercenairesGameState` : ajouter champs `CombatMusicIntro/Loop/Outro`
- [ ] J7.4 `StartMatch` : play Intro → schedule Loop après 8s → maintenir
- [ ] J7.5 `EndMatch` : crossfade vers Outro 3s
- [ ] J7.6 Importer Music split de Nourredine sem 4
- [ ] J7.7 Tester PIE : démarrer match → entendre intro+loop ; finir → outro

**Nourredine** :
- [ ] N7.1 Animation Sprint loop — 1s ★★☆
- [ ] N7.2 Refs cycle marche rapide fournies par Jilani
- [ ] N7.3 Livrer FBX

**Validation** : musique combat audible démarrage match. Crouch fonctionnel en PIE.

---

### Semaine 8 (16 - 22 juin) — Slide + intégration

**Jilani** :
- [ ] J8.1 Importer Sprint loop Nourredine sem 7 → ABP state Sprint
- [ ] J8.2 Logique : sprint condition `bIsSprinting && Velocity > Threshold`
- [ ] J8.3 Importer Slide Nourredine sem 8 (livré début sem) → montage one-shot
- [ ] J8.4 `RevenantOpsCharacter::OnSlideStart` joue le montage
- [ ] J8.5 Pickup interface : ajouter `IPickupInterface::PlayPickupSound()` virtual
- [ ] J8.6 Importer 3 sons pickup (santé/ammo/arme) freesound
- [ ] J8.7 Enregistrer démo Sprint 2 (30s gameplay audio audible)

**Nourredine** :
- [ ] N8.1 Animation Slide (glissement avant 1s) — ★★★
- [ ] N8.2 Accompagnement Discord avec Jilani 1h
- [ ] N8.3 Livrer FBX

**Validation Sprint 2** :
- [ ] Démo 30s validée par les 2 devs
- [ ] Tous les sons EnemyBase audibles
- [ ] Footsteps + sprint + crouch + slide fonctionnels
- [ ] Musique combat dynamique en PIE
- [ ] Tag `sprint-2-done`

---

## SPRINT 3 — GAME DESIGN AVANCÉ (sem 9-12)

**Objectif** : progression/unlock, difficulty curve, achievements, comportements ennemis distincts, tutoriel, animations ennemis.
**Démo livrable** : run 5min montrant progression + tier S + achievement popup + comportements ennemis variés.

### Semaine 9 (23 - 29 juin) — Progression + difficulty curve

**Jilani** :
- [ ] J9.1 Créer `UGameProgressSaveGame` (USaveGame) avec `TotalScoreCumul`, `TotalKills`, `TotalRuns`, `UnlockedCharacters`, `UnlockedTalents`, `UnlockedWeapons`, `Achievements`
- [ ] J9.2 `MercenairesGameState::EndMatch` save progress (append score, increment runs)
- [ ] J9.3 Logique unlock : si `TotalScoreCumul >= Tier`, mark unlocked
- [ ] J9.4 `CharacterSelectWidget` lit unlocked status, grise les non-unlocked
- [ ] J9.5 Difficulty curve : `EnemyWaveSpawner::ComputeWaveParams(elapsed, score)` retourne `EnemyCount/SpawnDelay/MaxAlive`
- [ ] J9.6 Voir `difficulty-curve.md` pour formule

**Validation** : 3 matchs successifs montrent unlock progressif. T>3min spawn rate visiblement plus rapide.

---

### Semaine 10 (30 juin - 6 juillet) — Tier visuel + achievements

**Jilani** :
- [ ] J10.1 `UGameOverWidget::ComputeTier(score, combo, time)` retourne S/A/B/C/D
- [ ] J10.2 Affichage tier visuel (lettre + couleur + son)
- [ ] J10.3 Achievement system : 8 achievements de base définis dans DA
- [ ] J10.4 `UAchievementWidget` (popup notification) — fade in/hold/out
- [ ] J10.5 Hooks dans gameplay : OnEnemyDied (compte kills), OnComboReached (combo x10), OnMatchEnd (speedrun if time<3min)
- [ ] J10.6 Tester achievements en PIE

**Nourredine** :
- [ ] N10.1 Animation Stagger Head (zombie se tient la tête) — ★★☆
- [ ] N10.2 Animation Stagger Leg (zombie tombe à genoux) — ★★☆
- [ ] N10.3 Animation Stagger Torso (zombie recule) — ★★☆

**Validation** : tier S/A/B/C/D visible en GameOver. Popup achievement Combo x10 visible.

---

### Semaine 11 (7 - 13 juillet) — Comportements ennemis distincts + tutoriel

**Jilani** :
- [ ] J11.1 `EnemyBase::UpdateCombat` lit `BehaviorProfile` et adapte mouvement/tir
- [ ] J11.2 Spitter : recule à <8m, tire à distance, fuit en mêlée
- [ ] J11.3 Exploder : warning visuel rouge MID 1s avant explosion
- [ ] J11.4 Tank : ignore stagger torso (set `bImmuneToTorsoStagger = true`)
- [ ] J11.5 Runner : zigzag (random offset target location), vitesse +30% en LOS
- [ ] J11.6 Slow : avance en groupe (cohésion via overlap query rayon 500cm)
- [ ] J11.7 `UTutorialHintWidget` (popup contextuel)
- [ ] J11.8 Hooks tutoriel : 1er match (Tab hint), 1er stagger (F hint), 1er pickup (E hint), 30s sans sprint (Shift hint)
- [ ] J11.9 Option "Désactiver tutoriel" dans Options

**Nourredine** :
- [ ] N11.1 Animation Death zombie variation 1 (chute en avant)
- [ ] N11.2 Animation Death zombie variation 2 (chute sur le côté)
- [ ] N11.3 Animation Death zombie variation 3 (chute en arrière)

**Validation** : Spitter recule visiblement. Exploder warning visible. Tutoriel hints affichés au 1er match.

---

### Semaine 12 (14 - 21 juillet) — Polish final + démo + release v4.0

**Jilani** :
- [ ] J12.1 Importer animations Death + Stagger Nourredine sem 10-11
- [ ] J12.2 Brancher `EnemyBase::DeathAnim` dans `HandleDeath` (sélection random parmi 3)
- [ ] J12.3 Pass de bug fixing complet (parcourir issues GitHub)
- [ ] J12.4 Trailer 90s (footage PIE + montage Premiere/DaVinci)
- [ ] J12.5 Update README.md avec screenshots gameplay
- [ ] J12.6 Tag release `v4.0`
- [ ] J12.7 Update `STATE.md` final v4.0

**Nourredine** :
- [ ] N12.1 Animation Attack mêlée bras (Slow zombie)
- [ ] N12.2 Animation Attack morsure (Runner)
- [ ] N12.3 Animation Attack charge (Tank)

**Validation Sprint 3 / v4.0** :
- [ ] Trailer 90s validé
- [ ] Démo run complet 5min montrant tous les nouveaux systèmes
- [ ] Build clean, pas de warning critique
- [ ] Tag `v4.0` poussé
- [ ] PROJECT.md / REQUIREMENTS.md / STATE.md à jour

---

## BACKLOG (idées émergentes — à traiter post-v4.0)

> Cette section recueille les idées qui apparaissent en cours de route, à traiter dans une v4.1+ pour ne pas polluer le sprint courant.

- [ ] Sons météo / ambiance dynamique selon zone Blacksite
- [ ] Système de daily challenges
- [ ] Mode Hard/Nightmare avec ennemis +50% HP +30% vitesse
- [ ] Skin de personnage débloquables (cosmétiques)
- [ ] Replay system (rejouer son meilleur run)
- [ ] Steam achievements integration
- [ ] Multijoueur coop 2 joueurs (énorme scope, v5.0?)
- [ ] Editeur de niveau intégré
- [ ] Mod support (DataTables externes)
- [ ] Système de couverture (cover system)
- [ ] Mantle/Vault automatique
- [ ] Manette : support complet Xbox/PS/Steam Deck
- [ ] Localization (en/fr/es/de)
- [ ] Accessibility options (colorblind, subtitles, font size)

---

## Indicateurs de progression

**Mise à jour à la fin de chaque semaine** dans `STATE.md` :
- Tâches complétées (X/total) cette semaine
- Bloqueurs identifiés
- Démo livrée ? (oui/non)
- Risques émergents

**Définition de "Done"** :
- Code compile sans warning critique
- Testé en PIE
- Mergé sur `main` (après PR review par l'autre dev)
- Documentation à jour (`STATE.md` + `CLAUDE.md` si changement architectural)
