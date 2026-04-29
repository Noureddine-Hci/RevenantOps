# Menu Redesign Mockups — RevenantOps v4.0

> Wireframes ASCII de chaque écran refondu. Specs visuelles pour le sprint 1.
> Conventions : `█` = bg sombre, `╔══╗` = bordure stencil, `[BUTTON]` = bouton interactif.

---

## 1. SplashScreen

**Existant** : 6/10 — fonctionnel mais brusque. Améliorations :
- Vignette permanente
- Audio whoosh entre splashes
- Fade plus dramatique (1.5s au lieu de cut)

**Mockup** (déjà fonctionnel, juste polish) :
```
┌────────────────────────────────────────────────────┐
│                                                    │
│                                                    │
│                                                    │
│                  [LOGO IMAGE]                      │
│                  400×400 px                        │
│                                                    │
│                                                    │
│              UN JEU REVENANT OPS                   │
│                  (sub-text)                        │
│                                                    │
│                                                    │
└────────────────────────────────────────────────────┘
   Background : noir plein
   Logo fade in 0.6s → hold 2s → fade out 0.5s
   Sub-text : Roboto Condensed 18pt, GoldDim
```

**Modifs sprint 1** :
- Ajout audio `whoosh` à chaque transition entre Logos
- Fade out final → fade in TitleScreen (crossfade au lieu de cut)

---

## 2. TitleScreen (refonte sprint 1 sem 1-2)

**Layout cible** :
```
╔══════════════════════════════════════════════════════════╗
║▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓║  bande rouge top 5px
║                                                          ║
║                                                          ║
║   ╔════════════════════════╗   ┌──────────────────┐     ║
║   ║                        ║   │                  │     ║
║   ║     REVENANTOPS        ║   │   ▶ JOUER        │     ║
║   ║   ─────────────         ║   │                  │     ║
║   ║   MODE MERCENAIRES      ║   │   ▷ OPTIONS      │     ║
║   ║                        ║   │                  │     ║
║   ║   "Combat Zone Forsaken"║   │   ▷ QUITTER      │     ║
║   ║                        ║   │                  │     ║
║   ╚════════════════════════╝   └──────────────────┘     ║
║                                                          ║
║                                                          ║
║   RevenantOps © 2026                  v4.0.0-sprint1     ║
╚══════════════════════════════════════════════════════════╝
```

**Specs** :
- **Bg** : `BgDeep` plein écran + image décorative subtle (Nourredine prop "RESTRICTED" panneau)
- **Bande rouge** : 5px haut, `RedBlood`, `HitTestInvisible`
- **Titre "REVENANTOPS"** : Allerta Stencil 64pt, `GoldTarnish`, dégradé subtle
- **Sub-titre "MODE MERCENAIRES"** : Stencil 18pt, `GoldDim`
- **Tagline italique** : Roboto Condensed Italic 14pt, `GreySoft`
- **Boutons droite** :
  - JOUER : style premium, `RedBlood` bg, hover → `RedAlert`
  - OPTIONS : `BgPanel` bg, hover → `BgPanelHover`
  - QUITTER : transparent, hover → `BgPanelDim`
- **Footer** : Roboto Condensed 11pt, `GreySoft`, version build à droite

**Animations** :
- Fade-in écran (300ms)
- Logo subtle float (translation Y ±5px sur 4s loop)
- Boutons hover : scale 1.05 + pulse rouge (à droite du bouton, ligne accent)

---

## 3. LevelSelect (refonte sprint 1 sem 2)

**Layout cible — style "dossier mission"** :
```
╔══════════════════════════════════════════════════════════╗
║   < RETOUR                            MISSION SELECT     ║
║                                                          ║
║   ┌──────────┐  ┌──────────┐  ┌──────────┐              ║
║   │          │  │ ╔══════╗ │  │          │              ║
║   │ MISSION  │  │ ║      ║ │  │ MISSION  │              ║
║   │   01     │  │ ║ M02  ║ │  │   03     │              ║
║   │          │  │ ║      ║ │  │ [LOCKED] │              ║
║   │ Compound │  │ ╚══════╝ │  │          │              ║
║   │          │  │ Blacksite│  │  ???     │              ║
║   │ ▶ ◀      │  │ ▶ ◀      │  │ ▶ ◀      │              ║
║   └──────────┘  └──────────┘  └──────────┘              ║
║                                                          ║
║   ╔════════════════════════════════════════════════╗    ║
║   ║                LEADERBOARD                     ║    ║
║   ║   ┌──┬──────────────────┬──────────┬────────┐  ║    ║
║   ║   │#1│ JILANI           │  18,450  │ 3:24   │  ║    ║
║   ║   │#2│ NOUREDDINE       │  12,800  │ 4:01   │  ║    ║
║   ║   │#3│ ANONYMOUS        │   8,200  │ 4:55   │  ║    ║
║   ║   └──┴──────────────────┴──────────┴────────┘  ║    ║
║   ╚════════════════════════════════════════════════╝    ║
║                                                          ║
║                              [CONFIRMER MISSION]         ║
╚══════════════════════════════════════════════════════════╝
```

**Specs** :
- **Cards mission** : 200×280px, `BgPanel` bg, bordure `GoldDim` 1px
  - Card sélectionnée : bordure `GoldTarnish` 2px + glow subtle
  - Card locked : grise + stamp "LOCKED" rouge
  - Photo/screenshot mission en haut (Nourredine peut générer en sprint 4)
  - Titre Stencil 22pt
  - Description Roboto 12pt
  - Boutons ◀▶ pour cycler les versions de difficulté (futur)
- **Leaderboard** : panel `BgPanelDim`, bordure stencil
  - Header "LEADERBOARD" Stencil 18pt `GoldTarnish`
  - Lignes Roboto Mono 14pt (alignement perfect)
- **Bouton CONFIRMER** : bottom-right, `RedBlood`, large

**Logique locked** : carte mission grisée si pas unlock dans `UGameProgressSaveGame` (sprint 3). Sprint 1 : tous unlocked en placeholder.

---

## 4. CharacterSelect (refonte sprint 1 sem 2)

**Layout cible — style "fiche personnel militaire"** :
```
╔══════════════════════════════════════════════════════════╗
║   < RETOUR              SÉLECTION OPÉRATEUR              ║
║                                                          ║
║   ┌─────────┐    ╔═══════════════╗   ┌────────────────┐ ║
║   │ ROSTER  │    ║               ║   │ FICHE PERSONN. │ ║
║   │         │    ║   [PORTRAIT   ║   │                │ ║
║   │ □ MARC  │    ║    3D LIVE]   ║   │ NOM    : MARC  │ ║
║   │ ■ JANE  │    ║               ║   │ GRADE  : SGT   │ ║
║   │ □ ALEX  │    ║   512×910     ║   │ UNITÉ  : ALPHA │ ║
║   │ □ KAI   │    ║               ║   │                │ ║
║   │ ▒ DR.X  │    ║               ║   │ TALENTS:       │ ║
║   │ (locked)│    ║               ║   │ ◆ Reload +20%  │ ║
║   │         │    ║               ║   │ ◆ Stamina +15% │ ║
║   │         │    ║               ║   │ ◆ Sprint +10%  │ ║
║   │         │    ║               ║   │                │ ║
║   │         │    ║               ║   │ STATS:         │ ║
║   │         │    ║               ║   │   HP   ████░  │ ║
║   │         │    ║               ║   │   STAM ██████ │ ║
║   │         │    ║               ║   │   SPD  ████░  │ ║
║   └─────────┘    ╚═══════════════╝   └────────────────┘ ║
║                                                          ║
║   ◀ PRÉCÉDENT      [SÉLECTIONNER]      SUIVANT ▶         ║
╚══════════════════════════════════════════════════════════╝
```

**Specs** :
- **Roster** : liste verticale 6 personnages, `BgPanel`
  - Sélectionné : barre `RedBlood` à gauche + bg `BgPanelHover`
  - Locked : grise + icône cadenas
- **Portrait 3D** : `ACharacterPreviewActor` (existant), background subtle vignette
- **Fiche personnel** : `BgPanel` bordure stencil
  - Champs alignés gauche : NOM / GRADE / UNITÉ
  - Section TALENTS : 3 talents avec icône ◆ et descriptif court
  - Section STATS : 3 bars (HP / STAMINA / SPEED) — bars en `GoldDim`
- **Boutons** : PRÉCÉDENT / SUIVANT cyclent. SÉLECTIONNER confirme.

**Existant** : déjà bien (6/10), juste habillage RE Grit + fonts custom + bordures stencil.

---

## 5. Loadout (refonte sprint 1 sem 2)

**Layout cible — style "armurerie"** :
```
╔══════════════════════════════════════════════════════════╗
║   < RETOUR                    ARMEMENT — ÉTABLI          ║
║                                                          ║
║   ╔══════════════════════════════════════════════════╗  ║
║   ║          ARME PRIMAIRE  [slot 1]                 ║  ║
║   ║   ◀  ┌─────────────────────────────────┐  ▶      ║  ║
║   ║      │   [WEAPON ICON 96×64]           │         ║  ║
║   ║      │                                 │         ║  ║
║   ║      │   ASSAULT RIFLE — AK74         │         ║  ║
║   ║      │   ━━━━━━━━━━━━━━━━━━━━         │         ║  ║
║   ║      │   DAMAGE   ████████░  36       │         ║  ║
║   ║      │   FIRE RT  ██████░░░  600       │         ║  ║
║   ║      │   MAG      █████████  30       │         ║  ║
║   ║      │   RELOAD   ███░░░░░░  2.3s     │         ║  ║
║   ║      │   AMMO     [icon] Rifle (5.56) │         ║  ║
║   ║      └─────────────────────────────────┘         ║  ║
║   ╚══════════════════════════════════════════════════╝  ║
║                                                          ║
║   ╔══════════════════════════════════════════════════╗  ║
║   ║          ARME SECONDAIRE  [slot 2]               ║  ║
║   ║   ◀  ┌─────────────────────────────────┐  ▶      ║  ║
║   ║      │   [WEAPON ICON]                 │         ║  ║
║   ║      │   PISTOL — M92F                 │         ║  ║
║   ║      │   (stats compactes)             │         ║  ║
║   ║      └─────────────────────────────────┘         ║  ║
║   ╚══════════════════════════════════════════════════╝  ║
║                                                          ║
║   MELEE: COUTEAU TACTIQUE (toujours équipé)             ║
║                                                          ║
║                              [CONFIRMER LOADOUT]         ║
╚══════════════════════════════════════════════════════════╝
```

**Specs** :
- **Slots** : 2 panels horizontaux (primaire / secondaire)
- **Boutons ◀▶** : cyclent les armes disponibles
- **Weapon icon** : grand format 96×64
- **Stats bars** : graduées, `GoldDim` fill
- **Ammo type** : icône hexagonale + nom (ex: `Rifle (5.56)`)
- **Footer Melee** : info-only, italique, `GreySoft`

**Existant** : 4/10 → upgrade massif requis. Le code est en place (`ULoadoutWidget::PopulateFromClasses`), juste l'habillage à refondre.

---

## 6. Options (refonte sprint 1 sem 4)

**Layout cible — style "fiche briefing"** :
```
╔══════════════════════════════════════════════════════════╗
║   < RETOUR                       PARAMÈTRES              ║
║                                                          ║
║   ╔════════════╗  ╔════════════════════════════════════╗║
║   ║ AUDIO    ► ║  ║                                    ║║
║   ║            ║  ║   AUDIO                            ║║
║   ║ VIDÉO      ║  ║                                    ║║
║   ║            ║  ║   Master         ████████░  85%   ║║
║   ║ CONTRÔLES  ║  ║   Music          ██████░░░  60%   ║║
║   ║            ║  ║   SFX            █████████  100%  ║║
║   ║ GAMEPLAY   ║  ║   Ambience       ███░░░░░░  30%   ║║
║   ║            ║  ║                                    ║║
║   ║            ║  ║   ☑ Désactiver tutoriel           ║║
║   ║            ║  ║                                    ║║
║   ╚════════════╝  ╚════════════════════════════════════╝║
║                                                          ║
║   [APPLIQUER]    [PAR DÉFAUT]                            ║
╚══════════════════════════════════════════════════════════╝
```

**Specs** :
- **Sidebar gauche** : 4 sections (Audio / Vidéo / Contrôles / Gameplay)
- **Sidebar item actif** : barre `RedBlood` + bg `BgPanelHover`
- **Panel droit** : contenu de la section sélectionnée
- **Sliders custom** : trackbar `BgPanelDim` + fill `GoldDim` + handle `WhiteText`
- **Checkboxes** : style stencil (carré + check rouge si actif)

**Section Contrôles (existant)** : key rebinding via `IMC` UE5 — voir `OptionsWidget.cpp`.

---

## 7. GameOver (refonte sprint 1 sem 4)

**Layout cible** :
```
╔══════════════════════════════════════════════════════════╗
║                                                          ║
║                  ╔══════════════════╗                    ║
║                  ║                  ║                    ║
║                  ║       S          ║   ← Tier letter    ║
║                  ║   ▓▓▓▓▓▓▓▓       ║      Stencil 200pt ║
║                  ║                  ║      RedBlood      ║
║                  ╚══════════════════╝                    ║
║                                                          ║
║                  MISSION COMPLETE                        ║
║                                                          ║
║   ┌────────────────────────────────────────────────┐    ║
║   │  SCORE FINAL          18,450                    │    ║
║   │  KILLS                  87                      │    ║
║   │  BEST COMBO             x12                     │    ║
║   │  TIME SURVIVED          5:00                    │    ║
║   │  ──────────────────────────────                 │    ║
║   │  XP GAGNÉ            +5,500 (×3 bonus tier S)   │    ║
║   └────────────────────────────────────────────────┘    ║
║                                                          ║
║   ┌─────────────────────────────────────────────┐       ║
║   │ 🏆 ACHIEVEMENT UNLOCKED — "COMBO MASTER"    │       ║
║   └─────────────────────────────────────────────┘       ║
║                                                          ║
║   [REJOUER]    [LEADERBOARD]    [QUITTER]               ║
╚══════════════════════════════════════════════════════════╝
```

**Specs** :
- **Tier letter** : énorme (200pt), centrée, fade-in dramatique 1s + scale
  - S : `RedBlood` + glow rouge
  - A : `GoldTarnish`
  - B : `WhiteText`
  - C : `GreySoft`
  - D : `GreyMid`
- **Stats** : Roboto Mono pour alignement parfait
- **Achievement popup** : si débloqué pendant le run, affiché en encart `BgPanel` bordure `GoldTarnish`
- **Boutons** : 3 actions, REJOUER en `RedBlood` (action principale)

**Tier logic** (sprint 3) :
```
Score >= 15000 + Combo >= 10  → S
Score >= 10000 + Combo >= 7   → A
Score >= 6000  + Combo >= 5   → B
Score >= 3000                  → C
Score < 3000                   → D
```

---

## 8. HUD in-game (polish sprint 1 sem 4)

**Existant** : 8/10. Polish :
- Bordures stencil sur Health/Stamina/Ammo panels
- Font Mono pour numbers (ammo, timer, score)
- Icônes status (sprint actif, crouch, ADS)

**Layout existant** :
```
┌────────────────────────────────────────────────────┐
│ TIMER 5:00      MISSION                  WAVE 1/3  │
│                                                    │
│                                                    │
│                                                    │
│                       +                            │  ← crosshair
│                                                    │
│                                                    │
│                                                    │
│  HP ████████░ 80%        RIFLE  30/120  COMBO x5   │
│  STAM ██████░ 60%        ┌─────┐                   │
│                          │ ICO │       SCORE       │
│                          └─────┘       12,450      │
└────────────────────────────────────────────────────┘
```

**Modifs sprint 1** :
- Health bar : bordure stencil (fine)
- Ammo : font Share Tech Mono 36pt
- Combo : Stencil 32pt `ComboGold`, pulse à chaque kill
- Score : Mono 24pt
- Timer : Mono 28pt, devient `RedAlert` pulse à <30s

---

## 9. Inventory (polish sprint 1 sem 4)

**Existant** : 6/10. Polish frame "caisse militaire" :

```
   ╔═══════════════════════════════════════════╗
   ║  INVENTAIRE — CAISSE PERSONNELLE          ║
   ║  ┌───┬───┬───┐                            ║
   ║  │ ▓ │   │   │   [E] Utiliser             ║
   ║  ├───┼───┼───┤                            ║
   ║  │   │ ▓ │   │   [Tab] Fermer             ║
   ║  ├───┼───┼───┤                            ║
   ║  │   │   │   │                            ║
   ║  └───┴───┴───┘                            ║
   ║                                            ║
   ║  Item sélectionné: KIT DE SOIN            ║
   ║  Description: Restaure 50% des PV.        ║
   ║                                            ║
   ╚═══════════════════════════════════════════╝
```

**Specs** :
- Grid 3×3 fixe
- Slots : 80×80px, bordure stencil
- Slot sélectionné : bordure `GoldTarnish` 2px, slight glow
- Item icon centré
- Quantité bottom-right (Mono 12pt)
- Description panel en bas avec étiquette laiton

---

## Patterns réutilisables (créer dans `UIHelpers.h`)

```cpp
// Helper pour créer une bordure stencil
UBorder* MakeStencilBorder(UWidgetTree* Tree, FLinearColor Color, FMargin Padding = FMargin(16.f));

// Helper pour créer une stat bar (label + bar + valeur)
UWidget* MakeStatBar(UWidgetTree* Tree, FText Label, float Value, float Max, FLinearColor Color);

// Helper pour créer un bouton premium (style RE Grit)
UButton* MakePremiumButton(UWidgetTree* Tree, FText Label, FLinearColor BgColor, FLinearColor HoverColor);

// Helper pour étiquette laiton
UTextBlock* MakeBrassLabel(UWidgetTree* Tree, FText Text);
```

---

## Validation visuelle (fin sprint 1)

Pour chaque écran, vérifier :
- [ ] Palette uniquement issue de `UUITheme` (pas de hex hardcodé)
- [ ] Fonts custom appliquées (pas de font système UE5)
- [ ] Bordures stencil présentes sur les panneaux principaux
- [ ] Animations fade-in à l'ouverture
- [ ] Sons hover/click sur tous les boutons
- [ ] Lisibilité contraste WCAG AA min
- [ ] Hierarchy claire (titre/sub/body évidents)
