# Blender Tasks — Guide Nourredine (Débutant)

> Guide pas-à-pas pour Nourredine. Tutos vidéo + checklists + workflow livraison.
> Niveau visé : débutant → intermédiaire en 12 semaines.

---

## Setup initial (jour 1)

### 1. Installer Blender 4.x
- Télécharger sur [blender.org/download](https://www.blender.org/download/)
- Version recommandée : **Blender 4.2 LTS** (stable, à jour)
- Installation simple : suivre l'installer

### 2. Configuration initiale
- Lancer Blender
- Edit → Preferences → Interface : choisir français si voulu (mais tutos sont en anglais)
- Edit → Preferences → Keymap : laisser par défaut (industry standard si tu viens d'autre 3D)
- Edit → Preferences → System : vérifier GPU pour preview (peut afficher CPU si pas de GPU dédié — pas grave)

### 3. Installer Audacity
- Télécharger sur [audacityteam.org](https://www.audacityteam.org/)
- Installation simple
- Ouvrir, créer un projet test, savoir où ouvrir/exporter un .wav

---

## Semaine 1 — Apprentissage Blender

### Tâche N1.1 — Tuto Donut Blender Guru

**Objectif** : maîtriser l'interface, les bases de modélisation, modifiers, materials, rendering.

**Durée** : 4-6 heures (réparties sur 2-3 jours)

**Lien** : [Blender Guru — Beginner Donut Tutorial](https://www.youtube.com/watch?v=B0J27sf9N1Y)

**Validation** : screenshot du donut final partagé sur Discord

**Ce que tu apprends** :
- Navigation 3D viewport (souris milieu, scroll, shift+scroll, ctrl+alt+Q)
- Modes Object / Edit / Sculpt
- Extrude, scale, rotate, loop cut
- Subdivision Surface modifier
- Création de materials (Principled BSDF)
- Render basique avec Cycles

### Tâche N1.2 — Audacity setup

**Objectif** : savoir normaliser et exporter un .wav.

**Tuto** : [Audacity Normalize Tutorial](https://www.youtube.com/watch?v=KNBbcCfCzDU) (10 min)

**Validation** : prendre n'importe quel .wav existant du projet (`Content/Mercenaires/Audio/SFX/`), le normaliser à -3dB peak, l'exporter dans un dossier perso. Confirmer dans Discord.

### Tâche N1.3 — Normalisation SFX

**Objectif** : normaliser tous les `SW_*.wav` à -3dB pour éviter les variations de volume.

**Étapes** :
1. Ouvrir Audacity
2. File → Open → sélectionner un .wav (depuis Drive partagé `incoming/SFX_Original/`)
3. Effect → Normalize → choisir -3dB → OK
4. File → Export → Export as WAV → mêmes settings (44.1kHz, stéréo si stéréo, 16-bit)
5. Naming : `SW_Fire_Heavy_norm.wav` (suffixe `_norm`)
6. Répéter pour les 8 fichiers SFX

**Livrable** : pack ZIP `Audio_Normalized_v1.zip` dans Drive `incoming/`

---

## Semaine 2 — Modélisation simple (props level)

### Workflow recommandé pour chaque prop

1. Reference image dans Blender (drag-drop dans le viewport, vue front)
2. Cube de base (Add → Mesh → Cube)
3. Edit Mode (Tab) : modifier le cube pour matcher la silhouette
4. Loop cuts (Ctrl+R) pour ajouter des subdivisions
5. Bevel (Ctrl+B) pour adoucir les arêtes
6. Modifier Subdivision Surface pour les formes organiques
7. Material simple (couleur de base + roughness)
8. Export FBX (voir section Export ci-dessous)

### Tâche N2.1 à N2.5 — 5 props

**Refs photos** : Jilani fournit 5 images dans Drive `incoming/Refs_Props/`

| Prop | Polycount cible | Difficulté | Tuto recommandé |
|---|---|---|---|
| Caisse munitions | < 500 tris | ★☆☆ | [Imphenzia Low Poly Crate](https://www.youtube.com/watch?v=1jHUY3qoBu8) |
| Barrel rouillé | < 800 tris | ★☆☆ | [Blender Barrel Tutorial](https://www.youtube.com/watch?v=gfXRiP9rhiY) (15 min) |
| Panneau "RESTRICTED" | < 200 tris | ★☆☆ | Plane + texture (image) |
| Lampe cassée | < 600 tris | ★★☆ | Cube + sphere + chaîne (modifiers) |
| Douille au sol | < 100 tris | ★☆☆ | Cylinder + 2 loops |

**Tips** :
- Pas de high-poly (le moteur va tout afficher en runtime)
- Material rouille / métal usé : Principled BSDF avec roughness 0.7-0.9, base color brun/gris
- Ne pas se soucier des UVs avancés au début — UV smart project fait le job

**Livrable** : 5 FBX dans Drive `incoming/Props_v1/`
- Naming : `Prop_Crate.fbx`, `Prop_Barrel.fbx`, etc.

---

## Semaine 3 — Mixamo + ambiance audio

### Tâche N3.1 — Mixamo workflow

**But** : comprendre comment importer un personnage UE5 dans Blender et identifier les os.

**Tuto vidéo critique** : [Smart Poly — UE5 Mannequin to Blender to Mixamo Workflow](https://www.youtube.com/watch?v=B8uIu-V-XgQ) (15 min)

**Étapes pratiques** :
1. Jilani exporte le mannequin UE5 squelette en FBX → Drive `incoming/Skeleton_UE5_Mannequin.fbx`
2. Tu l'importes dans Blender : File → Import → FBX
3. Inspecter la hierarchy bones (panneau outliner) :
   - root → pelvis → spine_01/02/03 → neck_01 → head
   - branches : clavicle_l/r → upperarm → lowerarm → hand
   - jambes : thigh_l/r → calf → foot → ball
4. Screenshot de la hierarchy + partage Discord

**Important** : tu ne touches pas encore aux animations cette semaine. Juste comprendre la structure.

### Tâche N3.4 — Ambiance Blacksite

**But** : créer une ambiance audio loopable de 60s pour le niveau Blacksite.

**Composition (4 couches dans Audacity)** :

1. **Vent extérieur** (CC0)
   - Source : freesound.org → "wind ambience loop"
   - Couper 60s, fade in/out 1s, normaliser -18dB

2. **Bourdon électrique** (CC0)
   - Source : freesound.org → "electric hum loop", "transformer hum"
   - Couper 60s, normaliser -22dB

3. **Radio statique périodique** (CC0)
   - Source : freesound.org → "radio static short"
   - Insérer 2-3 occurrences sur 60s aux positions aléatoires
   - Volume -20dB

4. **Effets distants** (optionnel)
   - Gouttes d'eau, ferraille au loin
   - Très subtle, -28dB

**Workflow Audacity** :
1. New project
2. Tracks → Add New → Stereo Track (4 fois pour 4 couches)
3. Importer chaque .wav dans une track : File → Import → Audio
4. Aligner sur 60s, ajuster volumes
5. Sélectionner tout (Ctrl+A) → Effect → Mix and Render
6. Vérifier le loop : copier-coller la track sur 120s pour entendre la jonction
7. Si "click" au loop : Effect → Crossfade Tracks (0.5s)
8. File → Export → Export as WAV → `Ambience_Blacksite_Loop.wav`

**Validation** : écouter en boucle 5 fois, aucun "click" audible

---

## Semaine 4 — Weapon attachments + split musique

### Tâche N4.1 à N4.3 — 3 weapon attachments

| Attachment | Polycount | Difficulté |
|---|---|---|
| Silencer | < 300 tris | ★☆☆ — cylindre + détails |
| Red dot sight | < 500 tris | ★★☆ — base + viseur cubique + lentille |
| Tactical light | < 400 tris | ★☆☆ — cylindre + lentille |

**Tuto général** : [Imphenzia — Weapon Attachments Speed Modeling](https://www.youtube.com/watch?v=1jHUY3qoBu8)

**Specs** :
- Hauteur silencer : ~10cm (échelle Blender = 1m donc 0.1)
- Échelle cohérente avec un canon de fusil (diamètre ~3cm = 0.03)
- Material métal noir mat (roughness 0.6, base color #1a1a1a)

### Tâche N4.5 — Split OST_1 en 3 phases

**Source** : `OST_1.wav` (42 MB) dans `Content/Mercenaires/Audio/Musiques/` (Jilani fournit copie locale)

**Workflow Audacity** :
1. Ouvrir OST_1.wav
2. **Repérer les sections musicales** en écoutant :
   - Intro : début → premier "drop" / changement de tempo (typiquement 8-15s)
   - Loop : section centrale qui peut boucler (60s contiguës)
   - Outro : finition naturelle (6-10s)
3. **Extraire chaque section** :
   - Sélectionner la section (drag dans la timeline)
   - File → Export Selected → WAV
   - Naming : `Music_Combat_Intro.wav`, `Music_Combat_Loop.wav`, `Music_Combat_Outro.wav`
4. **Pour le LOOP** : critique, doit boucler proprement
   - Tester : copier-coller la section sur 120s
   - Si "click" à la jonction : appliquer fade-in 50ms au début + fade-out 50ms à la fin
   - Re-tester loop, valider
5. **Pour l'INTRO** : doit transitionner naturellement vers le loop
   - Note la dernière note de l'intro et la première note du loop — elles doivent matcher musicalement
6. **Pour l'OUTRO** : fade-out à la fin (2-3s)

**Livrable** : 3 fichiers `.wav` dans Drive `incoming/Music_Splits_v1/`

---

## EXPORT FBX (workflow critique)

À chaque export d'un asset Blender, suivre ces settings exacts pour éviter les surprises à l'import UE5 :

### Settings FBX (File → Export → FBX)

```
Path Mode: Copy
Embed Textures: NON (textures séparées)
Use Custom Properties: NON

Geometries:
  Smoothing: Face
  Apply Modifiers: OUI
  Use Modifiers Render Setting: OUI
  Loose Edges: NON
  Triangulate Faces: OUI
  Tangent Space: OUI

Armature (si anim):
  Primary Bone Axis: Y Axis
  Secondary Bone Axis: X Axis
  Armature FBXNode Type: Null
  Only Deform Bones: OUI
  Add Leaf Bones: NON

Bake Animation (si anim):
  Bake Animation: OUI
  Key All Bones: OUI
  Force Start/End Keying: OUI
  Sampling Rate: 1.0
  Simplify: 1.0

Transform:
  Scale: 1.0
  Apply Scalings: All Local
  Forward: -Y Forward
  Up: Z Up
  Apply Unit: OUI
  Use Space Transform: OUI
```

### Naming convention FBX

| Type | Pattern | Exemple |
|---|---|---|
| Prop | `Prop_<Name>.fbx` | `Prop_Crate.fbx` |
| Weapon attachment | `WepAttach_<Name>.fbx` | `WepAttach_Silencer.fbx` |
| Player anim | `Player_<Action>.fbx` | `Player_Crouch_Idle.fbx` |
| Enemy anim | `Enemy_<Type>_<Action>.fbx` | `Enemy_Zombie_Death_01.fbx` |

---

## Semaines 5-8 — Animations Blender (intermédiaire)

### Workflow général animation

**À chaque animation, le process est identique** :

1. Importer le squelette UE5 Mannequin (FBX fourni par Jilani)
2. Mode Pose (sélectionner armature → Tab pour Edit Mode → Mode Pose)
3. Activer auto-keyframing (timeline → bouton record rouge)
4. Frame 1 : pose initiale (utiliser keyframe All Channels)
5. Frame N : pose suivante (keyframe automatique)
6. Continuer jusqu'à la fin de l'animation
7. Vérifier en pressant Espace (play timeline)
8. Affiner avec Graph Editor pour les courbes
9. Export FBX avec **Bake Animation: OUI**

### Tâche N5.1 — Pickup animation (★☆☆)

**Description** : personnage se baisse, ramasse, se relève. 1.5s total.

**Frames clés** :
- Frame 1 (0s) : idle pose
- Frame 18 (0.6s) : penché en avant, hanches abaissées de 30cm, bras tendus vers le sol
- Frame 28 (0.95s) : pose ramassage (main droite à terre)
- Frame 38 (1.3s) : retour position penchée
- Frame 45 (1.5s) : retour idle

**Tuto** : [Blender Pickup Animation Beginner](https://www.youtube.com/watch?v=8Cp3JFW2K7A) (30 min)

**Tip** : pas besoin d'IK, anime juste les bones spine, hips, upperarm, lowerarm, hand.

### Tâche N5.2 — Player Death (★☆☆)

**Description** : chute en avant, atterrissage face contre terre. 2s.

**Frames clés** :
- Frame 1 (0s) : idle
- Frame 12 (0.4s) : début bascule avant (rotation pelvis +20°)
- Frame 30 (1s) : à mi-chute, bras qui s'étendent
- Frame 50 (1.65s) : impact sol
- Frame 60 (2s) : pose finale (ragdoll-ready)

**Référence** : death animation dans Skyrim, Fallout, n'importe quel TPS.

### Tâche N6.1-N6.2 — Crouch Idle + Walk (★★☆)

**Crouch Idle** (loop 2s) :
- Pelvis abaissé de 50cm, genoux fléchis
- Légère oscillation respiration (pelvis Y ±2cm sur 2s)

**Crouch Walk** (loop 1s) :
- Cycle de pas en position accroupie
- Pied gauche frame 1, pied droit frame 30, retour frame 60

**Tuto** : [Blender Walk Cycle Tutorial](https://www.youtube.com/watch?v=KZi9fnlCmJk) — adapter en mode crouch

### Tâche N7.1 — Sprint loop (★★☆)

**Description** : cycle marche rapide / course. 1s loop.

**Frames clés** (24fps = 24 frames pour 1s) :
- Frame 1 : pied droit en avant, pied gauche en arrière
- Frame 12 : passage central, jambes croisées
- Frame 24 : pied gauche en avant, pied droit en arrière (mirror frame 1)

**Caractéristiques sprint vs walk** :
- Amplitude jambes plus large
- Buste légèrement penché en avant
- Bras qui se balancent vigoureusement

**Tuto** : [Blender Run Cycle](https://www.youtube.com/watch?v=4f5Mzb7Y3Eg)

### Tâche N8.1 — Slide (★★★ avec accompagnement)

**Difficile pour débutant** — Jilani et toi faites un appel Discord 1h.

**Description** : glissement avant 1s. Le perso passe de course à position glissée (genou plié, jambe avant tendue).

**Frames clés** :
- Frame 1 : pose course
- Frame 6 : début slide (pelvis bas, jambe avant tendue)
- Frame 20 : pose slide maintenue
- Frame 30 : retour pose normale (montage one-shot, pas de loop)

---

## Semaines 9-12 — Animations ennemis (intermédiaire-avancé)

Si tu progresses bien, on attaque les zombies.

### N10.1-N10.3 — Stagger (★★☆)

**Stagger Head** (1s) : zombie chancelle, mains à la tête, recul léger
**Stagger Leg** (1.5s) : zombie tombe à genoux, supporte avec bras
**Stagger Torso** (1s) : zombie recule en se tordant, bras vers torse

**Skelette à utiliser** : même UE5 Mannequin (les zombies utilisent le même squelette dans le projet)

### N11.1-N11.3 — Death variations (★★☆)

**Death 01** : chute en avant (front)
**Death 02** : chute sur le côté droit
**Death 03** : chute en arrière

Chaque animation 2s, ragdoll-ready à la fin.

### N12.1-N12.3 — Attack animations (★★★)

**Attack Slow (mêlée bras)** : 1.5s — bras levés, swing horizontal
**Attack Runner (morsure)** : 1s — leap forward, bouche ouverte
**Attack Tank (charge)** : 2s — bras croisés en charge bélier

---

## Validation des livraisons

Quand tu envoies un FBX, **toujours** :

1. Tester l'import dans un nouveau projet Blender (sanity check)
2. Vérifier que l'animation joue correctement (pas de bone qui part dans l'espace)
3. Pas de root motion (le perso ne bouge pas dans le monde, juste les bones)
4. Naming correct selon convention
5. Upload dans Drive `incoming/<sprint>/<type>/`
6. Message Discord : "Anim [name] livrée — frames [N], durée [Ns]"

**Jilani** confirme l'import dans `STATE.md` section "Feedback Nourredine" sous 24h.

---

## Ressources tutoriels — bookmarks

### Blender bases
- [Blender Guru — Beginner Donut](https://www.youtube.com/watch?v=B0J27sf9N1Y)
- [Imphenzia — Low Poly Modelling](https://www.youtube.com/watch?v=1jHUY3qoBu8)
- [Blender Daily Tips](https://www.youtube.com/c/BlenderDailyTutorials)

### Animation Blender
- [Sir Wade — Animation Fundamentals](https://www.youtube.com/c/SirWadeNeistadt)
- [Royal Skies — Rigging & Animation Series](https://www.youtube.com/c/TheRoyalSkies)

### UE5 + Blender workflow
- [Smart Poly — UE5 Mannequin Workflow](https://www.youtube.com/c/SmartPoly)
- [Pixel Helmet — UE5 Asset Pipeline](https://www.youtube.com/c/PixelHelmet)

### Audacity
- [Audacity Official Tutorials](https://manual.audacityteam.org/tutorials.html)

### Sources sons CC0
- [Freesound.org](https://freesound.org/) — filtrer par CC0
- [BBC Sound Effects](https://bbcsfx.acropolis.org.uk/) — license RemArc
- [Sonniss GDC Bundle](https://sonniss.com/gameaudiogdc) — annuel gratuit
- [Pixabay Music](https://pixabay.com/music/) — license commerciale OK

### Refs visuelles
- [BlenderArtists Forum](https://blenderartists.org/) — communauté FR/EN
- [ArtStation](https://www.artstation.com/) — refs pro
- [Pinterest — military props](https://www.pinterest.fr/search/pins/?q=military%20props%203d) — moodboard

---

## Quand tu bloques

**Avant de demander de l'aide**, essaye :
1. Re-regarder le tuto vidéo
2. Recherche YouTube : "blender [problem]"
3. Forum BlenderArtists / Reddit r/blender

**Si toujours bloqué** :
1. Screenshot du problème + description
2. Discord channel `#noureddine-help`
3. Jilani répond sous 24h
4. Si critique : appel vocal Discord

**On itère ensemble** — il n'y a pas de mauvaise question, juste des étapes à comprendre.
