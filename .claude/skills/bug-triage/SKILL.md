---
name: bug-triage
description: "Classifie un bug RevenantOps : sévérité, cause probable, qui doit le fixer, dans quelle phase. Utiliser quand un bug est découvert en PIE ou reporté par l'autre dev."
argument-hint: "[description du bug observé]"
user-invocable: true
allowed-tools: Read, Glob, Grep, Bash
model: sonnet
---

# Bug Triage — RevenantOps

Analyse et classifie rapidement un bug pour décider quoi faire.

## 1. Collecter les infos

Si `$ARGUMENTS` fourni → analyser la description.
Sinon → demander :
- Que s'est-il passé ? (comportement observé)
- Que devait-il se passer ? (comportement attendu)
- Dans quel contexte ? (PIE / Editor / Packaging / quelle action)
- Reproductible à chaque fois ?

## 2. Classification

### Sévérité
| Niveau | Définition | Exemple RevenantOps |
|---|---|---|
| **CRASH** | Le jeu plante | "Array index out of bounds" au sprint |
| **BLOQUANT** | Flow principal cassé | GameOver ne se déclenche pas |
| **MAJEUR** | Feature importante cassée | Inventaire bloque les inputs définitivement |
| **MINEUR** | Gêne mais contournable | Icône arme manquante dans un slot |
| **COSMÉTIQUE** | Visuel incorrect | Crosshair légèrement décalé |

### Cause probable
Chercher dans le code les patterns suspects :
- Double-bind ? → chercher `AddDynamic` sans `RemoveDynamic` préalable
- Null pointer ? → chercher les `Cast<>` sans vérification
- Array out of bounds ? → BlendSpace avec vitesse max < SprintSpeed
- Double-trigger ? → chercher les guards `bIsDead`, `bLoadoutConfirmed`
- Widget crash ? → `CreateWidget` dans `BeginPlay` sans joueur joint

### Ownership
| Zone | Dev responsable |
|---|---|
| C++ gameplay (WeaponBase, EnemyBase, Character) | Jilani |
| Level design, spawners, pickups | Jilani |
| Animations (ABP_Mercenaire, BlendSpaces) | Noureddine |
| Blender / Assets 3D | Noureddine |
| HUD / UMG | Selon la phase active |

## 3. Bugs connus et solutions

| Bug | Cause | Fix |
|---|---|---|
| "Array index 3 into array of size 3" au sprint | BS_Pistol_Walk_Run vitesse max < SprintSpeed | BS_IdleRun dans Graph[2] Walk/Run |
| Double-kill enemies | Pas de `bIsDead` guard | Guard dans EnemyBase::HandleDeath |
| Double-confirm loadout | Pas de `bLoadoutConfirmed` guard | Guard dans PlayerController |
| WBP corrompu ("Impossible de charger WidgetTree") | `RebuildWidget` modifie WidgetTree en éditeur | Ajouter `if (!IsDesignTime())` |
| CreateWidget crash | Appelé dans BeginPlay sans joueur | Utiliser `ReceivedPlayer()` |
| Caméra zoom intempestif | bDoCollisionTest=true sur SpringArm | bDoCollisionTest=False |

## 4. Rapport de triage

```
=== Bug Triage ===
Description : [bug décrit]
Sévérité    : CRASH / BLOQUANT / MAJEUR / MINEUR / COSMÉTIQUE
Cause prob. : [hypothèse + fichier:ligne suspect]
Owner       : Jilani / Noureddine / Les deux
Phase       : Fix dans Phase [X] ou hotfix immédiat
Effort est. : [30min / 2h / demi-journée]

Action immédiate : [quoi faire maintenant]
```
