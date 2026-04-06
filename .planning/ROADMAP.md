# Roadmap: RevenantOps v1.0 Demo Partageable

## Overview

En partant d'une base TPS fonctionnelle (locomotion, tir pistolet, HUD basique, wave spawner, IA ennemie), le projet construit couche par couche le mode Mercenaires complet : camera OTS serree, types de zombies varies, arsenal complet, systeme timer/combo, arene jouable, selection de loadout, menus complets, et polish audio/VFX. Chaque phase livre une capacite verifiable avant de passer a la suivante. La phase finale valide la demo comme experience partageable de bout en bout.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

- [x] **Phase 1: Zombies** - 5 types de zombies avec comportements distincts, waves progressives (completed 2026-03-22)
- [x] **Phase 2: Arsenal** - 5 armes a feu aux stats differenties + melee, switch en jeu (completed 2026-03-22)
- [ ] **Phase 3: Timer & Score** - Boucle Mercenaires : timer 5 min, score par kill, combo multiplicateur
- [ ] **Phase 4: Arene** - 1 arene jouable avec zones variees, spawners, pickups, ambiance sombre
- [ ] **Phase 5: Loadout** - Ecran de selection de 2 armes avant la partie, stats visibles
- [ ] **Phase 6: UI & Menus** - Ecran titre, HUD complet, ecran fin de partie, leaderboard local
- [ ] **Phase 7: Camera OTS** - Camera over-the-shoulder serree style RE4, zoom supplementaire en mode visee
- [ ] **Phase 8: Audio & VFX** - Sons par arme, impacts, zombies, musique, effets visuels
- [ ] **Phase 9: Integration** - Demo packagee et partageable, boucle complete validee sans bugs bloquants
- [ ] **Phase 10: Editor Setup & Playtest** - Configuration editeur UE5 (arena, widgets, PlayerController, audio) + validation partie complete

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
**Plans:** 4/4 plans complete

Plans:
- [x] 01-01-PLAN.md — ZombieBase + ZombieSlow + ZombieRunner (fondation + 2 types melee)
- [x] 01-02-PLAN.md — ZombieTank + ZombieExploder (tank HP eleve + explosif AoE)
- [x] 01-03-PLAN.md — ZombieSpitter + ZombieProjectile (zombie a distance + projectile)
- [x] 01-04-PLAN.md — Configuration 10 waves progressives + verification editeur

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
**Plans:** 3/3 plans complete

Plans:
- [x] 02-01-PLAN.md — Pistolet + Fusil d'Assaut + SMG (3 armes a feu + FireShot virtual)
- [x] 02-02-PLAN.md — Shotgun + Sniper (armes a comportement unique)
- [x] 02-03-PLAN.md — Arme de melee (sphere trace) + verification compilation

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
**Plans:** 1/1 plans complete

Plans:
- [x] 04-01-PLAN.md — Arena layout, spawn points, pickups, lighting (editor placement)

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
**Plans:** 1/1 plans complete

Plans:
- [x] 08-01-PLAN.md — Audio/VFX UPROPERTY hooks in WeaponBase, EnemyBase, ZombieBase, MercenairesGameState

### Phase 9: Integration
**Goal**: La demo est packagee, jouable de bout en bout sans bugs bloquants, comprehensible par un joueur externe sans instruction
**Depends on**: Phase 8
**Requirements**: (cross-cutting — valide l'ensemble des 37 requirements en conditions reelles)
**Success Criteria** (what must be TRUE):
  1. Un joueur externe peut lancer la demo, choisir son loadout, jouer une partie complete et voir son score sans explication prealable
  2. La boucle titre -> loadout -> partie -> fin de partie -> leaderboard s'execute sans crash ni blocage
  3. Le jeu est packageable (Build Development ou Shipping) et s'execute sans l'editeur UE5 ouvert
  4. Aucun bug bloquant (freeze, crash, soft lock) ne survient sur une session de jeu complete
**Plans:** 1/1 plans complete

Plans:
- [x] 09-01-PLAN.md — Game flow in PlayerController (title→loadout→match→gameover→leaderboard)

### Phase 10: Editor Setup & Playtest
**Goal**: L'arene est jouable, les widgets sont fonctionnels, le PlayerController est configure, les assets audio sont en place, et une partie complete se deroule sans blocage
**Depends on**: Phase 9
**Requirements**: AREN-01 a AREN-05 (completion), UI-01 a UI-04 (completion), FX-01 a FX-05 (placeholders)
**Success Criteria** (what must be TRUE):
  1. Les zombies spawn depuis 4-6 points repartis dans l'arene et naviguent via NavMesh
  2. Les 4 widgets (Title/Loadout/GameOver/Leaderboard) affichent leurs elements et repondent aux clics
  3. Le PlayerController enchaine le flow title→loadout→match→gameover→leaderboard sans erreur
  4. Des sons placeholder jouent au tir, au reload, et a la mort des zombies
  5. Une partie complete de 5 minutes se deroule du debut a la fin sans crash ni softlock
**Plans**: Complete (2026-03-29)

Plans:
- [x] 10-01-PLAN.md — BLACKSITE level + WaveSpawner config + PIE validation

## Progress

**Execution Order:**
Phases executent dans l'ordre numerique : 1 -> 2 -> 3 -> 4 -> 5 -> 6 -> 7 -> 8 -> 9 -> 10

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Zombies | 4/4 | Complete   | 2026-03-22 |
| 2. Arsenal | 3/3 | Complete   | 2026-03-22 |
| 3. Timer & Score | 1/1 | Complete   | 2026-03-23 |
| 4. Arene | 1/1 | Complete   | 2026-03-23 |
| 5. Loadout | 1/1 | Complete   | 2026-03-23 |
| 6. UI & Menus | 1/1 | Complete   | 2026-03-23 |
| 7. Camera OTS | 1/1 | Complete   | 2026-03-23 |
| 8. Audio & VFX | 1/1 | Complete   | 2026-03-23 |
| 9. Integration | 1/1 | Complete   | 2026-03-23 |
| 10. Editor Setup & Playtest | 1/1 | Complete | 2026-03-29 |

---

# Roadmap: RevenantOps v2.0 Finition Mode Mercenaires

## Overview

A partir d'une demo v1.0 validee en PIE (boucle complete, 6 armes, 5 zombies, BLACKSITE), v2.0 finalise le mode Mercenaires avec de vrais assets visuels et sonores, et des systemes de donnees editables sans recompiler. Les trois phases livrent la fondation data-driven d'abord (DataTables), puis les assets visuels armes, puis le polish audio et VFX Niagara — dans l'ordre qui minimise les reprises.

## Phases

- [x] **Phase 11: DataTables** - Stats armes et ennemis editables dans l'editeur UE5 via FTableRowBase, appliquees au runtime sans recompiler (completed 2026-03-29)
- [ ] **Phase 12: Assets Armes** - Les 6 armes ont de vrais meshes 3D depuis Fab.com, correctement positionnes sur hand_r
- [ ] **Phase 13: Audio & VFX Niagara** - Sons de tir et zombies depuis vrais assets audio, VFX muzzle flash / impact sang / explosion via Niagara

## Phase Details

### Phase 11: DataTables
**Goal**: Les stats des armes et des ennemis sont editables via DataTable dans l'editeur UE5 et s'appliquent au runtime sans recompiler
**Depends on**: Phase 10 (v1.0 complete)
**Requirements**: DATA-01, DATA-02, DATA-03, DATA-04
**Success Criteria** (what must be TRUE):
  1. Une DataTable DT_WeaponStats existe dans l'editeur avec une ligne par arme (pistolet, fusil, SMG, shotgun, sniper, melee) exposant damage, fireRate, ammo, range
  2. Modifier une valeur dans DT_WeaponStats et lancer PIE change le comportement de l'arme en jeu (ex : augmenter damage du pistolet = kills plus rapides) sans recompiler
  3. Une DataTable DT_EnemyStats existe avec une ligne par type de zombie (Slow, Runner, Tank, Spitter, Exploder) exposant HP, damage, movementSpeed
  4. Modifier une valeur dans DT_EnemyStats et spawner l'ennemi correspondant en PIE reflète la nouvelle stat (ex : augmenter HP du Tank = plus de balles necessaires)
**Plans**: TBD

### Phase 12: Assets Armes
**Goal**: Les 6 armes affichent de vrais meshes 3D visibles dans la main du joueur, correctement orientes sur le socket hand_r
**Depends on**: Phase 11
**Requirements**: ASSET-01, ASSET-02
**Success Criteria** (what must be TRUE):
  1. Chacune des 6 armes (pistolet, fusil, SMG, shotgun, sniper, melee) affiche un vrai mesh 3D importe depuis Fab.com — plus de SM_ChamferCube ni de mesh vide
  2. En PIE, chaque arme selectionnee au loadout apparait correctement positionnee dans la main droite du joueur (pas de rotation absurde, pas de flottement)
  3. Le switch d'arme en jeu met a jour le mesh visible immediatement sans artefact
**Plans**: TBD

### Phase 13: Audio & VFX Niagara
**Goal**: Le jeu produit des sons de tir distincts par arme, des sons zombies realistes, et des VFX Niagara visibles au tir, a l'impact et a l'explosion
**Depends on**: Phase 12
**Requirements**: FX-06, FX-07, FX-08, FX-09, FX-10
**Success Criteria** (what must be TRUE):
  1. Chaque arme produit un son de tir audiblement different (le shotgun sonne differemment du pistolet) depuis un vrai asset audio assigne dans le BP
  2. Les zombies emettent des sons depuis vrais assets : un grognement idle distinct, un son d'attaque et un son de mort audibles en PIE
  3. Un VFX Niagara de muzzle flash s'affiche au bout du canon a chaque tir (visible en PIE, disparait apres l'effet)
  4. Un VFX Niagara d'impact sang s'affiche sur le zombie au point d'impact quand une balle le touche
  5. Un VFX Niagara d'explosion s'affiche a la mort d'un ZombieExploder (remplacement ou complement de l'explosion de degats existante)
**Plans**: TBD

## Progress

**Execution Order:**
Phases executent dans l'ordre numerique : 11 -> 12 -> 13

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 11. DataTables | 1/1 | Complete   | 2026-03-29 |
| 12. Assets Armes | 0/? | Not started | — |
| 13. Audio & VFX Niagara | 0/? | Not started | — |

---

# Roadmap: RevenantOps v3.0 Polish Visuel & Feel

## Overview

A partir d'un mode Mercenaires fonctionnel avec vrais assets (v2.0), v3.0 ameliore le ressenti global du jeu sur trois fronts independants : les animations et la posture du personnage en combat (ABP_Mercenaire), l'arene retravaillee avec zones distinctes et eclairage atmospherique, et le polish visuel de l'interface (HUD + menus). Les phases sont ordonnees pour livrer le feedback de jeu en premier (animations/posture), puis l'environnement de combat (arene), puis l'enveloppe visuelle (UI).

## Phases

- [ ] **Phase 14: Animations & Posture** - Poses de tenue distinctes par type d'arme, animation "chargeur vide", IK mains sur l'arme, transition arme/desarme fluide
- [ ] **Phase 15: Arene Retravaillee** - 3 zones jouables distinctes, eclairage atmospherique, SpawnPoints repartis intelligemment
- [ ] **Phase 16: HUD & Menus Polish** - Barre de vie progressive, munitions lisibles, combo mis en evidence, menu principal avec background atmospherique et navigation fluide

## Phase Details

### Phase 14: Animations & Posture
**Goal**: Le personnage tient son arme de maniere credible et ses animations de combat sont fluides et distinctes par type d'arme
**Depends on**: Phase 13 (v2.0 complete — montages et ABP_Mercenaire existants)
**Requirements**: ANIM-01, ANIM-02, ANIM-03, ANIM-04, POST-01, POST-02
**Success Criteria** (what must be TRUE):
  1. En PIE, tenir un pistolet produit une pose de bras observablement differente de tenir un fusil d'assaut ou un shotgun
  2. Quand le chargeur est vide, une animation "slide-lock" ou "bolt-open" se joue et l'arme reste bloquee visuellement jusqu'au rechargement
  3. L'animation de rechargement est visible sur le personnage en third-person — on voit le geste de changement de chargeur
  4. Les transitions entre idle, marche, tir et rechargement se font sans saut brusque de pose (aucun "pop" visible)
  5. Les mains du personnage semblent tenir reellement l'arme — pas de mains flottantes ni de penetration dans le mesh arme
**Plans**: TBD

### Phase 15: Arene Retravaillee
**Goal**: L'arene BLACKSITE offre 3 zones jouables avec des caracteristiques distinctes, un eclairage qui cree de la tension, et des points de spawn ennemis couvrant l'ensemble de la map
**Depends on**: Phase 14
**Requirements**: MAP-01, MAP-02, MAP-03
**Success Criteria** (what must be TRUE):
  1. En se deplacant dans l'arene, le joueur traverse au moins 3 zones visuellement et geometriquement distinctes (ex : cour ouverte, couloir etroit, mezzanine en hauteur)
  2. L'eclairage de chaque zone cree une ambiance tendue — zones d'ombre, lumieres colorees, contraste fort entre zones sombres et eclairees
  3. Des zombies emergent de directions variees au cours d'une vague — aucune direction n'est jamais la seule source de menace
**Plans**: TBD

### Phase 16: HUD & Menus Polish
**Goal**: L'interface du jeu est lisible, moderne et coherente avec l'univers — la barre de vie communique l'urgence, les munitions sont immediatement lisibles, le combo est mis en evidence, et le menu principal est visuellement attrayant
**Depends on**: Phase 14
**Requirements**: HUD-01, HUD-02, HUD-03, MENU-01, MENU-02
**Success Criteria** (what must be TRUE):
  1. La barre de vie change de couleur de facon visible selon le niveau de vie (ex : vert → jaune → rouge) — un joueur comprend son etat de sante d'un coup d'oeil
  2. Les munitions en chargeur et en reserve sont distinguees visuellement (pas deux chiffres identiques cote a cote) — le joueur sait immediatement combien il lui reste
  3. Quand le multiplicateur de combo monte, il est mis en evidence a l'ecran (agrandissement, flash, couleur vive) — perceptible en combat sans regarder le HUD
  4. L'ecran titre affiche un background atmospherique en accord avec l'univers (image de l'arene, zombies, palette sombre) — pas un fond noir uni
  5. Naviguer de l'ecran titre vers le loadout et retour se fait avec un retour visuel sur les boutons (hover, click) et une transition fluide entre ecrans
**Plans**: TBD

## Progress

**Execution Order:**
Phases executent dans l'ordre numerique : 14 -> 15 -> 16
(Phase 16 peut commencer en parallele de Phase 15 — dependance sur Phase 14 uniquement)

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 14. Animations & Posture | 0/? | Not started | — |
| 15. Arene Retravaillee | 0/? | Not started | — |
| 16. HUD & Menus Polish | 0/? | Not started | — |

---
*Roadmap created: 2026-03-22 -- v1.0 Demo Partageable (Mercenaires mode)*
*Last updated: 2026-04-06 -- v3.0 phases 14-16 added (Polish Visuel & Feel)*
