---
name: sprint-plan
description: "Planifie la prochaine phase RevenantOps en tâches concrètes avec priorités. Utiliser quand on démarre une nouvelle phase ou qu'on veut organiser le travail à venir."
argument-hint: "[phase-number ou description]"
user-invocable: true
allowed-tools: Read, Glob, Grep, Bash, Write
model: sonnet
---

# Sprint Plan — RevenantOps

Génère un plan de phase structuré et actionnable.

## 1. Contexte

Lire avant de planifier :
- `CLAUDE.md` — état actuel, phases complètes, prochaines étapes
- `~/.claude/primer.md` — contexte de session actif
- Code C++ existant dans `Source/RevenantOps/` pour comprendre ce qui est déjà là

## 2. Structure du plan

Pour chaque phase, produire :

```markdown
# Phase [N] — [Nom]
**Objectif** : [une phrase — ce que le joueur ressent à la fin]
**Dev responsable** : [Jilani | Noureddine | Les deux]
**Durée estimée** : [X sessions]
**Dépendances** : [Phase X doit être complète]

## Tâches

### C++ (si applicable)
- [ ] [fichier.h/.cpp] : [description précise]

### Blueprints (si applicable)
- [ ] [BP_XXX] : [ce qu'il faut modifier]

### Assets (si applicable)
- [ ] [asset] : [import / configuration]

### Tests PIE
- [ ] [scénario de test précis]

## Critères de succès (Gate Check)
- [critère mesurable 1]
- [critère mesurable 2]

## Risques
- [risque 1] → mitigation
```

## 3. Règles de planification RevenantOps

- **Une seule phase à la fois** — finir avant de commencer la suivante
- **Ownership des assets binaires** — annoncer sur Discord/WhatsApp avant de toucher un .uasset
- **Commits** : format `[J]` ou `[N]` + type(scope)
- **Live Coding** : changements structurels (nouvelles classes, nouveaux UPROPERTY) → redémarrer UE5
- **Jamais** : MCP pendant PIE, `Open` level via MCP, APIs dépréciées

## 4. Phases possibles suivantes

Basé sur CLAUDE.md état actuel (phase 17 complète) :

**Phase 18 — Sons & Audio** :
- Sons Kenney CC0 : FireSound/ReloadSound/EmptySound/HitSound/DeathSound
- USoundCue ou USoundWave assignés sur les weapon BPs et enemy BPs
- Sound attenuation pour les sons 3D

**Phase 18 — Upper Body Animations** :
- Animation Blueprint : blending upper/lower body (armed posture)
- Recoil animation sur tir
- Idle armed vs unarmed

**Phase 18 — Arena Polish** :
- Éclairage affiné sur BlackSite
- Décoration / props supplémentaires
- Pickups visuellement attrayants (matériaux émissifs)

**Phase 19 — Build Démo Jouable** :
- Packaging standalone
- Settings menu (résolution, volume)
- Écran de chargement
