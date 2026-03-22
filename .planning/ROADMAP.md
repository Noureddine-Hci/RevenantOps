# Roadmap: RevenantOps v1.0 Demo Partageable

## Overview

En partant d'une base TPS fonctionnelle (locomotion, tir pistolet, HUD basique, wave spawner, IA ennemie), le projet construit couche par couche le mode Mercenaires complet : camera OTS serree, types de zombies varies, arsenal complet, systeme timer/combo, arene jouable, selection de loadout, menus complets, et polish audio/VFX. Chaque phase livre une capacite verifiable avant de passer a la suivante. La phase finale valide la demo comme experience partageable de bout en bout.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

- [ ] **Phase 1: Zombies** - 5 types de zombies avec comportements distincts, waves progressives
- [ ] **Phase 2: Arsenal** - 5 armes a feu aux stats differenties + melee, switch en jeu
- [ ] **Phase 3: Timer & Score** - Boucle Mercenaires : timer 5 min, score par kill, combo multiplicateur
- [ ] **Phase 4: Arene** - 1 arene jouable avec zones variees, spawners, pickups, ambiance sombre
- [ ] **Phase 5: Loadout** - Ecran de selection de 2 armes avant la partie, stats visibles
- [ ] **Phase 6: UI & Menus** - Ecran titre, HUD complet, ecran fin de partie, leaderboard local
- [ ] **Phase 7: Camera OTS** - Camera over-the-shoulder serree style RE4, zoom supplementaire en mode visee
- [ ] **Phase 8: Audio & VFX** - Sons par arme, impacts, zombies, musique, effets visuels
- [ ] **Phase 9: Integration** - Demo packagee et partageable, boucle complete validee sans bugs bloquants

## Phase Details

### Phase 1: Zombies
**Goal**: Le jeu contient 5 types de zombies avec comportements distincts et des waves progressivement plus difficiles
**Depends on**: Nothing (first phase)
**Requirements**: ZOMB-01, ZOMB-02, ZOMB-03, ZOMB-04, ZOMB-05, ZOMB-06
**Success Criteria** (what must be TRUE):
  1. Chaque type de zombie a un comportement observable distinct : lent en horde, coureur, tank lent, cracheur a distance, explosif sur proximite
  2. Les zombies infligent des degats variables selon leur type (le tank fait plus de degats que le lent)
  3. Les waves successives augmentent le nombre de zombies et melangent plusieurs types dans une meme vague
  4. Un zombie explosif cause des degats de zone au joueur s'il s'approche trop
  5. Le wave spawner fait spawn le bon type de zombie selon la configuration de la vague
**Plans:** 4 plans

Plans:
- [ ] 01-01-PLAN.md — ZombieBase + ZombieSlow + ZombieRunner (fondation + 2 types melee)
- [ ] 01-02-PLAN.md — ZombieTank + ZombieExploder (tank HP eleve + explosif AoE)
- [ ] 01-03-PLAN.md — ZombieSpitter + ZombieProjectile (zombie a distance + projectile)
- [ ] 01-04-PLAN.md — Configuration 10 waves progressives + verification editeur

### Phase 2: Arsenal
**Goal**: Le joueur dispose de 5 armes a feu avec comportements distincts et d'une arme de melee, switchables en jeu
**Depends on**: Phase 1
**Requirements**: ARME-01, ARME-02, ARME-03, ARME-04, ARME-05, ARME-06
**Success Criteria** (what must be TRUE):
  1. Chaque arme a un comportement de tir observablement different : semi-auto, full-auto, spread shotgun, lent+zoom sniper, cadence SMG
  2. Le joueur peut switcher entre ses armes equipees en jeu (touche ou molette)
  3. Le rechargement de chaque arme est fonctionnel avec une duree propre a l'arme
  4. L'arme de melee attaque au corps-a-corps sans consommer de munitions
  5. Les degats infliges sur un zombie varient visiblement selon l'arme (le shotgun a bout portant tue plus vite que le pistolet)
**Plans**: TBD

### Phase 3: Timer & Score
**Goal**: La boucle Mercenaires est active : timer 5 minutes, score par kill avec combo multiplicateur, fin de partie sur zero
**Depends on**: Phase 1
**Requirements**: SCORE-01, SCORE-02, SCORE-03, SCORE-04, SCORE-05, SCORE-06
**Success Criteria** (what must be TRUE):
  1. Un timer de 5 minutes decompte a l'ecran et la partie se termine automatiquement quand il atteint zero
  2. Chaque kill ajoute des points au score en fonction du type de zombie tue
  3. Enchainer des kills rapidement monte le multiplicateur de combo (x2, x3, x4...) affiche a l'ecran
  4. Le multiplicateur de combo se reinitialise apres quelques secondes sans kill, avec un timer de combo visible
  5. Ramasser un pickup bonus temps ajoute du temps au timer (+30s ou +15s selon le type)
**Plans**: TBD

### Phase 4: Arene
**Goal**: Une arene jouable complete offre des zones variees, des spawners positionnes, des pickups accessibles et une ambiance sombre
**Depends on**: Phase 3
**Requirements**: AREN-01, AREN-02, AREN-03, AREN-04, AREN-05
**Success Criteria** (what must be TRUE):
  1. L'arene offre au moins trois zones distinctes de jeu : une zone ouverte, un couloir etroit et un espace en hauteur
  2. Les zombies emergent depuis des points de spawn repartis sur l'ensemble de la carte (pas concentres au meme endroit)
  3. Des pickups de bonus temps et de munitions sont accessibles dans l'arene et disparaissent a la ramasse
  4. L'eclairage et l'atmosphere generale creent une tension visuelle coherente avec le mode Mercenaires
**Plans**: TBD

### Phase 5: Loadout
**Goal**: Le joueur choisit son equipement avant la partie via un ecran de selection avec les stats affichees
**Depends on**: Phase 2
**Requirements**: LOAD-01, LOAD-02, LOAD-03
**Success Criteria** (what must be TRUE):
  1. Un ecran de selection de loadout apparait avant le lancement de la partie
  2. Le joueur choisit 2 armes a feu parmi les 5 disponibles (l'arme de melee est toujours equipee)
  3. Les stats de chaque arme (degats, cadence, portee) sont affichees dans l'ecran de selection
  4. La partie demarre avec exactement les armes choisies dans l'inventaire du joueur
**Plans**: TBD

### Phase 6: UI & Menus
**Goal**: La boucle de jeu est encadree par un ecran titre, un HUD complet en jeu, un ecran de fin et un leaderboard local
**Depends on**: Phase 3, Phase 5
**Requirements**: UI-01, UI-02, UI-03, UI-04
**Success Criteria** (what must be TRUE):
  1. L'ecran titre affiche les options "Jouer" et "Quitter" fonctionnelles
  2. Le HUD en jeu affiche simultanement : vie, shield, arme active, munitions, timer, score et multiplicateur de combo
  3. L'ecran de fin affiche le score final, le nombre de kills, le meilleur combo atteint et une option rejouer
  4. Le leaderboard local affiche et persiste les 10 meilleurs scores entre les sessions de jeu
**Plans**: TBD

### Phase 7: Camera OTS
**Goal**: Le joueur joue avec une camera over-the-shoulder serree style RE4, avec zoom supplementaire en mode visee
**Depends on**: Phase 6
**Requirements**: CAM-01, CAM-02
**Success Criteria** (what must be TRUE):
  1. La camera se positionne sur l'epaule droite du personnage a courte distance (style RE4, pas la camera TPS par defaut)
  2. En mode visee (clic droit), la camera zoome davantage et l'offset lateral se resserre
  3. La camera suit les rotations du personnage sans saccade ni artefact visuel
  4. Le retour a la vue normale apres avoir relache la visee est fluide et immediat
**Plans**: TBD

### Phase 8: Audio & VFX
**Goal**: Le jeu dispose de sons distincts par arme, de feedback sonore sur les impacts et zombies, d'une musique de fond et de VFX de base
**Depends on**: Phase 7
**Requirements**: FX-01, FX-02, FX-03, FX-04, FX-05
**Success Criteria** (what must be TRUE):
  1. Chaque arme produit un son de tir distinct et audible
  2. Les impacts (balle sur zombie, attaque melee) produisent un retour sonore
  3. Les zombies emettent des sons de grognement, d'attaque et de mort
  4. Une musique d'ambiance tension/action joue en continu pendant la partie
  5. Des VFX de base sont visibles : muzzle flash au tir, impact sang sur zombie, explosion du zombie explosif
**Plans**: TBD

### Phase 9: Integration
**Goal**: La demo est packagee, jouable de bout en bout sans bugs bloquants, comprehensible par un joueur externe sans instruction
**Depends on**: Phase 8
**Requirements**: (cross-cutting — valide l'ensemble des 37 requirements en conditions reelles)
**Success Criteria** (what must be TRUE):
  1. Un joueur externe peut lancer la demo, choisir son loadout, jouer une partie complete et voir son score sans explication prealable
  2. La boucle titre -> loadout -> partie -> fin de partie -> leaderboard s'execute sans crash ni blocage
  3. Le jeu est packageable (Build Development ou Shipping) et s'execute sans l'editeur UE5 ouvert
  4. Aucun bug bloquant (freeze, crash, soft lock) ne survient sur une session de jeu complete
**Plans**: TBD

## Progress

**Execution Order:**
Phases executent dans l'ordre numerique : 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Zombies | 0/4 | Planning complete | - |
| 2. Arsenal | 0/TBD | Not started | - |
| 3. Timer & Score | 0/TBD | Not started | - |
| 4. Arene | 0/TBD | Not started | - |
| 5. Loadout | 0/TBD | Not started | - |
| 6. UI & Menus | 0/TBD | Not started | - |
| 7. Camera OTS | 0/TBD | Not started | - |
| 8. Audio & VFX | 0/TBD | Not started | - |
| 9. Integration | 0/TBD | Not started | - |

---
*Roadmap created: 2026-03-22 -- v1.0 Demo Partageable (Mercenaires mode)*
*Last updated: 2026-03-22*
