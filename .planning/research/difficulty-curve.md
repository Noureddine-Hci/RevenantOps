# Difficulty Curve — RevenantOps v4.0

> Vagues adaptatives : la difficulté évolue dynamiquement selon le temps écoulé et le score du joueur.
> Sprint 3 sem 9 — refonte de `EnemyWaveSpawner`.

---

## État actuel (v3.0)

**Vagues hardcodées dans `BP_EnemyWaveSpawner`** :
- Vague 1 : 7 ennemis (5 Slow, 2 Runner)
- Vague 2 : 8 ennemis (4 Slow, 3 Runner, 1 Tank)
- Vague 3 : 9 ennemis (4 Runner, 3 Spitter, 2 Tank)
- `MaxAliveEnemies` = 8 (constant)
- `SpawnDelay` = 2s (constant)

**Problème** : le joueur expérimenté trouve ça trop facile, le débutant trouve ça trop dur. Pas d'adaptation.

---

## Approche v4.0 — courbe adaptative

### Principe

Au lieu de vagues fixes, le spawner calcule dynamiquement les paramètres selon :
1. **Temps écoulé** (T) — la difficulté monte avec le temps
2. **Score actuel** (S) — bonus difficulté si le joueur fait beaucoup de score (= bon joueur)
3. **Vague en cours** (W) — escalade entre vagues

### Formule

```cpp
struct FWaveParams {
    int32 MaxAliveEnemies;
    float SpawnDelay;
    float SlowRatio;      // 0-1
    float RunnerRatio;    // 0-1
    float TankRatio;      // 0-1
    float SpitterRatio;   // 0-1
    float ExploderRatio;  // 0-1
};

FWaveParams ComputeWaveParams(float ElapsedSec, int32 Score, int32 WaveIndex)
{
    // Normaliser le temps : 0 au début, 1 à 5 minutes
    const float TimeFactor = FMath::Clamp(ElapsedSec / 300.f, 0.f, 1.f);

    // Score factor : 0 si <2k, 1 si >18k
    const float ScoreFactor = FMath::Clamp((Score - 2000.f) / 16000.f, 0.f, 1.f);

    // Difficulty global : pondération 70% temps + 30% score
    const float Difficulty = 0.7f * TimeFactor + 0.3f * ScoreFactor;

    FWaveParams Params;

    // MaxAlive : 6 → 12 selon difficulty
    Params.MaxAliveEnemies = FMath::RoundToInt(FMath::Lerp(6.f, 12.f, Difficulty));

    // Spawn delay : 2.5s → 0.8s selon difficulty
    Params.SpawnDelay = FMath::Lerp(2.5f, 0.8f, Difficulty);

    // Mix des types — évolue par phase
    if (Difficulty < 0.3f)
    {
        // PHASE 1 : début — Slow majoritaires
        Params.SlowRatio     = 0.7f;
        Params.RunnerRatio   = 0.3f;
        Params.TankRatio     = 0.0f;
        Params.SpitterRatio  = 0.0f;
        Params.ExploderRatio = 0.0f;
    }
    else if (Difficulty < 0.6f)
    {
        // PHASE 2 : milieu — Runners arrivent, Tanks en intro
        Params.SlowRatio     = 0.4f;
        Params.RunnerRatio   = 0.4f;
        Params.TankRatio     = 0.1f;
        Params.SpitterRatio  = 0.1f;
        Params.ExploderRatio = 0.0f;
    }
    else
    {
        // PHASE 3 : fin — mix complet, Tanks et Exploders fréquents
        Params.SlowRatio     = 0.2f;
        Params.RunnerRatio   = 0.3f;
        Params.TankRatio     = 0.2f;
        Params.SpitterRatio  = 0.2f;
        Params.ExploderRatio = 0.1f;
    }

    return Params;
}
```

### Tableau visuel

| T (sec) | Score | Difficulty | MaxAlive | SpawnDelay | Mix dominant |
|---|---|---|---|---|---|
| 0 | 0 | 0.0 | 6 | 2.5s | Slow 70% |
| 60 | 1500 | 0.16 | 7 | 2.2s | Slow 70% |
| 120 | 4000 | 0.34 | 8 | 1.9s | Slow/Runner 50/50 |
| 180 | 8000 | 0.53 | 9 | 1.6s | Mix mid |
| 240 | 12000 | 0.71 | 10 | 1.3s | Tanks freq |
| 300 | 18000 | 0.90 | 12 | 0.9s | Mix end |

**Comportement émergent** :
- Joueur débutant (low score) → courbe douce mais pas plate
- Joueur expert (high score) → escalade plus rapide
- Toujours un challenge progressif, jamais punitif

---

## Implémentation C++

### `EnemyWaveSpawner` refacto

**Existant** : `Source/RevenantOps/AI/EnemyWaveSpawner.h/.cpp`

**Modifications** :
```cpp
// Header
USTRUCT(BlueprintType)
struct FWaveParams
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly) int32 MaxAliveEnemies = 6;
    UPROPERTY(BlueprintReadOnly) float SpawnDelay = 2.5f;
    UPROPERTY(BlueprintReadOnly) float SlowRatio = 0.7f;
    UPROPERTY(BlueprintReadOnly) float RunnerRatio = 0.3f;
    UPROPERTY(BlueprintReadOnly) float TankRatio = 0.f;
    UPROPERTY(BlueprintReadOnly) float SpitterRatio = 0.f;
    UPROPERTY(BlueprintReadOnly) float ExploderRatio = 0.f;
};

UCLASS()
class AEnemyWaveSpawner : public AActor
{
    // ... existant ...

protected:
    /** Récupère les params de vague adaptatifs au moment T */
    UFUNCTION(BlueprintCallable, Category = "WaveSpawner")
    FWaveParams ComputeWaveParams() const;

    /** Choisit le type d'ennemi à spawn selon les ratios */
    TSubclassOf<AEnemyBase> PickEnemyClass(const FWaveParams& Params) const;

    /** Référence au GameState pour lire score/temps */
    UPROPERTY()
    AMercenairesGameState* CachedGameState = nullptr;
};
```

```cpp
// Cpp
FWaveParams AEnemyWaveSpawner::ComputeWaveParams() const
{
    FWaveParams P;
    if (!CachedGameState) return P;

    const float ElapsedSec = CachedGameState->GetElapsedTime();
    const int32 Score = CachedGameState->GetTotalScore();

    const float TimeFactor = FMath::Clamp(ElapsedSec / 300.f, 0.f, 1.f);
    const float ScoreFactor = FMath::Clamp((Score - 2000.f) / 16000.f, 0.f, 1.f);
    const float Difficulty = 0.7f * TimeFactor + 0.3f * ScoreFactor;

    P.MaxAliveEnemies = FMath::RoundToInt(FMath::Lerp(6.f, 12.f, Difficulty));
    P.SpawnDelay = FMath::Lerp(2.5f, 0.8f, Difficulty);

    // Phases
    if (Difficulty < 0.3f) {
        P.SlowRatio = 0.7f; P.RunnerRatio = 0.3f;
    } else if (Difficulty < 0.6f) {
        P.SlowRatio = 0.4f; P.RunnerRatio = 0.4f;
        P.TankRatio = 0.1f; P.SpitterRatio = 0.1f;
    } else {
        P.SlowRatio = 0.2f; P.RunnerRatio = 0.3f;
        P.TankRatio = 0.2f; P.SpitterRatio = 0.2f; P.ExploderRatio = 0.1f;
    }

    return P;
}

TSubclassOf<AEnemyBase> AEnemyWaveSpawner::PickEnemyClass(const FWaveParams& Params) const
{
    const float Roll = FMath::FRand();
    float Acc = 0.f;

    Acc += Params.SlowRatio;     if (Roll <= Acc && SlowClass)     return SlowClass;
    Acc += Params.RunnerRatio;   if (Roll <= Acc && RunnerClass)   return RunnerClass;
    Acc += Params.TankRatio;     if (Roll <= Acc && TankClass)     return TankClass;
    Acc += Params.SpitterRatio;  if (Roll <= Acc && SpitterClass)  return SpitterClass;
    Acc += Params.ExploderRatio; if (Roll <= Acc && ExploderClass) return ExploderClass;

    return SlowClass; // fallback
}
```

### Tick logic

Dans `AEnemyWaveSpawner::Tick` (ou via timer) :
```cpp
void AEnemyWaveSpawner::TrySpawn()
{
    const FWaveParams Params = ComputeWaveParams();

    // Vérifier qu'on n'a pas dépassé MaxAlive
    if (CurrentAliveEnemies.Num() >= Params.MaxAliveEnemies) return;

    // Spawn un nouvel ennemi
    TSubclassOf<AEnemyBase> Class = PickEnemyClass(Params);
    if (Class) SpawnEnemyAtRandomPoint(Class);

    // Schedule next spawn avec le delay adaptatif
    GetWorld()->GetTimerManager().SetTimer(
        SpawnTimerHandle, this, &AEnemyWaveSpawner::TrySpawn,
        Params.SpawnDelay, false);
}
```

---

## Concept "vagues" vs "spawn continu"

### Option A — Vagues distinctes (existant)
Pour : structure claire, breaks entre vagues, sentiment de progression
Contre : peut sembler artificiel, breaks cassent le rythme

### Option B — Spawn continu adaptatif (proposé v4.0)
Pour : flux constant, immersion maintenue, courbe lissée
Contre : pas de "victoire" visible (pas de fin de vague)

### Décision : **HYBRIDE**

- Spawn continu mais avec des **"surges"** marquées
- Toutes les 60s, un mini-event : 8-10 ennemis spawn rapidement (3 secondes), puis retour à la normale
- Annonce HUD : "WAVE INCOMING" 5s avant le surge
- Donne une structure rythmique sans casser le flux

**Implémentation** :
```cpp
// Dans Tick :
const float TimeSinceLastSurge = ElapsedSec - LastSurgeTime;
if (TimeSinceLastSurge >= 60.f && !bSurgeActive)
{
    StartSurge();
    LastSurgeTime = ElapsedSec;
}

void StartSurge()
{
    bSurgeActive = true;
    BroadcastSurgeIncoming(); // HUD warning
    GetWorld()->GetTimerManager().SetTimer(
        SurgeEndTimer, this, &AEnemyWaveSpawner::EndSurge, 3.f, false);

    // Pendant 3s : SpawnDelay réduit à 0.3s, MaxAlive +5
}

void EndSurge()
{
    bSurgeActive = false;
    // Retour params normaux
}
```

---

## Variables exposées BP

Dans `AEnemyWaveSpawner` (UPROPERTY EditAnywhere) :

```cpp
/** Classes d'ennemis (assigner dans BP_EnemyWaveSpawner) */
UPROPERTY(EditAnywhere, Category = "WaveSpawner|Classes")
TSubclassOf<AEnemyBase> SlowClass;

UPROPERTY(EditAnywhere, Category = "WaveSpawner|Classes")
TSubclassOf<AEnemyBase> RunnerClass;

UPROPERTY(EditAnywhere, Category = "WaveSpawner|Classes")
TSubclassOf<AEnemyBase> TankClass;

UPROPERTY(EditAnywhere, Category = "WaveSpawner|Classes")
TSubclassOf<AEnemyBase> SpitterClass;

UPROPERTY(EditAnywhere, Category = "WaveSpawner|Classes")
TSubclassOf<AEnemyBase> ExploderClass;

/** Override courbe : pour debug / playtest */
UPROPERTY(EditAnywhere, Category = "WaveSpawner|Tuning")
bool bUseAdaptiveCurve = true;

/** Si bUseAdaptiveCurve == false, valeurs fixes */
UPROPERTY(EditAnywhere, Category = "WaveSpawner|Tuning",
          meta = (EditCondition = "!bUseAdaptiveCurve"))
int32 FixedMaxAlive = 8;

UPROPERTY(EditAnywhere, Category = "WaveSpawner|Tuning",
          meta = (EditCondition = "!bUseAdaptiveCurve"))
float FixedSpawnDelay = 2.f;
```

---

## Surge timing

| T (sec) | Event |
|---|---|
| 55 | "WAVE INCOMING" warning HUD |
| 60 | Surge start — spawn rapide 8-10 ennemis |
| 63 | Surge end — retour normal |
| 115 | Warning |
| 120 | Surge 2 |
| 175 | Warning |
| 180 | Surge 3 |
| 235 | Warning |
| 240 | Surge 4 (final, plus gros — 12 ennemis) |
| 295 | Warning |
| 300 | Surge boss / fin de match |

**5 surges sur 5 minutes** — donne une structure cyclique 60s.

---

## Difficulty modes (futur — backlog)

Pour v4.1 ou v5.0 :

| Mode | Multiplicateur | Description |
|---|---|---|
| Easy | ×0.7 | Half spawn rate, ennemis -25% HP |
| Normal | ×1.0 | Courbe ci-dessus |
| Hard | ×1.3 | Spawn rate +30%, ennemis +25% HP |
| Nightmare | ×1.6 | Spawn rate +60%, ennemis +50% HP, no pickups |

À implémenter avec un simple `DifficultyMultiplier` qui multiplie `MaxAlive` et divise `SpawnDelay`.

---

## Validation playtest

Test à effectuer en sprint 3 sem 9 :
- [ ] Run de 1min : difficulty < 0.3, principalement Slow/Runner
- [ ] Run de 3min : difficulty ~0.6, mix avec Tanks visibles
- [ ] Run de 5min : difficulty ~0.9, rythme intense, mix complet
- [ ] Surge à 60s/120s/180s/240s : warning + spawn boost visibles
- [ ] Joueur scoring fort : difficulty monte plus vite (visible)
- [ ] Joueur scoring faible : difficulty plus douce (visible)

---

## Métriques à logger

Pour ajuster la courbe en playtest, logger dans la console UE :
```cpp
UE_LOG(LogTemp, Log, TEXT("[Wave] T=%.1f Score=%d Diff=%.2f MaxAlive=%d SpawnDelay=%.2f"),
    ElapsedSec, Score, Difficulty, Params.MaxAliveEnemies, Params.SpawnDelay);
```

Permet de voir exactement comment évolue la difficulté en cours de run.

---

## Calibration (post-playtest)

Si après tests les valeurs sont mauvaises, ajuster les **constantes magiques** :

```cpp
// Dans ComputeWaveParams :
const float TimeFactor = FMath::Clamp(ElapsedSec / 300.f, 0.f, 1.f);
//                                              ^^ AJUSTER (300 = 5min cible)

const float ScoreFactor = FMath::Clamp((Score - 2000.f) / 16000.f, 0.f, 1.f);
//                                             ^^         ^^^^^ AJUSTER (cap haut)

const float Difficulty = 0.7f * TimeFactor + 0.3f * ScoreFactor;
//                       ^^                    ^^ AJUSTER (poids time vs score)

P.MaxAliveEnemies = FMath::RoundToInt(FMath::Lerp(6.f, 12.f, Difficulty));
//                                                ^^   ^^ AJUSTER bornes

P.SpawnDelay = FMath::Lerp(2.5f, 0.8f, Difficulty);
//                          ^^^   ^^^ AJUSTER bornes
```

Procéder par dichotomie : si trop facile, monter MaxAlive max. Si trop chaotique, baisser. Tester chaque changement avec 3 runs de référence (débutant / moyen / expert).
