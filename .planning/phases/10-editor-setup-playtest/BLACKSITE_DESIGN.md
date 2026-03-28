# BLACKSITE — Layout Design Document
## Mode Mercenaires — RevenantOps

**Date:** 2026-03-26
**Thème:** Base militaire souterraine — éclairage rouge d'urgence
**Script MCP:** `Scripts/build_blacksite.py`

---

## Vue d'ensemble (top-down, X=Est, Y=Nord)

```
Y+2500 ┌────────────────────────────────────────┐
       │         LOADING DOCK                   │
       │   X:-4000 à +1000, Y:+1000 à +2500    │
       │   [CTRL ROOM élevé Z+600]  [containers]│
       │   X:-1500 à 0, Y:+1400-+2400           │
Y+1000 ├────────────────┬───────────┬───────────┤
       │                │ N_CONNECT │ SERVER RM  │
       │    HANGAR      │ X:+1000-  │ X:+2000-  │
       │  X:-4000→+1000 │ +2000     │ +4000     │
       │  Y:-2000→+1000 │ Y:+1000-  │ Y:-2000-  │
       │  [PlayerStart] │ +2000     │ +2000     │
       │                ├───────────┤           │
       │           CORR │           │           │
       │        X:+1000-│           │           │
Y-2000 └────────────────┴───────────┴───────────┘
       X:-4000        +1000      +2000        +4000
```

---

## Zones & Dimensions

| Zone | X min | X max | Y min | Y max | Z | Hauteur |
|------|-------|-------|-------|-------|---|---------|
| HANGAR | -4000 | +1000 | -2000 | +1000 | 0 | 800cm |
| CORRIDOR | +1000 | +2000 | -300 | +300 | 0 | 400cm |
| N_CONNECT | +1000 | +2000 | +1000 | +2000 | 0 | 600cm |
| SERVER ROOM | +2000 | +4000 | -2000 | +2000 | 0 | 500cm |
| LOADING DOCK | -4000 | +1000 | +1000 | +2500 | 0 | 600cm |
| CTRL ROOM | -1500 | 0 | +1400 | +2400 | +600 | 400cm |

---

## Circuit (boucle principale)

```
HANGAR → [porte est X=+1000, Y:±300] → CORRIDOR → [porte X=+2000]
→ SERVER ROOM → [porte nord X=+2000, Y:+1300-+1700] → N_CONNECT
→ [porte ouest X=+1000, pleine hauteur] → LOADING DOCK
→ [portes sud Y=+1000] → HANGAR
```

**Détour haut risque:** HANGAR → [escalier nord] → LOADING DOCK → CTRL ROOM
(time bonus pickup, sniper perch, un seul chemin — zone piégée si trop de zombies)

---

## Gameplay par zone

### HANGAR (départ joueur)
- **Rôle:** Zone centrale, grande, ouverte. Kite des Slows, DPS Tank à distance.
- **Armes recommandées:** AR, Sniper vers corridor, Shotgun si débordé
- **Danger:** Runners qui arrivent de tous côtés si le joueur s'attarde
- **Features:** 4 piliers béton (cover medium), 2 caisses équipement

### CORRIDOR (goulot d'étranglement)
- **Rôle:** Chokepoint dangereux. Exploders MORTELS ici. Shotgun = roi.
- **Armes recommandées:** Shotgun, SMG
- **Pickup:** AMMO au centre (ammo A1) — risqué car bloque le corridor
- **Design:** 400cm de haut (oppressant), 600cm de large (2 zombies côte à côte)

### SERVER ROOM (dense)
- **Rôle:** Zone de respiration momentanée mais Spitters visuellement gênés par les racks.
- **Armes recommandées:** Pistol/SMG entre les rangées, Sniper pour sightlines
- **Features:** 8 server racks (2 colonnes × 4 rangées), walkways entre
- **Pickup:** TIME BONUS au centre (TB2) — entouré de racks, pas facile à atteindre

### N_CONNECT (vestibule)
- **Rôle:** Transition Server Room ↔ Loading Dock. Espace intermédiaire.
- **Danger:** Runners peuvent contourner ici pour surprendre le joueur

### LOADING DOCK (secondaire ouvert)
- **Rôle:** Zone de respiration, containers comme cover. Ammo disponible.
- **Features:** 4 containers, accès au CTRL ROOM
- **Pickup:** TIME BONUS en coin NW (TB3) — dead-end, les zombies se groupent ici
- **Spawn:** SP7, SP8 (zombies arrivent par le nord)

### CTRL ROOM (élevé — risque/récompense)
- **Rôle:** Sniper heaven. Overlook hangar. Mais un seul chemin (escalier).
- **Armes recommandées:** Sniper uniquement (sinon gaspillage de la hauteur)
- **Pickup:** TIME BONUS (TB1) — meilleur reward, pire position si bloqué
- **Design:** Parapet 100cm au sud (tirer vers hangar), murs pleins ailleurs

---

## Placements importants

### PlayerStart
```
Position: (-2000, -500, 100)
Rotation: Yaw=90° (face à l'est, vers le corridor)
```

### Enemy Spawn Points (8 total)
```
SP1: (-3800, -1800, 100)  — SW hangar (arrivent par derrière)
SP2: (-3800,   600, 100)  — NW hangar (flanc gauche)
SP3: (  200, -1800, 100)  — S hangar center
SP4: ( 3800, -1800, 100)  — SE server room (via server room)
SP5: ( 3800,  1800, 100)  — NE server room
SP6: ( 3800,     0, 100)  — E server room (pression directe)
SP7: (-3800,  2200, 100)  — NW loading dock
SP8: (    0,  2400, 100)  — N loading dock center
```

### Pickups
```
TB1 (Time +30s): ( -750, 1900, 700)  — CTRL ROOM (risqué max)
TB2 (Time +20s): ( 3000,    0, 100)  — SERVER ROOM center (entouré de racks)
TB3 (Time +15s): (-3600, 2200, 100)  — LOADING DOCK coin NW (dead-end)
A1  (Ammo):      ( 1500,    0, 100)  — CORRIDOR center (chokepoint)
A2  (Ammo):      (-2000, -800, 100)  — HANGAR (libre accès, peu risqué)
```

### NavMesh Bounds
```
Center: (0, 250, 300)
Scale: (82, 52, 10)  — couvre tout le map + marge
```

---

## Notes techniques MCP

- Sol Z=0 (top surface), floor center Z=-50, scale_z=1
- Objet sur le sol: center_z = scale_z × 50
- Mur sur le sol: center_z = hauteur_cm / 2
- Plafond: center_z = hauteur_interieure + 50 (bottom face = hauteur intérieure)
- JAMAIS lancer le script en PIE — mode Editor uniquement
