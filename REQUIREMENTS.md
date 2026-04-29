# REQUIREMENTS.md — RevenantOps v4.0

> Specs fonctionnelles par domaine. Chaque exigence est **testable** et a un état actuel.
> Légende : ✅ Implémenté | 🚧 Partiel | ❌ Manquant | 🔄 À refondre

---

## 1. UI / MENUS

### 1.1 Identité visuelle (refonte sprint 1)

| ID | Exigence | État |
|---|---|---|
| UI-001 | Une seule classe `UUITheme` (UDataAsset) centralise toute la palette | ❌ |
| UI-002 | Aucun widget ne hardcode de `FLinearColor` — tout vient de `UUITheme` | 🔄 chaque widget réinvente sa palette |
| UI-003 | Fonts custom (Stencil titres + Roboto Condensed body + Mono HUD) | ❌ utilise font système UE5 |
| UI-004 | Vignette permanente subtle sur tous les menus | ❌ |
| UI-005 | Bordures stencil "metal plating" sur les panneaux | ❌ |
| UI-006 | Tous les widgets menus héritent `UMenuWidgetBase` | 🚧 sauf `MainMenuWidget` (à supprimer), `RevenantOpsHUD`, `LeaderboardWidget` |

### 1.2 Animations & transitions (sprint 1 sem 3)

| ID | Exigence | État |
|---|---|---|
| UI-010 | Fade-in entrée écran (300ms) | ❌ |
| UI-011 | Slide latéral entre écrans (Title→Level→Char→Loadout) | ❌ transition cut |
| UI-012 | Boutons hover : pulse + scale léger | ❌ |
| UI-013 | Splash → TitleScreen : fade cinématique 1.5s | 🚧 fade existe mais brusque |

### 1.3 Sons UI (déjà OK, vérification sprint 1)

| ID | Exigence | État |
|---|---|---|
| UI-020 | Son hover sur tous les boutons de tous les menus | ✅ via `UMenuWidgetBase` |
| UI-021 | Son click sur tous les boutons de tous les menus | ✅ via `UMenuWidgetBase` |
| UI-022 | Sons whoosh + impact aux transitions d'écran | ❌ |

### 1.4 HUD in-game (sprint 1 sem 4)

| ID | Exigence | État |
|---|---|---|
| UI-030 | Bordures stencil sur les panels HUD | ❌ |
| UI-031 | Font Mono pour ammo / timer / score | ❌ |
| UI-032 | Crosshair dynamique (4 traits, gap selon spread + ADS) | ✅ |
| UI-033 | Hit marker au toucher | ✅ |
| UI-034 | Damage direction indicator | ✅ |
| UI-035 | Kill notification popup | ✅ |
| UI-036 | Reload bar | ✅ |
| UI-037 | Pickup prompt RE5 (icône + [E] + nom) | ✅ |
| UI-038 | Finisher prompt ([F] Corps-à-corps) | ✅ |
| UI-039 | Vignette low health (pulse rouge) | ✅ |

### 1.5 Écrans à refondre (sprint 1 sem 2-4)

| ID | Écran | Évolution |
|---|---|---|
| UI-040 | TitleScreen | Vignette + bordures stencil + logo placeholder + version build affichée |
| UI-041 | LevelSelect | Cards style "dossier mission" (frame métallique, étiquette laiton) |
| UI-042 | CharacterSelect | Fiche personnel militaire (photo, nom, grade, talents radar) |
| UI-043 | Loadout | Layout "armurerie" — armes posées sur établi, stats bars |
| UI-044 | Options | Fiche briefing — sections Audio/Vidéo/Contrôles |
| UI-045 | GameOver | Tier visuel S/A/B/C/D, fond ambiance, fade dramatique |
| UI-046 | Inventory | Frame "caisse militaire", slots avec étiquettes laiton |

---

## 2. AUDIO

### 2.1 Architecture (sprint 2 sem 5)

| ID | Exigence | État |
|---|---|---|
| AUD-001 | `USoundClass` Master parent | ❌ |
| AUD-002 | `USoundClass` Music (enfant Master) | ❌ |
| AUD-003 | `USoundClass` SFX_Weapons (enfant Master) | ❌ |
| AUD-004 | `USoundClass` SFX_Enemies (enfant Master) | ❌ |
| AUD-005 | `USoundClass` SFX_UI (enfant Master) | ❌ |
| AUD-006 | `USoundClass` Ambience (enfant Master) | ❌ |
| AUD-007 | `USoundClass` Footsteps (enfant Master) | ❌ |
| AUD-008 | `USoundMix` "Combat" : duck Music -6dB, boost SFX_Weapons +2dB | ❌ |

### 2.2 Sons existants à brancher (sprint 2 sem 5-6)

| ID | Sound | État | Action |
|---|---|---|---|
| AUD-010 | `WeaponBase::FireSound` | ✅ joué | OK |
| AUD-011 | `WeaponBase::ReloadSound` | ✅ joué | OK |
| AUD-012 | `WeaponBase::EmptySound` | ✅ joué | OK |
| AUD-013 | `EnemyBase::HitSound` | 🚧 déclaré, **jamais joué** | brancher dans `HandleDamage` |
| AUD-014 | `EnemyBase::DeathSound` | 🚧 déclaré, **jamais joué** | brancher dans `HandleDeath` |
| AUD-015 | `EnemyBase::AmbientSound` | 🚧 déclaré, **jamais joué** | brancher via timer dans `Tick` |
| AUD-016 | Musique menu (`MenuMusic`) | ✅ joué | OK |

### 2.3 Sons à créer/intégrer (sprint 2)

| ID | Sound | Source |
|---|---|---|
| AUD-020 | Footsteps perso (béton, métal, gravier — 3 surfaces × 3 variantes) | Nourredine sprint 1 sem 2 |
| AUD-021 | Footsteps ennemis | Nourredine + variation pitch |
| AUD-022 | Ambiance level Blacksite (loop 60s) | Nourredine sprint 1 sem 3 |
| AUD-023 | Sons pickup (santé, ammo, arme — 3 sons distincts) | freesound.org CC0 |
| AUD-024 | Sons UI whoosh transition + impact panel | freesound.org CC0 |
| AUD-025 | Musique combat dynamique (intro 8s + loop 60s + outro 6s) | Nourredine sprint 1 sem 4 (split OST_1) |

### 2.4 Musique combat dynamique (sprint 2 sem 7)

| ID | Exigence | État |
|---|---|---|
| AUD-030 | Intro joué quand `MercenairesGameState::StartMatch` | ❌ |
| AUD-031 | Loop continu pendant le match | ❌ |
| AUD-032 | Outro fade-out 3s sur `EndMatch` | ❌ |
| AUD-033 | Crossfade 2s vers musique calme entre les vagues | ❌ (nice-to-have) |

### 2.5 Hooks code

| ID | Fichier | Méthode |
|---|---|---|
| AUD-040 | `IPickupInterface` | Ajouter `PlayPickupSound()` virtual |
| AUD-041 | `RevenantOpsCharacter` | Ajouter `AnimNotify_Footstep` UFUNCTION |
| AUD-042 | `MercenairesGameState` | Champ `USoundBase* CombatMusicIntro/Loop/Outro` |

---

## 3. ANIMATIONS

### 3.1 Animations joueur (montages)

| ID | Animation | État C++ | État Asset |
|---|---|---|---|
| ANIM-P-001 | `FireMontage` (Pistol/Rifle) | ✅ branché | ✅ existe |
| ANIM-P-002 | `ReloadMontage` (Pistol/Rifle) | ✅ branché | ✅ existe |
| ANIM-P-003 | `EquipMontage` (Pistol/Rifle) | ✅ branché | ✅ existe |
| ANIM-P-010 | Sprint loop | ❌ pas de var/montage | ❌ Nourredine sprint 2 sem 7 |
| ANIM-P-011 | Crouch idle + walk | ❌ | ❌ Nourredine sprint 2 sem 6 |
| ANIM-P-012 | Slide | ❌ | ❌ Nourredine sprint 2 sem 8 |
| ANIM-P-013 | Dodge / Roll | ❌ | ❌ Nourredine sprint 3 si rythme OK |
| ANIM-P-014 | Jump | ❌ | ❌ Nourredine sprint 3 |
| ANIM-P-015 | Pickup (se baisser, prendre, relever) | ❌ | ❌ Nourredine sprint 2 sem 5 |
| ANIM-P-016 | Player Death | ❌ | ❌ Nourredine sprint 2 sem 5 |
| ANIM-P-017 | Finisher Head/Leg/Torso | ❌ déclarés `RevenantOpsCharacter::FinisherMontageX` | ❌ Nourredine sprint 3 |

### 3.2 Animations ennemis

| ID | Animation | État C++ | État Asset |
|---|---|---|---|
| ANIM-E-001 | `HitReactAnim` | 🚧 déclaré dans `EnemyBase`, **jamais joué** | ❌ |
| ANIM-E-002 | `DeathAnim` | 🚧 déclaré, **jamais joué** | ❌ |
| ANIM-E-003 | `AttackAnim` | 🚧 déclaré, **jamais joué** | ❌ |
| ANIM-E-010 | `StaggerMontageHead` | ✅ branché dans `TakeHitZoneDamage` | ❌ Nourredine sprint 3 |
| ANIM-E-011 | `StaggerMontageLeg` | ✅ branché | ❌ Nourredine sprint 3 |
| ANIM-E-012 | `StaggerMontageTorso` | ✅ branché | ❌ Nourredine sprint 3 |
| ANIM-E-020 | Death × 3 directions | ❌ | ❌ Nourredine sprint 3 |
| ANIM-E-021 | Attack mêlée bras (Slow) | ❌ | ❌ Nourredine sprint 3 |
| ANIM-E-022 | Attack morsure (Runner) | ❌ | ❌ Nourredine sprint 3 |
| ANIM-E-023 | Attack charge (Tank) | ❌ | ❌ Nourredine sprint 3 |

### 3.3 ABP_Mercenaire

| ID | Exigence | État |
|---|---|---|
| ANIM-ABP-001 | Transition Locomotion ↔ Armed (selon `bIsArmed`) | ✅ |
| ANIM-ABP-002 | BlendSpace `BS_IdleRun` (locomotion) | ✅ |
| ANIM-ABP-003 | Sprint state (lecture sprint loop) | ❌ |
| ANIM-ABP-004 | Crouch state (Idle + Walk) | ❌ |
| ANIM-ABP-005 | Slide state (one-shot) | ❌ |
| ANIM-ABP-006 | Death state (depuis n'importe quel state si dead) | ❌ |

---

## 4. GAME DESIGN

### 4.1 Boucle de gameplay (existant)

| ID | Système | État |
|---|---|---|
| GD-001 | Title screen → Loadout (2 armes) → Match | ✅ |
| GD-002 | Match : timer 5min countdown | ✅ |
| GD-003 | Vagues d'ennemis (3 vagues hardcoded) | ✅ |
| GD-004 | Score + combo (window 5s, x1→x10) | ✅ |
| GD-005 | Pickups (santé, munitions, bonus temps) | ✅ |
| GD-006 | Inventaire RE5 3x3 (Tab) | ✅ |
| GD-007 | Talents (UTalentDefinition × 6) | ✅ |
| GD-008 | Dégâts localisés + finishers | ✅ |
| GD-009 | GameOver → Leaderboard local | ✅ |

### 4.2 Progression v4.0 (sprint 3 sem 9)

| ID | Exigence | État |
|---|---|---|
| GD-010 | `UGameProgressSaveGame` (USaveGame) avec score cumulé | ❌ |
| GD-011 | Unlock characters par paliers (5 perso, paliers 0/1k/5k/15k/30k) | ❌ |
| GD-012 | Unlock talents par paliers | ❌ |
| GD-013 | Unlock weapons par paliers | ❌ |
| GD-014 | Stats persistantes (best score, total kills, time played, runs) | ❌ |

### 4.3 Difficulty curve (sprint 3 sem 9)

| ID | Exigence | État |
|---|---|---|
| GD-020 | Vagues adaptatives selon temps écoulé (formule) | ❌ |
| GD-021 | `MaxAliveEnemies` croissant : 8 → 12 sur 5min | ❌ |
| GD-022 | Spawn delay décroissant : 3s → 1s sur 5min | ❌ |
| GD-023 | Mix de types ennemis évolue (Slow start, Tank/Runner mid, mix end) | ❌ |

### 4.4 Tier visuel & achievements (sprint 3 sem 10)

| ID | Exigence | État |
|---|---|---|
| GD-030 | Rank S/A/B/C/D affiché en GameOver selon score+combo+temps | ❌ |
| GD-031 | XP bonus par rang (×3 si S, ×2 si A, ×1 si B, ×0.5 si C/D) | ❌ |
| GD-032 | 8 achievements de base (100 kills, Combo x10, Speedrun, etc.) | ❌ |
| GD-033 | Notification popup achievement débloqué | ❌ |

### 4.5 Comportements ennemis distincts (sprint 3 sem 10-11)

| ID | Type ennemi | Behavior actuel | Behavior cible |
|---|---|---|---|
| GD-040 | Slow | Fonce vers joueur | Avance en groupe, ralentit en ligne droite, solidarité |
| GD-041 | Runner | Fonce vers joueur | Zigzag, vitesse +30% en line of sight, esquive |
| GD-042 | Tank | Fonce vers joueur | Charge directe, encaisse, ignore stagger torse |
| GD-043 | Spitter | Tire projectile | Recule à <8m, tire à distance, fuit en mêlée |
| GD-044 | Exploder | Self-destruct close range | Warning visuel rouge 1s avant explosion |

### 4.6 Tutoriel (sprint 3 sem 11)

| ID | Hint | Trigger |
|---|---|---|
| GD-050 | "Tab : Inventaire" | Apparaît au 1er match, première seconde |
| GD-051 | "F : Corps-à-corps" | Apparaît au 1er stagger ennemi |
| GD-052 | "E : Interagir" | Apparaît au 1er overlap pickup |
| GD-053 | "Shift : Sprint" | Apparaît si joueur n'a pas sprinté en 30s du 1er match |
| GD-054 | Hints désactivables (Options menu) | — |

---

## 5. CONTRAINTES TECHNIQUES (rappel)

- **UE 5.7** — pas de deprecated API
- **MSVC VS 2026** — build via Live Coding (Ctrl+Alt+F11) ou Build complet
- **Pas de packaging shipping** — Editor Development uniquement
- **Pas de mocks** — tests en conditions réelles PIE
- **Pas d'Open Level via MCP** — crash UE5
- **Pas de MCP pendant PIE** — TCP bloqué
- **Workflow autonome C++** : code → compile → test PIE → fix → recompile (boucle)
