---
name: balance-check
description: "Analyse l'équilibre des armes et ennemis via les DataTables. Détecte les valeurs aberrantes, les armes trop fortes/faibles, les ennemis déséquilibrés. Utiliser avant une démo ou après avoir modifié les DataTables."
user-invocable: true
allowed-tools: Read, Glob, Grep, Bash
model: sonnet
---

# Balance Check — RevenantOps

Analyse les DataTables et détecte les déséquilibres gameplay.

## 1. Lire les DataTables

Chercher dans le projet :
- `DT_WeaponStats` — stats des 6 armes (damage, fireRate, ammo, reloadTime, spread...)
- `DT_EnemyStats` — stats des 5 ennemis (health, speed, damage, killPoints...)

Lire aussi `Source/RevenantOps/` pour comprendre comment les valeurs sont utilisées :
- `WeaponBase.h/.cpp` — FireShot, spread, recoil
- `EnemyBase.h/.cpp` — GetKillPoints(), ApplyEnemyDataRow()

## 2. Analyse armes

Pour chaque arme (Pistol, AssaultRifle, SMG, Shotgun, Sniper, Melee) :

| Métrique | Formule | Seuil alerte |
|---|---|---|
| DPS | Damage × FireRate | Outlier > 2× la moyenne |
| TTK (Time To Kill) | Runner HP / DPS | < 0.5s ou > 5s |
| Efficacité ammo | DPS × MagSize | Déséquilibre > 3× |
| Reload penalty | ReloadTime / MagSize | > 0.5s par balle = lent |

Flags à signaler :
- Une arme avec DPS > 2× la moyenne des autres
- Sniper avec spread élevé (devrait être 0 ou quasi-0)
- Melee avec range = 0 mais damage > 200 (trop punitif)

## 3. Analyse ennemis

Pour chaque ennemi (Slow, Runner, Tank, Spitter, Ghost) :

| Métrique | Vérification |
|---|---|
| Kill Points proportionnels à la difficulté ? | Tank > Runner > Slow |
| Health vs Pistol DPS | Tank doit survivre > 3 secondes |
| Speed vs NavMesh | Runner speed < 600 (au-delà = difficile à toucher) |
| Projectile Spitter | Damage raisonnable (< 30% HP en un hit) |

## 4. Équilibre vague

Analyser la config de `BP_EnemyWaveSpawner` :
- Vague 1 : que des Slow → correct pour warmup
- Vague 2 : mix Slow + Runner → pic de difficulté raisonnable ?
- Vague 3 : Runner + Tank → gérable avec les armes disponibles ?

Score moyen par vague = Σ(ennemis × killPoints) → doit croître progressivement.

## 5. Rapport

Format de sortie :

```
=== Balance Check RevenantOps ===

ARMES :
✅ Pistol — DPS: 48, TTK Runner: 2.1s — OK
⚠️  Shotgun — DPS: 180, TTK Runner: 0.6s — TROP FORT au corps-à-corps
✅ Sniper — DPS: 95, TTK Runner: 1.1s — OK

ENNEMIS :
✅ Slow — 100 HP, 150 speed, 50pts — OK
⚠️  Tank — 400 HP, 80 speed, 200pts — vérifier si atteignable en vague 3

VAGUES :
✅ Vague 1 : 350 pts possibles — warmup correct
✅ Vague 2 : 675 pts possibles — pic raisonnable
⚠️  Vague 3 : 1200 pts possibles — difficile si Shotgun nerf

RECOMMANDATIONS :
1. Réduire Shotgun damage de 90 → 65 (trop dominant en close range)
2. Augmenter Tank kill points de 200 → 250 (difficulté justifie la récompense)
```
