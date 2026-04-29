# Game Design — Système de progression v4.0

> Système d'unlock + XP + achievements. Permet d'allonger la durée de vie du jeu et donner un sens à la rejouabilité.

---

## Pourquoi un système de progression ?

**État v3.0 actuel** : tous les personnages, talents et armes sont disponibles dès le premier match.
**Problème** : aucune raison de rejouer après avoir testé toutes les combinaisons.
**Solution v4.0** : unlock progressif basé sur le score cumulé total.

**Inspiration** :
- **Resident Evil 5 Mercenaries** : unlock characters via points "BSAA"
- **Vampire Survivors** : meta-progression simple, addictive
- **Doom Eternal** : challenges → cosmétiques

---

## Architecture C++ (sprint 3 sem 9)

### `UGameProgressSaveGame` (USaveGame)

**Fichier** : `Source/RevenantOps/Gameplay/GameProgressSaveGame.h/.cpp`

```cpp
UCLASS()
class REVENANTOPS_API UGameProgressSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    // Stats cumulatives
    UPROPERTY() int64 TotalScoreCumul = 0;
    UPROPERTY() int32 TotalKills      = 0;
    UPROPERTY() int32 TotalRuns       = 0;
    UPROPERTY() float TotalTimePlayed = 0.f; // secondes
    UPROPERTY() int32 BestSingleScore = 0;
    UPROPERTY() int32 BestCombo       = 0;

    // Unlocks (par nom unique)
    UPROPERTY() TSet<FName> UnlockedCharacters;
    UPROPERTY() TSet<FName> UnlockedTalents;
    UPROPERTY() TSet<FName> UnlockedWeapons;
    UPROPERTY() TSet<FName> UnlockedAchievements;

    // Slot save name constant
    static const FString SlotName;

    UFUNCTION(BlueprintCallable)
    static UGameProgressSaveGame* LoadOrCreate();

    UFUNCTION(BlueprintCallable)
    void Save();

    /** Apply un run terminé : ajoute score, increment runs, vérifie unlocks */
    UFUNCTION(BlueprintCallable)
    void RegisterRunCompleted(int32 RunScore, int32 RunKills, int32 RunCombo, float RunTime);
};
```

### Hooks dans le code existant

**`AMercenairesGameState::EndMatch`** :
```cpp
void AMercenairesGameState::EndMatch(bool bVictory)
{
    // ... code existant ...

    // Sauvegarder progression
    UGameProgressSaveGame* Save = UGameProgressSaveGame::LoadOrCreate();
    if (Save)
    {
        const float ElapsedTime = MatchTimeLimit - GetTimeRemaining();
        Save->RegisterRunCompleted(GetTotalScore(), GetTotalKills(), GetBestCombo(), ElapsedTime);

        // Vérifier les nouveaux unlocks
        TArray<FName> NewUnlocks = CheckNewUnlocks(Save);
        for (const FName& UnlockId : NewUnlocks)
        {
            // Notifier via HUD (popup achievement)
            BroadcastUnlock(UnlockId);
        }

        Save->Save();
    }
}
```

---

## Tiers d'unlock — characters

| Tier | Score cumul requis | Personnage débloqué | Talents starter |
|---|---|---|---|
| 0 | 0 | **MARC** (Sergeant) | Reload +20%, Stamina +15% |
| 1 | 1,000 | **JANE** (Operator) | Sprint +10%, Damage Resist +10% |
| 2 | 5,000 | **ALEX** (Heavy) | MaxHealth +30%, MoveSpeed -5% |
| 3 | 15,000 | **KAI** (Scout) | Sprint +25%, Stamina +30%, Damage -10% |
| 4 | 30,000 | **DR. X** (???) | AmmoCapacity +50%, Reload +40% |

**Note** : MARC est unlock dès le départ pour permettre de jouer. Les 4 autres apparaissent grisés en CharacterSelect avec leur score requis affiché.

---

## Tiers d'unlock — weapons

| Tier | Score cumul requis | Arme débloquée |
|---|---|---|
| 0 | 0 | Pistol (M92F) |
| 0 | 0 | SMG (VZ61) |
| 1 | 2,000 | Assault Rifle (AK74) |
| 2 | 7,000 | Shotgun |
| 3 | 18,000 | Sniper |
| 4 | 35,000 | LMG (futur) |

---

## Tiers d'unlock — talents

| Tier | Score cumul requis | Talent |
|---|---|---|
| 0 | 0 | Reload Speed |
| 0 | 0 | Stamina Boost |
| 1 | 3,000 | Damage Resistance |
| 2 | 8,000 | Move Speed |
| 3 | 12,000 | Max Health |
| 4 | 25,000 | Ammo Capacity |

---

## Système XP & rang

### XP gagné par run

**Formule** :
```
XP_base   = RunScore / 10
XP_bonus  = bonus selon tier obtenu en fin de run
XP_total  = XP_base × multiplicateur tier

Tier S : ×3.0 (premium)
Tier A : ×2.0
Tier B : ×1.5
Tier C : ×1.0
Tier D : ×0.7
```

**Exemple** :
- Run 18,000 score, tier S → XP = 1800 × 3 = 5,400 XP
- Run 5,000 score, tier C → XP = 500 × 1 = 500 XP

### Tier de fin de run

**Calcul** dans `UGameOverWidget::ComputeTier(int32 Score, int32 BestCombo, float TimeUsed)` :

```cpp
EGameTier ComputeTier(int32 Score, int32 BestCombo, float TimeUsed)
{
    // Bonus combo : +1000 par tranche de 10 combo
    int32 ComboBonus = (BestCombo / 10) * 1000;

    // Bonus speedrun : seulement si bVictory && TimeUsed < 3min30
    int32 SpeedBonus = (TimeUsed < 210.f) ? 2000 : 0;

    int32 TotalScore = Score + ComboBonus + SpeedBonus;

    if      (TotalScore >= 20000 && BestCombo >= 12) return EGameTier::S;
    else if (TotalScore >= 12000 && BestCombo >= 8)  return EGameTier::A;
    else if (TotalScore >= 7000  && BestCombo >= 5)  return EGameTier::B;
    else if (TotalScore >= 3000)                      return EGameTier::C;
    else                                              return EGameTier::D;
}
```

**Affichage GameOver** : grande lettre tier (Stencil 200pt) avec couleur correspondante :
- S : `RedBlood` + glow
- A : `GoldTarnish`
- B : `WhiteText`
- C : `GreySoft`
- D : `GreyMid`

---

## Achievements (8 de base v4.0)

| ID | Nom | Description | Trigger |
|---|---|---|---|
| ACH-001 | "First Blood" | Tuer 1 ennemi | OnEnemyDied (premier kill) |
| ACH-002 | "Centurion" | Tuer 100 ennemis (cumulé) | TotalKills >= 100 |
| ACH-003 | "Combo Master" | Atteindre combo x10 dans un run | OnComboReached(10) |
| ACH-004 | "Survivor" | Survivre 5 minutes complètes | OnMatchEnd && TimeUsed >= 300 |
| ACH-005 | "Speedrun" | Compléter mission en moins de 3min30 | OnMatchEnd && TimeUsed < 210 |
| ACH-006 | "Tier S Run" | Obtenir un rank S | ComputeTier == S |
| ACH-007 | "Pacifist" | Compléter une vague sans subir dégât | Wave OnComplete && DamageTaken == 0 |
| ACH-008 | "Boom Headshot" | 50 headshots cumulés | OnHeadshot (cumul) |

### `UAchievementDefinition` (DataAsset)

**Fichier** : `Source/RevenantOps/Gameplay/AchievementDefinition.h`

```cpp
UCLASS(BlueprintType)
class UAchievementDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere) FName AchievementId;
    UPROPERTY(EditAnywhere) FText DisplayName;
    UPROPERTY(EditAnywhere, meta=(MultiLine=true)) FText Description;
    UPROPERTY(EditAnywhere) UTexture2D* Icon = nullptr;
    UPROPERTY(EditAnywhere) int32 XPReward = 500;
};
```

**Stockage** : un DataAsset par achievement dans `Content/Mercenaires/Achievements/DA_ACH_*`

### Notification popup

**Widget** : `Source/RevenantOps/UI/AchievementWidget.h`

Animation : fade-in 300ms → hold 3s → fade-out 500ms.
Position : top-right, sous le HUD.
Son : "achievement_unlock.wav" (CC0)

---

## Logique unlock dans `CharacterSelectWidget`

```cpp
void UCharacterSelectWidget::PopulateCharacters(const TArray<FCharacterInfo>& Characters)
{
    UGameProgressSaveGame* Save = UGameProgressSaveGame::LoadOrCreate();

    for (const FCharacterInfo& Char : Characters)
    {
        bool bUnlocked = Save->UnlockedCharacters.Contains(Char.CharacterId);

        // Créer la card avec état grisé si pas unlocked
        UMenuCardWidget* Card = CreateCard(Char, bUnlocked);
        if (!bUnlocked)
        {
            Card->ShowLockOverlay(Char.RequiredScore); // affiche "🔒 5,000 pts requis"
        }
    }
}
```

Pareil pour `LoadoutWidget` (armes grisées si pas unlock) et talents.

---

## UI dédié — section Stats Personnel

**Nouveau widget** : `UStatsPersonnelWidget` (accessible depuis TitleScreen → STATS)

**Affiche** :
- Total Score Cumul : 87,450
- Total Kills : 1,243
- Best Single Score : 18,450
- Best Combo : x14
- Total Runs : 27
- Total Time Played : 3h 42min
- Achievements unlocked : 6/8
- Liste characters/weapons/talents unlocked + locked

**Pas critique sprint 3** — peut attendre v4.1.

---

## Reset progression (option Options)

Dans `OptionsWidget` section Gameplay, ajouter :
```
[Réinitialiser progression] (avec confirmation)
```

Code : `UGameProgressSaveGame::Reset()` qui clear tous les fields et save.

**Pourquoi** : utile en dev / tests, et pour les joueurs qui veulent recommencer.

---

## Validation sprint 3 — progression

Tests à effectuer en PIE :
- [ ] 1er match : seulement MARC dispo, autres grisés avec scores requis
- [ ] Run de 1500 pts → unlock JANE → notification + JANE débloqué
- [ ] Stats persistantes entre sessions (fermer/relancer UE5)
- [ ] Tier S correctement calculé
- [ ] Tous les 8 achievements triggerable
- [ ] Reset progression efface bien toutes les données
- [ ] Pas de crash si savefile corrompu (fallback sur LoadOrCreate)

---

## Estimation impact gameplay

**Sans progression (v3.0)** :
- Sessions moyennes : 3-5 runs avant lassitude
- Aucun objectif de long terme

**Avec progression (v4.0)** :
- Sessions moyennes : 15-30 runs (target unlock complet)
- Replayability : très élevée — chaque run contribue
- Sentiment d'évolution : présent à chaque match

**Inspiration validée** : Vampire Survivors a prouvé qu'une simple meta-progression peut tripler le temps de jeu moyen.
