---
name: team-combat
description: "Orchestre plusieurs agents en parallèle sur un problème de combat gameplay. Utiliser pour des tâches qui touchent simultanément le code C++, les BPs, les VFX et les sons."
argument-hint: "[description du problème combat à résoudre]"
user-invocable: true
allowed-tools: Read, Glob, Grep, Bash, Task
model: sonnet
---

# Team Combat — Orchestration Multi-Agents

Lance des agents spécialisés en parallèle pour résoudre des problèmes combat complexes.

## Quand utiliser

- Camera shake + hit flash + VFX en même temps
- Refonte d'un système d'arme (tir, rechargement, spread, recul)
- Nouveau type d'ennemi (IA + animations + stats + VFX mort)
- Polish combat feel (juice : sons + particules + screen shake)

## Pipeline

### 1. Analyser la demande

Identifier les domaines impactés :
- **C++ gameplay** : WeaponBase, EnemyBase, Character, GameState
- **Blueprint** : BP_Weapon*, BP_Zombie*, ABP_Mercenaire
- **UMG** : HUD, crosshair, hit markers
- **VFX** : Niagara systems
- **Audio** : FireSound, HitSound, DeathSound

### 2. Lancer les agents en parallèle (via Task)

Pour chaque domaine impacté, lancer un sous-agent avec :
- Contexte exact : fichiers concernés, état actuel, ce qui doit changer
- Contraintes : pas de deprecated APIs, Live Coding si structurel, pas de MCP pendant PIE
- Output attendu : code / config prêt à appliquer

Agents disponibles :
- `unreal-specialist` — C++ systems, architecture
- `ue-blueprint-specialist` — BP graphs, BP/C++ boundary
- `ue-umg-specialist` — HUD, crosshair, widgets
- `ue-gas-specialist` — si GAS impliqué

### 3. Intégrer les résultats

- Vérifier la cohérence entre les outputs des agents
- Résoudre les conflits (ex: deux agents qui modifient le même fichier)
- Appliquer dans l'ordre : C++ → compile → BPs → PIE test

### 4. Validation

Lancer `/smoke-check` après intégration.
Si tout passe → `/gate-check [phase]` pour valider la phase complète.

## Exemple d'usage

```
/team-combat Phase 14 combat feel — camera shake au tir + hit flash ennemis + VFX muzzle flash
```

→ Lance en parallèle :
- Agent C++ : UCS_WeaponFire, UCS_TakeDamage, hit flash MID sur EnemyBase
- Agent Blueprint : assigner VFX aux 6 weapon BPs via CDO
- Agent UMG : hit marker dans WBP_Mercenaires_HUD

## Contraintes RevenantOps
- JAMAIS changer de niveau via MCP TCP (`Open` command) — crash UE5
- JAMAIS envoyer commandes MCP pendant PIE
- Changements structurels C++ → Live Coding → redémarrer UE5
- Assets binaires (.uasset) : un seul dev à la fois
