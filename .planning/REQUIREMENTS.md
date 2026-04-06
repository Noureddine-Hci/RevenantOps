# Requirements: RevenantOps

**Defined:** 2026-03-22
**Core Value:** Le joueur peut lancer une partie, choisir son loadout, tuer des zombies en enchainant des combos sous pression du timer, et voir son score final.

## v1 Requirements

Requirements pour la demo partageable du mode Mercenaires.

### Camera

- [ ] **CAM-01**: Camera over-the-shoulder serree (distance courte, offset epaule, style RE4)
- [ ] **CAM-02**: Zoom-in supplementaire en mode visee (clic droit)

### Timer & Score

- [ ] **SCORE-01**: Timer fixe de 5 minutes affiche a l'ecran
- [ ] **SCORE-02**: Bonus de temps ramassables sur la map (+30s, +15s)
- [ ] **SCORE-03**: Score par kill (points variables selon type de zombie)
- [ ] **SCORE-04**: Multiplicateur de combo (x2, x3, x4...) qui monte avec les kills rapides
- [ ] **SCORE-05**: Le combo reset apres X secondes sans kill (timer de combo visible)
- [ ] **SCORE-06**: Fin de partie quand le timer atteint zero

### Zombies

- [x] **ZOMB-01**: Zombie lent (horde de base, peu de vie, degats faibles, spawn en groupe)
- [x] **ZOMB-02**: Zombie rapide (coureur, peu de vie, degats moyens, attaque en sprint)
- [x] **ZOMB-03**: Zombie tank (gros costaud, beaucoup de vie, degats lourds, lent)
- [x] **ZOMB-04**: Zombie cracheur (attaque a distance, projectile, vie moyenne)
- [x] **ZOMB-05**: Zombie explosif (explose a proximite, degats de zone, meurt en explosant)
- [x] **ZOMB-06**: Waves progressives (difficulte croissante, plus de zombies, mix de types)

### Armes

- [x] **ARME-01**: Pistolet (semi-auto, degats moyens, munitions abondantes)
- [x] **ARME-02**: Fusil d'assaut (full-auto, degats faibles, cadence rapide)
- [x] **ARME-03**: Shotgun (degats massifs proches, spread, rechargement lent)
- [x] **ARME-04**: Sniper (degats enormes, lent, zoom)
- [x] **ARME-05**: SMG (cadence tres rapide, degats faibles, grande capacite)
- [ ] **ARME-06**: Arme de melee (couteau/machette, degats bons, pas de munitions, risque)

### Loadout

- [ ] **LOAD-01**: Ecran de selection de loadout avant la partie
- [ ] **LOAD-02**: Le joueur choisit 2 armes a feu + melee toujours equipee
- [ ] **LOAD-03**: Affichage des stats de chaque arme dans l'ecran de selection

### Arene

- [ ] **AREN-01**: 1 arene jouable avec zones variees (ouverte, couloirs, hauteurs)
- [ ] **AREN-02**: Points de spawn zombies repartis dans l'arene
- [ ] **AREN-03**: Pickups de bonus temps places dans l'arene
- [ ] **AREN-04**: Pickups de munitions dans l'arene
- [ ] **AREN-05**: Eclairage et ambiance sombre/tendue

### UI & Menus

- [ ] **UI-01**: Ecran titre avec "Jouer" et "Quitter"
- [ ] **UI-02**: HUD en jeu (vie, munitions, arme, timer, score, combo)
- [ ] **UI-03**: Ecran de fin de partie (score, stats, rejouer)
- [ ] **UI-04**: Leaderboard local (top 10 scores)

### Audio & VFX

- [ ] **FX-01**: Sons de tir pour chaque arme
- [ ] **FX-02**: Sons d'impact (balle sur zombie, melee)
- [ ] **FX-03**: Sons de zombie (grognements, attaque, mort)
- [ ] **FX-04**: Musique d'ambiance tension/action
- [ ] **FX-05**: VFX de base (muzzle flash, impact sang, explosion)

## v2.0 Requirements — Finition Mode Mercenaires

### Assets Armes (ASSET)

- [ ] **ASSET-01**: Les 6 armes (pistolet, fusil, SMG, shotgun, sniper, melee) ont un vrai mesh 3D visible quand equipees (assets importes depuis Fab.com)
- [ ] **ASSET-02**: Les meshes armes sont correctement positionnes et orientes sur le socket hand_r du Mannequin UE5

### DataTables (DATA)

- [x] **DATA-01**: Les stats des armes (damage, fireRate, ammo, range) sont editables via DataTable dans l'editeur UE5 sans recompiler
- [x] **DATA-02**: Le jeu applique les stats de la DataTable armes au lancement (BeginPlay) — modifier la DataTable change le comportement en jeu
- [x] **DATA-03**: Les stats des ennemis (HP, damage, movementSpeed) sont editables via DataTable pour chaque type de zombie
- [x] **DATA-04**: Le jeu applique les stats de la DataTable ennemis au spawn — modifier la DataTable change les ennemis sans recompiler

### Audio & VFX (FX)

- [ ] **FX-06**: Chaque arme produit un son de tir distinct depuis un vrai asset audio (remplace les placeholders nullptrs)
- [ ] **FX-07**: Les zombies produisent des sons depuis vrais assets (grognement idle, son d'attaque, son de mort)
- [ ] **FX-08**: Un VFX muzzle flash Niagara est visible au canon de l'arme a chaque tir
- [ ] **FX-09**: Un VFX impact sang Niagara est visible la ou une balle touche un zombie
- [ ] **FX-10**: Un VFX explosion Niagara est visible quand ZombieExploder meurt

## v3.0 Requirements — Polish Visuel & Feel

### Arene & Map (MAP)

- [ ] **MAP-01** : L'arene dispose d'au moins 3 zones jouables distinctes avec des caracteristiques differentes (ouverte, couloirs, hauteur)
- [ ] **MAP-02** : L'eclairage de l'arene cree une atmosphere de tension (zones sombres, lumieres dynamiques, couleurs d'ambiance)
- [ ] **MAP-03** : Les SpawnPoints ennemis sont repartis intelligemment autour des zones de jeu (zombies emergent de plusieurs directions)

### Animations Armes (ANIM)

- [ ] **ANIM-01** : Chaque type d'arme a une pose de tenue distincte visible en third-person (pistolet tenu different d'un shotgun ou rifle)
- [ ] **ANIM-02** : Quand le chargeur est vide, une animation "bolt-open/slide-lock" se joue et l'arme reste bloquee jusqu'au rechargement
- [ ] **ANIM-03** : L'animation de rechargement est visible sur le personnage en third-person (geste de changement de chargeur)
- [ ] **ANIM-04** : Les transitions entre animations (idle → marche → tir → reload) sont fluides sans saut de pose

### Posture Personnage (POST)

- [ ] **POST-01** : La posture du personnage en combat est naturelle — les mains tiennent vraiment l'arme (IK mains sur l'arme)
- [ ] **POST-02** : La transition entre "desarme" et "arme" est fluide et visible (pas de pop instantane)

### HUD (HUD)

- [ ] **HUD-01** : La barre de vie affiche des changements visuels progressifs (couleur qui change selon les niveaux de vie)
- [ ] **HUD-02** : Les munitions (chargeur/reserve) sont affichees clairement avec distinction visuelle entre current/reserve
- [ ] **HUD-03** : Le multiplicateur de combo est mis en evidence quand il monte (animation ou effet visuel)

### Menu Principal (MENU)

- [ ] **MENU-01** : Le menu principal affiche un background atmospherique (image ou video en boucle) en accord avec l'univers du jeu
- [ ] **MENU-02** : La navigation dans les menus (titre → loadout → options) est fluide avec transitions et feedback visuel sur les boutons

## v4+ Requirements (Deferred)

### 2eme Mode de Jeu

- **MODE2-01**: Un deuxieme mode de jeu distinct du mode Mercenaires (survie infinie, chrono inverse, ou defense de zone)
- **MODE2-02**: Ecran de selection du mode de jeu depuis le menu principal

### PvP / Multijoueur

- **PVP-01**: Mode deathmatch humain vs humain
- **PVP-02**: Matchmaking online
- **PVP-03**: Arenes PvP dediees

## Out of Scope

| Feature | Reason |
|---------|--------|
| 2eme mode de jeu | Deferred v3.0 — Mode 1 doit etre fini en premier |
| Mode PvP | Milestone v3+, valider le PvE d'abord |
| Multijoueur online | Milestone v3+, complexite reseau |
| Coop | Milestone v3+ |
| Arenes supplementaires | Apres v2.0 |
| Save/progression | Arcade pur, pas de progression |
| Mobile | Desktop first |
| Accroupissement | Pas pertinent pour arcade action |
| Variant Combat melee | Reorientation du projet |
| Cinematiques | Pas pertinent pour arcade |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| ZOMB-01 | Phase 1 | Complete |
| ZOMB-02 | Phase 1 | Complete |
| ZOMB-03 | Phase 1 | Complete |
| ZOMB-04 | Phase 1 | Complete |
| ZOMB-05 | Phase 1 | Complete |
| ZOMB-06 | Phase 1 | Complete |
| ARME-01 | Phase 2 | Complete |
| ARME-02 | Phase 2 | Complete |
| ARME-03 | Phase 2 | Complete |
| ARME-04 | Phase 2 | Complete |
| ARME-05 | Phase 2 | Complete |
| ARME-06 | Phase 2 | Pending |
| SCORE-01 | Phase 3 | Pending |
| SCORE-02 | Phase 3 | Pending |
| SCORE-03 | Phase 3 | Pending |
| SCORE-04 | Phase 3 | Pending |
| SCORE-05 | Phase 3 | Pending |
| SCORE-06 | Phase 3 | Pending |
| AREN-01 | Phase 4 | Pending |
| AREN-02 | Phase 4 | Pending |
| AREN-03 | Phase 4 | Pending |
| AREN-04 | Phase 4 | Pending |
| AREN-05 | Phase 4 | Pending |
| LOAD-01 | Phase 5 | Pending |
| LOAD-02 | Phase 5 | Pending |
| LOAD-03 | Phase 5 | Pending |
| UI-01 | Phase 6 | Pending |
| UI-02 | Phase 6 | Pending |
| UI-03 | Phase 6 | Pending |
| UI-04 | Phase 6 | Pending |
| CAM-01 | Phase 7 | Pending |
| CAM-02 | Phase 7 | Pending |
| FX-01 | Phase 8 | Pending |
| FX-02 | Phase 8 | Pending |
| FX-03 | Phase 8 | Pending |
| FX-04 | Phase 8 | Pending |
| FX-05 | Phase 8 | Pending |
| DATA-01 | Phase 11 | Complete |
| DATA-02 | Phase 11 | Complete |
| DATA-03 | Phase 11 | Complete |
| DATA-04 | Phase 11 | Complete |
| ASSET-01 | Phase 12 | Pending |
| ASSET-02 | Phase 12 | Pending |
| FX-06 | Phase 13 | Pending |
| FX-07 | Phase 13 | Pending |
| FX-08 | Phase 13 | Pending |
| FX-09 | Phase 13 | Pending |
| FX-10 | Phase 13 | Pending |
| ANIM-01 | Phase 14 | Pending |
| ANIM-02 | Phase 14 | Pending |
| ANIM-03 | Phase 14 | Pending |
| ANIM-04 | Phase 14 | Pending |
| POST-01 | Phase 14 | Pending |
| POST-02 | Phase 14 | Pending |
| MAP-01 | Phase 15 | Pending |
| MAP-02 | Phase 15 | Pending |
| MAP-03 | Phase 15 | Pending |
| HUD-01 | Phase 16 | Pending |
| HUD-02 | Phase 16 | Pending |
| HUD-03 | Phase 16 | Pending |
| MENU-01 | Phase 16 | Pending |
| MENU-02 | Phase 16 | Pending |

**Coverage:**
- v1 requirements: 37 total
- Mapped to phases: 37
- v2.0 requirements: 11 total
- Mapped to phases: 11
- v3.0 requirements: 14 total
- Mapped to phases: 14
- Unmapped: 0

---
*Requirements defined: 2026-03-22*
*Last updated: 2026-04-06 — v3.0 requirements added (MAP-01/03, ANIM-01/04, POST-01/02, HUD-01/03, MENU-01/02) + traceability updated (phases 14-16)*
