# RevenantOps — Primer (etat courant)
> Ce fichier est mis a jour en debut et fin de chaque session. Il sert de relais entre sessions.

## Derniere mise a jour
**2026-03-25** — Debut de session : relecture etat, mise a jour fichiers

## Branche active
main

## Ou on en est

### Accompli (sessions precedentes)
- **C++ phases 1-9** : compilees, zero erreur
- **Fix 1 C++** : `hand_r` socket dans RevenantOpsCharacter.h, EnemyBase.h, WeaponBase.h
- **Fix 2 C++** : WaveSpawner auto-start dans StartMercenairesMatch()
- **Fix 3 MCP** : Waves configurees (3 vagues : 7/8/9 ennemis)
- **Build** : reussi 0 erreur (VS 2026, 20s)
- **Nouvelle map "The Compound"** : 73 meshes, 16 lumieres, 5 zones, sol 12000x8000u
- **Gameplay actors** : 27 repositionnes (spawns, pickups, NavMesh)
- **Flow jeu** : Title -> Loadout -> Match -> GameOver/Leaderboard (end-to-end)

### Map "The Compound" (Lvl_ThirdPerson)
| Zone | Contenu |
|---|---|
| Centrale | 8 piliers (4 grands + 4 petits), plateforme surelevee, 10 covers |
| Nord | Passerelle 300u, 4 colonnes support, 3 covers, escaliers |
| Sud | Couloir avec 3 ouvertures, toit, 3 covers |
| Est | 2 plateformes (200u + 400u), escaliers, 3 covers |
| Ouest | Batiment ferme, separateur, interieur meuble |

Eclairage : DirectionalLight atmosphere + 16 PointLights (warm/cool/orange par zone) + SkyLight realtime

### Priorite immediate : TEST PIE
1. Ouvrir Lvl_ThirdPerson dans UE5 Editor
2. Lancer PIE
3. Title Screen -> JOUER -> Loadout -> CONFIRMER
4. Verifier : arme visible ? ennemis spawn ? HUD ? kills ?
5. Si OK -> commit + phase 10 DONE

### Probleme connu (non-bloquant)
- **Meshes armes invisibles** : BP_Pistol etc. n'ont pas de StaticMesh -> tirs fonctionnent mais arme invisible
- **Screenshots MCP intermittents** : AutomationLibrary capture parfois mauvais viewport

## Fichiers modifies (non commites)
- Source/RevenantOps/RevenantOpsCharacter.h
- Source/RevenantOps/AI/EnemyBase.h
- Source/RevenantOps/Weapons/WeaponBase.h
- Source/RevenantOps/RevenantOpsPlayerController.cpp
- Source/RevenantOps/RevenantOps.Build.cs
- Source/RevenantOps/UI/*.cpp, *.h
- Content/ThirdPerson/Lvl_ThirdPerson.umap + ~90 ExternalActors

## Stack technique
- UE 5.7, C++, Enhanced Input, StateTree, MCPython/MCP TCP
- Modules : Engine, InputCore, EnhancedInput, AIModule, NavigationSystem, StateTreeModule, UMG, Slate
- VS 2026 Insiders (compiler 14.50.35727)
