# RevenantOps

## What This Is

RevenantOps est un TPS (Third-Person Shooter) multivariante sous Unreal Engine 5.7, ecrit en C++ pur. Le projet comprend un mode TPS principal (tir, locomotion, IA ennemie, wave spawner, HUD, save system) et un mode Combat melee (combo, charged attacks, StateTree AI). Developpe en solo par Noureddine.

## Core Value

Le joueur peut incarner un personnage TPS complet avec tir, esquive, et affronter des vagues d'ennemis IA dans un environnement reactif.

## Current Milestone: v2.0 Editor Integration

**Goal:** Configurer tout ce qui manque cote editeur UE5 pour rendre le jeu jouable — le code C++ est ecrit mais jamais teste en jeu.

**Target features:**
- Input Actions et Input Mapping Contexts configures
- Blueprints lies aux classes C++ (Character, Weapons, Enemies, GameMode, PlayerController)
- HUD Widget et LifeBar Widgets fonctionnels
- Variant Combat entierement configure (Blueprints, StateTree IA, Montages)
- Integration testee et jouable

## Requirements

### Validated

- Locomotion TPS (sprint, crouch, dodge) — v1.0 Phase 1.1
- Systeme de tir complet & armes — v1.0 Phase 2.2
- IA ennemie TPS (EnemyBase) — v1.0 Phase 3.1
- HUD systeme C++ (vie, stamina, munitions, crosshair) — v1.0 Phase 7.1
- Wave Spawner, Interactables, Destructibles, Pickups — v1.0 Phase 8
- Health Component reutilisable avec shield — v1.0
- Save System & Checkpoints — v1.0 Phase 10.1

### Active

- [ ] Input Actions et Mapping Contexts pour TPS et Combat
- [ ] Blueprints Character/Weapon/Enemy/GameMode/PlayerController
- [ ] HUD Widget UMG avec bindings
- [ ] Variant Combat Blueprints et configuration
- [ ] StateTree IA Combat
- [ ] Animation Montages references
- [ ] Integration et test en jeu

### Out of Scope

- Variant Platforming — mode different, milestone futur
- Variant SideScrolling — mode different, milestone futur
- Mobile controls widget — pas prioritaire
- Level design avance — milestone futur
- Audio/VFX finaux — placeholder suffisants pour v2.0
- Multiplayer — pas prevu

## Context

- Unreal Engine 5.7 avec Enhanced Input et Python Editor Script Plugin
- Plugin unreal-mcp connecte : Claude peut piloter l'editeur UE5 via 64 outils MCP
- Tout le C++ compile mais n'a jamais ete teste en jeu
- Les classes C++ sont abstract/Blueprintable — necessitent des Blueprints enfants
- Le HUD utilise BindWidgetOptional — les noms des sous-widgets doivent correspondre exactement

## Constraints

- **Tech stack**: UE 5.7, C++ pur, Enhanced Input System
- **Solo dev**: Noureddine est le seul developpeur
- **MCP limitations**: Pas de screenshot UE5, pas de playtesting automatise, pas d'import d'assets externes
- **Assets**: Utiliser les assets Mannequin/placeholder UE5 par defaut

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| C++ pur, pas de Blueprint logic | Performance et controle total | -- Pending |
| Enhanced Input System | Standard UE5.7, flexible | -- Pending |
| MCP pour pilotage editeur | Autonomie Claude sans intervention manuelle | -- Pending |
| StateTree pour IA Combat | Plus moderne que Behavior Trees dans UE5 | -- Pending |

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
*Last updated: 2026-03-22 after milestone v2.0 initialization*
