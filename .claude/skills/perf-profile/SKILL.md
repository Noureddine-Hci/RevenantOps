---
name: perf-profile
description: "Guide un audit de performance UE5.7 sur RevenantOps. Identifie les goulots d'étranglement (Tick, spawns, VFX, UI). Utiliser avant une démo ou si le jeu lag."
user-invocable: true
allowed-tools: Read, Glob, Grep, Bash
model: sonnet
---

# Perf Profile — RevenantOps UE5.7

Guide structuré pour identifier et corriger les problèmes de performance.

## 1. Commandes de profiling in-game (console UE5)

Taper dans la console PIE (touche `~`) :

```
stat fps              — FPS + frame time
stat unit             — Game / Draw / GPU breakdown
stat game             — coût CPU gameplay (Tick, etc.)
stat slate            — coût UI
stat niagara          — coût VFX Niagara
stat ai               — coût IA / PathFinding
```

Cible pour une démo : **60 FPS stable**, frame time < 16ms.

## 2. Audit C++ — Tick suspects

Chercher tous les Actors avec Tick activé :
```
grep -r "PrimaryActorTick.bCanEverTick = true" Source/
grep -r "::Tick(" Source/
```

Pour chaque Tick trouvé, vérifier :
- [ ] Est-ce vraiment nécessaire ? (souvent remplaçable par Timer ou Delegate)
- [ ] Y a-t-il des Cast<> ou FindObject dedans ? (cache dans BeginPlay)
- [ ] Combien d'instances simultanées ? (50 ennemis × Tick = problème)

### Patterns coûteux à flaguer
```cpp
// ❌ Cast dans Tick — coûteux si fréquent
void AEnemyBase::Tick(float DeltaTime) {
    if (ARevenantOpsCharacter* PC = Cast<ARevenantOpsCharacter>(GetWorld()->GetFirstPlayerController()->GetPawn()))

// ✅ Cache dans BeginPlay
void AEnemyBase::BeginPlay() {
    CachedPlayer = Cast<ARevenantOpsCharacter>(...);
}
```

## 3. Audit Blueprints

Dans UE5 Editor → Window → Blueprint Profiler (si activé) :
- Identifier les BPs avec > 1ms de coût par frame
- Chercher les Event Tick dans les BPs (souvent oubliés)
- Vérifier ABP_Mercenaire : transitions trop fréquentes ?

## 4. Audit VFX Niagara

`stat niagara` en PIE. Si > 2ms :
- NS_MuzzleFlash : vérifie que l'émetteur s'arrête après 0.1s
- NS_Impact_Surface / NS_Impact_Blood : pooling activé ?
- Spawn rate raisonnable (pas de boucle infinie)

## 5. Audit UI

`stat slate` en PIE. Si > 2ms :
- HUD : `Invalidation Box` autour des parties statiques
- Crosshair : interpolation dans NativeTick — coûteux si widget lourd
- Inventaire : ne devrait pas être actif en permanence (Collapsed quand fermé)

## 6. Audit IA / NavMesh

`stat ai` en PIE. Si > 3ms avec 8 ennemis :
- PathFinding trop fréquent → augmenter `PathFollowingInterval`
- BehaviorTree Tick trop court → passer à 0.2s minimum
- NavMesh trop grand → réduire à la zone de jeu réelle

## 7. Rapport

```
=== Perf Profile RevenantOps ===
FPS moyen : [X] | Frame time : [X]ms
GPU : [X]ms | CPU Game : [X]ms | Draw : [X]ms

PROBLÈMES DÉTECTÉS :
🔴 [AEnemyBase::Tick] Cast<> non-caché — 8 instances × 0.3ms = 2.4ms
🟡 [NS_MuzzleFlash] émetteur ne s'arrête pas — accumulation sur tir rapide
🟢 UI : OK (0.8ms)

ACTIONS PRIORITAIRES :
1. Cacher le Cast<> dans AEnemyBase::BeginPlay → gain estimé 2ms
2. Vérifier loop condition NS_MuzzleFlash

VERDICT : ACCEPTABLE pour démo / OPTIMISATION REQUISE avant release
```
