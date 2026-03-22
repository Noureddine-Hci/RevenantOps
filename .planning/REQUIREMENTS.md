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
- [ ] **ZOMB-03**: Zombie tank (gros costaud, beaucoup de vie, degats lourds, lent)
- [ ] **ZOMB-04**: Zombie cracheur (attaque a distance, projectile, vie moyenne)
- [ ] **ZOMB-05**: Zombie explosif (explose a proximite, degats de zone, meurt en explosant)
- [ ] **ZOMB-06**: Waves progressives (difficulte croissante, plus de zombies, mix de types)

### Armes

- [ ] **ARME-01**: Pistolet (semi-auto, degats moyens, munitions abondantes)
- [ ] **ARME-02**: Fusil d'assaut (full-auto, degats faibles, cadence rapide)
- [ ] **ARME-03**: Shotgun (degats massifs proches, spread, rechargement lent)
- [ ] **ARME-04**: Sniper (degats enormes, lent, zoom)
- [ ] **ARME-05**: SMG (cadence tres rapide, degats faibles, grande capacite)
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
- [ ] **UI-03**: Ecran de fin de partie (score final, kills, meilleur combo, rejouer)
- [ ] **UI-04**: Leaderboard local (top 10 scores)

### Audio & VFX

- [ ] **FX-01**: Sons de tir pour chaque arme
- [ ] **FX-02**: Sons d'impact (balle sur zombie, melee)
- [ ] **FX-03**: Sons de zombie (grognements, attaque, mort)
- [ ] **FX-04**: Musique d'ambiance tension/action
- [ ] **FX-05**: VFX de base (muzzle flash, impact sang, explosion)

## v2 Requirements

### PvP Mode

- **PVP-01**: Mode deathmatch humain vs humain
- **PVP-02**: Matchmaking online
- **PVP-03**: Arenes PvP dediees

### Contenu

- **CONT-01**: Arenes supplementaires (3+)
- **CONT-02**: Types de zombies additionnels
- **CONT-03**: Armes supplementaires
- **CONT-04**: Personnages jouables avec stats differentes

### Progression

- **PROG-01**: Systeme de progression persistant (XP, niveaux)
- **PROG-02**: Deblocage d'armes et equipements
- **PROG-03**: Leaderboard online

## Out of Scope

| Feature | Reason |
|---------|--------|
| Mode PvP | Milestone v2, valider le PvE d'abord |
| Multijoueur online | Milestone v2, complexite reseau |
| Coop | Milestone v2 |
| Arenes multiples | Apres validation de la premiere |
| Save/progression | v1 est arcade pur, pas de progression |
| Mobile | Desktop first |
| Accroupissement | Pas pertinent pour arcade action |
| Variant Combat melee | Reorientation du projet |
| Variant Platforming | Reorientation du projet |
| Variant SideScrolling | Reorientation du projet |
| Assets custom (modeles 3D) | Mannequin placeholder pour v1 |
| Cinematiques | Pas pertinent pour arcade |

## Traceability

| Requirement | Phase | Status |
|-------------|-------|--------|
| ZOMB-01 | Phase 1 | Complete |
| ZOMB-02 | Phase 1 | Complete |
| ZOMB-03 | Phase 1 | Pending |
| ZOMB-04 | Phase 1 | Pending |
| ZOMB-05 | Phase 1 | Pending |
| ZOMB-06 | Phase 1 | Pending |
| ARME-01 | Phase 2 | Pending |
| ARME-02 | Phase 2 | Pending |
| ARME-03 | Phase 2 | Pending |
| ARME-04 | Phase 2 | Pending |
| ARME-05 | Phase 2 | Pending |
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

**Coverage:**
- v1 requirements: 37 total
- Mapped to phases: 37
- Unmapped: 0

---
*Requirements defined: 2026-03-22*
*Last updated: 2026-03-22 — traceability complete, all 37 requirements mapped to phases 1-8*
