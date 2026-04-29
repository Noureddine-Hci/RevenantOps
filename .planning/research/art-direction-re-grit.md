# Art Direction — "Combat Zone Forsaken" (RE Grit)

> Direction artistique de RevenantOps v4.0. Mood, palette, typographie, refs visuelles.

---

## Mood — l'idée centrale

**Une opération militaire qui a mal tourné.** Base secrète abandonnée, équipement laissé sur place, lumières qui clignotent, taches de sang séchées sur les murs. Le joueur arrive après la catastrophe.

L'identité doit transmettre :
- **Tension oppressante** — pas de couleurs vives, pas de fun cartoon
- **Crédibilité militaire** — terminologie OTAN, fonts stencil, équipement réaliste
- **Trace humaine** — pas un environnement stérile : papiers déchirés, gobelets renversés, photos
- **Danger latent** — le rouge sang n'est pas décoratif, il signale toujours quelque chose (mort, alerte, menace)

---

## Inspirations directes

### Resident Evil 5 (Mercenaries mode)
- Layout HUD : timer en haut, score en haut-droit, ammo en bas-droit
- Sound design : musique anxiogène + cris de zombies
- Typographie : sérif/sans-serif mixé, beaucoup de blanc cassé
- Color grading : tons chauds (sable, ocre) + accents sang

### Tom Clancy's The Division (1 & 2)
- UI militaire moderne : boxes rectangulaires, hierarchie claire
- Iconographie : icônes vectorielles fines, blanc + accent orange
- Animations UI : slides latéraux nets, fade-in courts
- HUD minimaliste mais info-dense

### Metal Gear Solid V: The Phantom Pain
- Diegetic UI : éléments qui ressemblent à du matériel militaire (radio, briefing papers)
- Color : kaki délavé + rouge sang + acier
- Texture omniprésente : grain, scratches, scanlines

### Call of Duty: Modern Warfare (2019)
- Palette sombre + accent rouge tactique
- Fonts stencil pour les titres
- Bordures nettes, séparations visuelles fortes

---

## Palette finale (validée pour `UUITheme`)

### Backgrounds
```cpp
BgDeep         = (0.030, 0.025, 0.020, 1.0)  // Noir terre brûlée — fond plein écran
BgPanel        = (0.070, 0.060, 0.040, 1.0)  // Panneau sombre — boxes/cards
BgPanelDim     = (0.050, 0.040, 0.030, 0.85) // Overlay translucide — pop-ups
BgPanelHover   = (0.110, 0.090, 0.060, 1.0)  // Panneau au survol
```

### Accents principaux
```cpp
RedBlood       = (0.750, 0.150, 0.100, 1.0)  // Rouge sang — boutons d'action, alertes
RedAlert       = (1.000, 0.250, 0.150, 1.0)  // Rouge vif — danger immédiat (low HP)
RedDeep        = (0.450, 0.080, 0.050, 1.0)  // Rouge profond — fond accent
GoldTarnish    = (0.850, 0.700, 0.300, 1.0)  // Or terni — titres, accents premium
GoldDim        = (0.550, 0.450, 0.200, 1.0)  // Or éteint — sub-text, dividers
```

### Neutres / texte
```cpp
WhiteText      = (0.950, 0.930, 0.880, 1.0)  // Blanc cassé — texte courant
GreySoft       = (0.450, 0.420, 0.380, 1.0)  // Gris doux — text secondaire
GreyDeep       = (0.180, 0.160, 0.140, 1.0)  // Gris profond — séparateurs
GreyMid        = (0.300, 0.280, 0.250, 1.0)  // Gris milieu — borders inactifs
```

### Sémantique gameplay
```cpp
HealthGreen    = (0.250, 0.700, 0.300, 1.0)  // Vie pleine
HealthYellow   = (0.900, 0.750, 0.200, 1.0)  // Vie 50%
HealthRed      = (0.850, 0.150, 0.150, 1.0)  // Vie critique
StaminaBlue    = (0.400, 0.600, 0.850, 1.0)  // Stamina
ComboGold      = (1.000, 0.800, 0.000, 1.0)  // Combo (existe déjà HUD)
KillYellow     = (1.000, 1.000, 0.000, 1.0)  // Notification kill
```

---

## Typographie

### Titres (h1, h2)
- **Police** : `Allerta Stencil` ou `Saira Stencil One` (Google Fonts, gratuit, OFL)
- **Poids** : Regular
- **Tracking** : +50 (espacement large)
- **Transform** : MAJUSCULES toujours
- **Couleur** : `GoldTarnish` (titres principaux) ou `WhiteText` (sub-titres)

### Body (paragraphes, descriptions)
- **Police** : `Roboto Condensed` (Google Fonts, Apache 2.0)
- **Poids** : Regular 400 / Bold 700 pour emphasis
- **Couleur** : `WhiteText` (body) / `GreySoft` (secondaire)
- **Line height** : 1.4

### HUD numbers (ammo, timer, score)
- **Police** : `Share Tech Mono` ou `JetBrains Mono` (Google Fonts, OFL/Apache 2.0)
- **Poids** : Regular
- **Couleur** : `WhiteText`, `RedAlert` quand <30s ou ammo bas

### Buttons / UI labels
- **Police** : `Roboto Condensed` Bold 700
- **Tracking** : +30
- **Transform** : MAJUSCULES

---

## Effets visuels signature

### 1. Vignette permanente
Subtil assombrissement des bords (~15%) sur tous les menus. Donne une focalisation centrale et un côté "vu à travers une optique militaire".

**Implémentation** : `UImage` plein écran avec un brush gradient radial transparent au centre, sombre aux bords. Z-order le plus élevé, `HitTestInvisible`.

### 2. Grain / scanlines (subtle)
Texture grain sur les transitions menu uniquement (pas permanent — fatiguant). 5-8% d'opacité, animé.

**Implémentation** : material `M_UI_Grain` avec `Texture2D` noise + scrolling UV. Brush sur un `UImage` plein écran activé pendant transitions.

### 3. Glitch flash rouge
Bref flash rouge (100ms) sur events critiques :
- Joueur prend dégât critique (< 25% HP)
- Kill (très subtle, juste un flash rouge périphérique)
- Achievement unlocked
- Musique combat starts

**Implémentation** : `UImage` plein écran rouge, animation opacity 0→0.3→0 sur 100ms.

### 4. Bordures stencil "metal plating"
Panneaux UI ont des bordures qui ressemblent à du métal vissé / soudé. Pas juste un rectangle.

**Implémentation** : `UBorder` avec `Brush.DrawAs = ESlateBrushDrawType::Box`, marges configurées, texture métal pré-faite (Nourredine sprint 4 — `Tex_StencilFrame.png`).

### 5. Stamps / labels laiton
Étiquettes type "RESTRICTED", "CLASSIFIED", "MISSION 03" en or terni sur fond sombre, comme tamponnés.

**Implémentation** : `UTextBlock` avec `GoldTarnish` color, `Allerta Stencil`, bordure rectangulaire fine.

---

## Iconographie

### Style général
- **Vectorielles fines** (1-2px stroke), pas de fill solide
- **Monochromes** par défaut (`WhiteText` ou `GoldDim`), accent couleur sur état actif
- **Style militaire** : reconnaissable (silhouettes claires), pas trop détaillé

### Icônes critiques à produire (sprint 1)
- Icônes armes (déjà existantes, à harmoniser style)
- Icônes ammo type (pistol/rifle/shotgun/sniper) — frame hexagonal
- Icônes status (sprint, crouch, ADS) — pour HUD
- Icônes pickups (santé croix médicale, ammo cartouche, time horloge militaire)
- Logo "REVENANTOPS" — placeholder texte stencil pour le moment, version finale Nourredine sprint 3

---

## Layouts standards

### Spacing rules (8pt grid)
- Padding intérieur boxes : 16pt ou 24pt
- Gaps verticaux entre sections : 32pt
- Gaps horizontaux : 24pt
- Margins écran : 64pt depuis les bords

### Hierarchy visuelle (z-order)
```
0   : Background
5   : HUD éléments
10  : Menus
15  : Pop-ups (pickup prompt, finisher prompt)
20  : Notifications (achievement, kill)
25  : Modals (pause, options)
30  : Splash / transitions
```

---

## À NE PAS FAIRE

❌ **Couleurs vives saturées** — pas de bleu cyan, pas de violet, pas de vert lime. Tout passe par notre palette.

❌ **Comic Sans / fonts arrondies** — incompatible avec le mood militaire.

❌ **Effets glow néon** — c'est pas cyberpunk. Le seul "glow" autorisé est le rouge alert subtil.

❌ **Particules festives** — pas de confettis kill, pas d'étoiles brillantes. Le feedback positif passe par le son + le combo number qui pulse.

❌ **Backgrounds chargés en menus** — flous d'arrière-plan ou full screen image. On veut de la lisibilité, du minimalisme militaire.

❌ **UI trop info-dense** — éviter d'afficher 15 stats simultanément. Hiérarchiser.

---

## Refs visuelles à collecter (sprint 1)

À mettre dans `Content/Mercenaires/_References/` (dossier non-buildé) ou Drive partagé :

- 5 screenshots RE5 Mercenaries menus
- 5 screenshots The Division UI
- 3 screenshots MGS V briefing screens
- 3 screenshots COD Modern Warfare loadout
- Mood board photos : bunker abandonné, base militaire désaffectée, équipement militaire usé

**Responsable collection** : Jilani (Pinterest / Reddit / r/gameui)

---

## Validation

Une fois `UUITheme` créé et appliqué à TitleScreen, valider visuellement contre :
- ✅ Cohérence palette (pas de couleur "off")
- ✅ Lisibilité texte (contraste WCAG AA min)
- ✅ Hiérarchie claire (titre > sub > body évident)
- ✅ Mood militaire palpable (pas "menu Unreal basique")

Demo screenshot validée par les 2 devs avant de passer aux autres écrans.
