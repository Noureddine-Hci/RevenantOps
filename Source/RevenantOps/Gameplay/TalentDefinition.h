// Copyright RevenantOps. All Rights Reserved.
#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TalentDefinition.generated.h"

/**
 *  UTalentDefinition — Data Asset créable dans le Content Browser.
 *  Définit un talent assignable à un personnage.
 *  Créer via : Clic droit → Miscellaneous → Data Asset → TalentDefinition
 */
UCLASS(BlueprintType)
class REVENANTOPS_API UTalentDefinition : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    // ── Identité ──────────────────────────────────────────────────────────────

    /** Nom affiché dans l'écran de sélection */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Identity")
    FText DisplayName = FText::FromString(TEXT("Talent"));

    /** Description courte affichée dans l'écran de sélection */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Identity",
              meta = (MultiLine = true))
    FText Description;

    /** Icône affichée dans le panneau droit (optionnel) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Identity")
    TObjectPtr<UTexture2D> Icon = nullptr;

    // ── Bonus de statistiques ─────────────────────────────────────────────────

    /** Multiplicateur de vitesse de rechargement (+0.5 = 50% plus vite) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Stats",
              meta = (ClampMin = 0.f, ClampMax = 3.f))
    float ReloadSpeedBonus = 0.f;

    /** Réduction des dégâts reçus en % (0.25 = 25% de réduction) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Stats",
              meta = (ClampMin = 0.f, ClampMax = 0.75f))
    float DamageResistanceBonus = 0.f;

    /** Multiplicateur de munitions en réserve (+0.5 = 50% de plus) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Stats",
              meta = (ClampMin = 0.f, ClampMax = 2.f))
    float AmmoCapacityBonus = 0.f;

    /** Bonus de vitesse de déplacement (ajouté à WalkSpeed, en unités/s) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Stats",
              meta = (ClampMin = 0.f, ClampMax = 300.f))
    float MoveSpeedBonus = 0.f;

    /** Bonus de vie maximale (ajouté à MaxHealth) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Stats",
              meta = (ClampMin = 0.f, ClampMax = 200.f))
    float MaxHealthBonus = 0.f;

    /** Bonus d'endurance maximale (ajouté à MaxStamina) */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Talent|Stats",
              meta = (ClampMin = 0.f, ClampMax = 100.f))
    float StaminaBonus = 0.f;
};
