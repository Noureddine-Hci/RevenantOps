---
name: code-review
description: "Review C++ ciblée UE5.7 sur un fichier ou une feature. Vérifie conventions, memory leaks, patterns UE5, performance. Utiliser après avoir écrit du nouveau code C++ avant de committer."
argument-hint: "[fichier ou feature à reviewer, ex: WeaponBase ou Phase14]"
user-invocable: true
allowed-tools: Read, Glob, Grep, Bash
model: sonnet
---

# Code Review — RevenantOps C++ UE5.7

Review ciblée sur le code C++ du projet.

## 1. Identifier les fichiers à reviewer

Si `$ARGUMENTS` fourni → chercher les fichiers correspondants dans `Source/RevenantOps/`.
Sinon → lister les fichiers modifiés récemment : `git diff --name-only HEAD~1`

## 2. Checklist de review

### Conventions UE5
- [ ] Prefixes corrects : U (UObject), A (Actor), F (struct), E (enum), I (interface)
- [ ] `GENERATED_BODY()` dans toutes les classes UObject-dérivées
- [ ] `UPROPERTY()` sur tous les pointeurs UObject (GC safety)
- [ ] `UFUNCTION()` sur les fonctions exposées Blueprint
- [ ] CamelCase partout, pas de snake_case
- [ ] `Super::` appelé dans tous les overrides (BeginPlay, EndPlay, Tick...)

### Memory & GC
- [ ] Pas de `new`/`delete` pour UObjects
- [ ] `TObjectPtr<>` ou `UPROPERTY` pour les références UObject (pas raw pointers)
- [ ] `TWeakObjectPtr` pour les références qui peuvent expirer
- [ ] Pas de circular references non gérées
- [ ] Delegates : `RemoveDynamic` avant `AddDynamic` sur les rebinds (anti double-bind replay)

### APIs UE 5.7
- [ ] `SetCrouchedHalfHeight()` (pas `CrouchedHalfHeight` direct)
- [ ] Enhanced Input System (pas legacy Input)
- [ ] Pas d'APIs deprecated signalées par le compilateur
- [ ] Cast `AController` → `APlayerController` pour `AddPitchInput`/`AddYawInput`

### Performance
- [ ] Tick désactivé si non nécessaire (`PrimaryActorTick.bCanEverTick = false`)
- [ ] Pas de string operations dans Tick
- [ ] Pas de `FindObject` / `LoadObject` dans Tick ou FireShot
- [ ] VFX/Sons : pas de spawn/destroy chaque frame (pooling ou one-shot)

### Guards & Null checks
- [ ] `bIsDead` guard dans HandleDeath (anti double-death)
- [ ] `bLoadoutConfirmed` guard dans PlayerController (anti double-confirm)
- [ ] Null check `OwnerPawn` dans WeaponBase::FireShot
- [ ] Null checks sur tous les `Cast<>` avant utilisation

### Patterns RevenantOps
- [ ] Camera shake : `UDefaultCameraShakeBase` + `UPerlinNoiseCameraShakePattern` (pas `ChangeRootShakePattern` dans constructeur)
- [ ] Widget creation dans `ReceivedPlayer()` (pas `BeginPlay`)
- [ ] DataTable access via `ApplyWeaponDataRow()` / `ApplyEnemyDataRow()`
- [ ] `ScopedEditorTransaction` + `modify()` pour les CDO changes via Python MCP

## 3. Format de sortie

```
=== Code Review : [fichier] ===

✅ Conventions UE5 : OK
✅ Memory/GC : OK
⚠️  [ligne 42] : Raw pointer sans UPROPERTY — risque GC stall
❌ [ligne 87] : Super::BeginPlay() manquant — bug potentiel

PRIORITÉ HAUTE :
- [correction à faire]

PRIORITÉ BASSE :
- [amélioration suggérée]

VERDICT : APPROVE / REQUEST CHANGES
```
