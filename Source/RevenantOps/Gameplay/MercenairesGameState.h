// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MercenairesGameState.generated.h"

class AEnemyBase;
class UAudioComponent;
class USoundBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnScoreChanged, int32, NewScore,
                                              int32, PointsAdded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnComboChanged, int32,
                                              NewMultiplier, float,
                                              ComboTimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimerChanged, float,
                                             TimeRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchStateChanged, bool,
                                             bIsActive);

/**
 *  Mercenaires mode game state.
 *  Manages: 5-minute countdown timer, score with combo multiplier,
 *  time bonus pickups, and match flow (start/end).
 */
UCLASS(Blueprintable)
class AMercenairesGameState : public AGameStateBase {
  GENERATED_BODY()

public:
  AMercenairesGameState();

  virtual void Tick(float DeltaSeconds) override;

  // ========== MATCH FLOW ==========

  /** Starts the match timer and scoring */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires")
  void StartMatch();

  /** Ends the match (called when timer hits zero or manually) */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires")
  void EndMatch();

  /** Is the match currently active */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires")
  bool IsMatchActive() const { return bMatchActive; }

  // ========== TIMER ==========

  /** Adds bonus time (from pickups) */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires|Timer")
  void AddBonusTime(float Seconds);

  /** Returns remaining time in seconds */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires|Timer")
  float GetTimeRemaining() const { return TimeRemaining; }

  // ========== SCORE ==========

  /** Registers an enemy kill and awards points based on class */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires|Score")
  void RegisterKill(AEnemyBase *KilledEnemy);

  /** Returns the current total score */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires|Score")
  int32 GetCurrentScore() const { return CurrentScore; }

  /** Returns the current combo multiplier */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires|Score")
  int32 GetComboMultiplier() const { return ComboMultiplier; }

  /** Returns the combo timer remaining */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires|Score")
  float GetComboTimeRemaining() const { return ComboTimeRemaining; }

  /** Returns total kills this match */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires|Score")
  int32 GetTotalKills() const { return TotalKills; }

  /** Returns the best combo achieved this match */
  UFUNCTION(BlueprintCallable, Category = "Mercenaires|Score")
  int32 GetBestCombo() const { return BestCombo; }

  // ========== DELEGATES ==========

  UPROPERTY(BlueprintAssignable, Category = "Mercenaires|Events")
  FOnScoreChanged OnScoreChanged;

  UPROPERTY(BlueprintAssignable, Category = "Mercenaires|Events")
  FOnComboChanged OnComboChanged;

  UPROPERTY(BlueprintAssignable, Category = "Mercenaires|Events")
  FOnTimerChanged OnTimerChanged;

  UPROPERTY(BlueprintAssignable, Category = "Mercenaires|Events")
  FOnMatchStateChanged OnMatchStateChanged;

protected:
  // ========== MATCH CONFIG ==========

  /** Total match time in seconds (default 5 minutes) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mercenaires|Config",
            meta = (ClampMin = 30, ClampMax = 1800))
  float MatchDuration = 300.f;

  // ========== COMBO CONFIG ==========

  /** Time window to maintain combo (seconds without a kill) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mercenaires|Combo",
            meta = (ClampMin = 1.0, ClampMax = 15.0))
  float ComboWindowDuration = 5.0f;

  /** Maximum combo multiplier */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mercenaires|Combo",
            meta = (ClampMin = 2, ClampMax = 50))
  int32 MaxComboMultiplier = 20;

  // ========== SCORE CONFIG ==========

  /** Default points for unknown enemy types (fallback) */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mercenaires|Points")
  int32 DefaultKillPoints = 100;

  // ========== AUDIO ==========

  /** Background music that plays during the match */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mercenaires|Audio")
  USoundBase *BackgroundMusic = nullptr;

  /** Music volume */
  UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mercenaires|Audio",
            meta = (ClampMin = 0.0, ClampMax = 1.0))
  float MusicVolume = 0.5f;

  /** Audio component for background music */
  UPROPERTY()
  UAudioComponent *MusicComponent = nullptr;

  // ========== RUNTIME STATE ==========

  UPROPERTY(BlueprintReadOnly, Category = "Mercenaires|State")
  bool bMatchActive = false;

  UPROPERTY(BlueprintReadOnly, Category = "Mercenaires|State")
  float TimeRemaining = 300.f;

  UPROPERTY(BlueprintReadOnly, Category = "Mercenaires|State")
  int32 CurrentScore = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Mercenaires|State")
  int32 ComboMultiplier = 1;

  UPROPERTY(BlueprintReadOnly, Category = "Mercenaires|State")
  float ComboTimeRemaining = 0.f;

  UPROPERTY(BlueprintReadOnly, Category = "Mercenaires|State")
  int32 TotalKills = 0;

  UPROPERTY(BlueprintReadOnly, Category = "Mercenaires|State")
  int32 BestCombo = 1;

  // ========== INTERNAL ==========

  /** Determines points for an enemy class */
  int32 GetPointsForEnemy(AEnemyBase *Enemy) const;

  /** Resets combo to x1 */
  void ResetCombo();
};
