# Phase 04 — Arène : Résumé d'exécution

**Complété le :** 2026-03-23 (géométrie)
**Statut :** PARTIEL — géométrie OK, setup éditeur manquant
**Fichiers modifiés :** 0 C++ (pure création éditeur via MCP)

## Ce qui a été fait

### Géométrie de l'arène (22 pièces via MCP Python)

Toutes les pièces utilisent `/Engine/BasicShapes/Cube` scalé.

| Pièce | Position | Scale |
|-------|----------|-------|
| Floor | [0,0,-50] | [50,50,1] |
| Wall Nord | [0,2500,300] | [50,1,7] |
| Wall Sud | [0,-2500,300] | [50,1,7] |
| Wall Est | [2500,0,300] | [1,50,7] |
| Wall Ouest | [-2500,0,300] | [1,50,7] |
| Couloir gauche mur 1 | [-1500,-800,200] | [1,10,5] |
| Couloir gauche mur 2 | [-1500,800,200] | [1,10,5] |
| Couloir toit | [-1500,0,500] | [1,17,1] |
| Plateforme élevée | [1800,1500,300] | [8,8,1] |
| Rampe d'accès | [1400,1500,150] | [1,8,4] |
| 5 caisses couverture | Centre dispersé | [2,2,2] |
| 4 piliers | [-800,±800], [800,±800] | [1,1,6] |
| Petite salle mur A | [1600,-1600,200] | [6,1,5] |
| Petite salle mur B | [2200,-1600,200] | [6,1,5] |
| Petite salle mur C | [1900,-1300,200] | [1,6,5] |

## Ce qui MANQUE (à faire dans UE5 Editor)

### AREN-01 — Spawn points zombies
- Placer 4-6 instances de `EnemyWaveSpawner` BP aux 4 coins + couloir + salle
- Configurer les wave configs sur chaque spawner

### AREN-02 — Pickups dans l'arène
- Placer 2-3 BP_TimeBonusPickup_30s (zones faciles d'accès)
- Placer 1-2 BP_TimeBonusPickup_15s (zones risquées)
- Placer 3-4 BP_AmmoBonusPickup dispersés

### AREN-03 — Navigation AI
- Ajouter un `Nav Mesh Bounds Volume` couvrant toute l'arène (5200x5200)
- Build Navigation
- Vérifier que les zombies pathfindent correctement autour des obstacles

### AREN-04 / AREN-05 — Éclairage
- Supprimer ou affaiblir le Directional Light par défaut
- Ajouter Point Lights orange/rouge pour ambiance tension
- Fog atmosphérique léger

## Blueprint à placer dans la Level

- EnemyWaveSpawner (déjà existant si Phase 1 créé)
- BP_TimeBonusPickup_30s, BP_TimeBonusPickup_15s
- BP_AmmoBonusPickup

## Décision

L'arène est dans `Lvl_ThirdPerson` (level existante). Aucune nouvelle level créée pour éviter crash MCP.
