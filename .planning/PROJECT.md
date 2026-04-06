# RevenantOps

## What This Is

RevenantOps est un TPS arcade score-attack inspire du mode Mercenaires de Resident Evil, sous Unreal Engine 5.7 en C++ pur. Le joueur affronte des vagues de zombies dans une arene en temps limite, enchaine des combos pour maximiser son score, et choisit son loadout avant chaque partie. Camera over-the-shoulder serree style RE4. Developpe en solo par Noureddine.

## Core Value

Le joueur peut lancer une partie, choisir son loadout, tuer des zombies en enchainant des combos sous pression du timer, et voir son score final.

## Current Milestone: v3.0 Polish Visuel & Feel

**Goal:** Ameliorer le ressenti global du jeu — arene plus immersive, animations armes realistes par type d'arme, posture personnage naturelle, HUD lisible et moderne, menu principal attrayant.

**Target features:**
- Arene retravaillee : layout plus dynamique, zones distinctes, eclairage atmospherique
- Animations armes : pose de tenue specifique par type (pistolet vs shotgun vs rifle), anim chargeur vide, anim rechargement visible
- Posture personnage : transition naturelle entre decontracte et arme, IK mains propre
- HUD ameliore : barre de vie progressive, indicateurs de munitions clairs, combo visible
- Menu principal retravaille : background atmospherique, navigation fluide, presentation soignee

## Previous Milestone: v2.0 Finition Mode Mercenaires (COMPLETE — 2026-04-05)

**Delivered:** DataTables armes+ennemis (DT_WeaponStats, DT_EnemyStats), 6 vrais meshes armes Kenney, sons de tir/rechargement/impact, VFX Niagara (muzzle flash, impact, blood), animated montages, armed locomotion avec BS_Pistol_Walk_Run, HitFlash emissive.

## Previous Milestone: v1.0 Demo Partageable (COMPLETE — 2026-03-29)

**Goal:** Produire une demo jouable et partageable du mode Mercenaires — une boucle de gameplay complete qu'un joueur externe peut comprendre et apprecier.

**Delivered:** 1 arene BLACKSITE, 5 types zombies, 6 armes + melee, timer/score/combo, loadout selection, ecran titre, HUD, GameOver, Leaderboard — PIE valide.

## Requirements

### Validated

- Locomotion TPS (sprint, mouvement, saut) — v0 existant
- Systeme de tir complet (pistolet fonctionne en jeu) — v0 existant
- HUD basique (vie, shield, munitions) — v0 existant
- Wave Spawner C++ — v0 existant
- IA ennemie de base (EnemyBase) — v0 existant
- Health Component avec shield — v0 existant
- Enhanced Input System configure (18 Input Actions, 5 IMC) — v0 existant
- Blueprints TPS configures (Character, GameMode, PlayerController) — v0 existant

### Validated (v1.0 — 2026-03-29)

- [x] Camera over-the-shoulder serree (style RE4)
- [x] Systeme de timer (5 min, bonus de temps sur la map)
- [x] Systeme de score avec multiplicateur de combo
- [x] 4-5 types de zombies (lent, rapide, tank, cracheur, explosif)
- [x] 4-5 armes (pistolet, fusil, shotgun, sniper, SMG) + melee
- [x] Selection de loadout avant la partie
- [x] 1 arene de jeu complete (BLACKSITE — level design, spawners, pickups)
- [x] Ecran titre / menu principal
- [x] Ecran de fin de partie (score, stats, rejouer)
- [x] Leaderboard local (top scores)
- [x] Boucle complete title→loadout→match→gameover→leaderboard validee en PIE

### Validated (v2.0 — 2026-04-05)

- [x] Vrais meshes armes depuis Fab.com (6 armes : pistolet, fusil, SMG, shotgun, sniper, melee)
- [x] DataTable armes : stats editables sans recompiler (damage, fireRate, ammo, range)
- [x] DataTable ennemis : HP, damage, speed editables par type
- [x] Sons reels au tir par arme (remplacer placeholders)
- [x] Sons reels zombies (grognement, attaque, mort)
- [x] VFX : muzzle flash, impact sang, explosion ZombieExploder
- [x] AnimMontages armes (Fire/Reload/Equip) + armed locomotion avec BS_Pistol_Walk_Run
- [x] HitFlash emissive sur ennemis (feedback visuel de hit)
- [x] Camera shake au tir et a la reception de degats

### Active (v3.0)

- [ ] Arene retravaillee avec zones distinctes et eclairage atmospherique
- [ ] Animations de tenue d'arme distinctes par type (pistolet, rifle, shotgun)
- [ ] Animation "chargeur vide" (arme se bloque, anim specifique)
- [ ] Animation de rechargement visible en third-person
- [ ] Posture personnage naturelle en combat (IK mains, transition arme/desarme fluide)
- [ ] HUD v3 : barre de vie, munitions lisibles, combo dynamique
- [ ] Menu principal retravaille (background, navigation, presentation)

### Out of Scope

- Mode PvP / Affrontement — milestone v3+
- Multijoueur online — milestone v3+
- 2eme mode de jeu — deferred to v3.0 (Mode 1 doit etre fini en premier)
- Arenes supplementaires — apres validation de la premiere
- Variant Combat melee — reorientation du projet, plus pertinent
- Variant Platforming — reorientation du projet
- Variant SideScrolling — reorientation du projet
- Save system / progression persistante — v1 est arcade, pas de progression
- Mobile — desktop first
- Accroupissement — pas pertinent pour l'arcade action

## Context

- UE 5.7 avec Enhanced Input et Python Editor Script Plugin
- Plugin unreal-mcp connecte : Claude pilote l'editeur UE5 via 64 outils MCP + socket TCP port 12029
- Code C++ existant compile et teste en PIE : locomotion, tir, HUD, wave spawner fonctionnels
- Le code des Variants (Combat, Platforming, SideScrolling) existe mais n'est plus dans le scope — elements reutilisables a evaluer
- Les Blueprints TPS sont configures et fonctionnels (BP_ThirdPersonCharacter, BP_ThirdPersonGameMode, BP_ThirdPersonPlayerController)
- PIE Play bloque la communication MCP TCP — configuration en mode Editor, test par l'utilisateur
- Ne jamais changer de niveau via MCP (cause crash)
- Assets placeholder Mannequin UE5 utilises, pas d'assets custom

## Constraints

- **Tech stack**: UE 5.7, C++ pur, Enhanced Input System
- **Solo dev**: Noureddine est le seul developpeur
- **MCP limitations**: PIE Play bloque TCP, pas de changement de niveau via MCP, screenshots limites
- **Assets**: Mannequin/placeholder UE5 pour la v1, assets custom en v2
- **Objectif**: Demo partageable — doit etre comprehensible par un joueur externe

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| Mode Mercenaires RE4 comme reference | Gameplay loop prouve, arcade, score-attack | -- Pending |
| Camera OTS serree | Immersion, tension, style RE4 | -- Pending |
| Timer fixe 5 min + bonus temps | Balance entre pression et recompense | -- Pending |
| Combo multiplicateur | Encourage le jeu agressif et la prise de risque | -- Pending |
| PvE d'abord, PvP plus tard | Reduire la complexite, valider le core gameplay solo | -- Pending |
| 1 seule arene pour la v1 | Focus sur la qualite du gameplay, pas la quantite de contenu | -- Pending |
| Loadout pre-partie | Strategie et rejouabilite sans complexite in-game | -- Pending |
| C++ pur, pas de Blueprint logic | Performance et controle total | -- Pending |

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd:transition`):
1. Requirements invalidated? -> Move to Out of Scope with reason
2. Requirements validated? -> Move to Validated with phase reference
3. New requirements emerged? -> Add to Active
4. Decisions to log? -> Add to Key Decisions
5. "What This Is" still accurate? -> Update if drifted

**After each milestone** (via `/gsd:complete-milestone`):
1. Full review of all sections
2. Core Value check -- still the right priority?
3. Audit Out of Scope -- reasons still valid?
4. Update Context with current state

---
*Last updated: 2026-04-06 — v2.0 complete, v3.0 started (Polish Visuel & Feel)*
