// Copyright RevenantOps. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Gameplay/AmmoTypes.h"
#include "Gameplay/ItemDefinition.h"
#include "DestructibleObject.generated.h"

class UHealthComponent;
class UStaticMeshComponent;

// ─────────────────────────────────────────────────────────────────────────────
//  Entrée de loot : une classe d'acteur + sa probabilité de drop individuelle.
//  Exemples :
//    - BP_HealthPickup_Small  / DropChance = 0.8
//    - BP_AmmoBonusPickup     / DropChance = 0.5  (configurer l'AmmoType dans le BP)
//    - BP_Pistol              / DropChance = 0.1
// ─────────────────────────────────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCrateLootEntry
{
    GENERATED_BODY()

    /** Classe de l'acteur à spawner (BP_HealthPickup_Small, BP_AmmoBonusPickup, etc.) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    TSubclassOf<AActor> ActorClass;

    /** Probabilité de drop (0 = jamais, 1 = toujours) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot",
              meta = (ClampMin = 0.f, ClampMax = 1.f))
    float DropChance = 0.5f;

    /**
     *  Si true, garantit le drop de cet item même si un autre item a déjà été droppé.
     *  Si false, l'item est optionnel selon DropChance.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    bool bGuaranteed = false;

    /**
     *  Filtre adaptatif — si different de None, cet item ne droppera QUE si
     *  le joueur possede une arme du type correspondant dans son inventaire.
     *  Laisser a None pour les soins, armes et autres items toujours pertinents.
     *
     *  Exemple : AmmoTypeFilter = Rifle -> ne drop que si le joueur a un fusil.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    EAmmoType AmmoTypeFilter = EAmmoType::None;

    /**
     *  Poids de selection en mode PickOne.
     *  Poids egal sur toutes les entrees = chance egale.
     *  Doubler le poids d'une entree = deux fois plus probable que les autres.
     *  Ex : Pistolet=1, Fusil=1, SMG=2 -> SMG a 50%, les autres 25% chacun.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot",
              meta = (ClampMin = 0.01f, ClampMax = 100.f))
    float Weight = 1.f;
};

// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct FHealthDropEntry
{
    GENERATED_BODY()

    /** Classe du pickup à spawner (BP_HealthPickup ou enfant) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    TSubclassOf<AActor> PickupClass;

    /** DA qui définit le mesh, l'icône et le nom affiché */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Loot")
    TObjectPtr<UItemDefinition> ItemDefinition = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class ELootMode : uint8
{
    /**
     *  Chaque entree est tiree independamment selon son DropChance.
     *  Plusieurs items peuvent dropper simultanement.
     *  A utiliser pour caisses de soin et munitions.
     */
    Independent UMETA(DisplayName = "Independent - chaque entree tiree separement"),

    /**
     *  Une seule entree est selectionnee au hasard selon les poids (Weight).
     *  Garantit exactement 1 drop.
     *  A utiliser pour caisses d'armes.
     */
    PickOne UMETA(DisplayName = "Pick One - 1 seule entree par poids"),
};

// ─────────────────────────────────────────────────────────────────────────────

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnObjectDestroyed,
                                             ADestructibleObject*, Object);

/**
 *  Caisse / objet destructible.
 *  - Prend des dégâts et change de matériau quand endommagée (< DamagedThreshold %)
 *  - Spawne des items au décès selon un loot table par entrée
 *  - Peut exploser au décès
 */
UCLASS(Blueprintable)
class ADestructibleObject : public AActor
{
    GENERATED_BODY()

public:
    ADestructibleObject();

protected:
    virtual void BeginPlay() override;

    // ── COMPOSANTS ─────────────────────────────────────────────────────────

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ObjectMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UHealthComponent* HealthComp;

    // ── VISUELS ────────────────────────────────────────────────────────────

    /**
     *  Matériau normal (slot 0). Laisser vide = matériau du mesh.
     *  Assigne un M_Crate_Normal dans le BP.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Visual")
    TObjectPtr<UMaterialInterface> NormalMaterial = nullptr;

    /**
     *  Matériau endommagé — appliqué quand HP < DamagedThreshold %.
     *  Assigne un M_Crate_Damaged dans le BP (même mat + cracks, plus sombre).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Visual")
    TObjectPtr<UMaterialInterface> DamagedMaterial = nullptr;

    /**
     *  Seuil HP (en %) sous lequel le matériau passe en "endommagé".
     *  Défaut : 50 % (entre 0 et 100).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Visual",
              meta = (ClampMin = 1, ClampMax = 99))
    float DamagedThreshold = 50.f;

    /** True si le matériau endommagé a déjà été appliqué (évite les resets inutiles) */
    bool bDamagedMaterialApplied = false;

    // ── EXPLOSION ──────────────────────────────────────────────────────────

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Explosion")
    bool bExplodesOnDestruction = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Explosion",
              meta = (ClampMin = 0, ClampMax = 500, EditCondition = "bExplodesOnDestruction"))
    float ExplosionDamage = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Explosion",
              meta = (ClampMin = 0, ClampMax = 2000, EditCondition = "bExplodesOnDestruction"))
    float ExplosionRadius = 300.f;

    // ── LOOT TABLE ─────────────────────────────────────────────────────────

    /**
    /**
     *  Mode de tirage du loot.
     *  Independent : chaque entree est tiree selon son DropChance (caisses soins/munitions).
     *  PickOne      : exactement 1 entree est choisie selon Weight (caisses armes).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Loot")
    ELootMode LootMode = ELootMode::Independent;

    /**
     *  Table de loot.
     *  Mode Independent : utiliser DropChance + AmmoTypeFilter.
     *  Mode PickOne     : utiliser Weight (poids relatif de selection).
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Loot")
    TArray<FCrateLootEntry> LootTable;

    /** Distance max de dispersion des drops autour du centre de la caisse (cm). */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Loot",
              meta = (ClampMin = 0, ClampMax = 300))
    float LootScatterRadius = 60.f;

    /**
     *  Mapping automatique EAmmoType → BP pickup de munitions.
     *  La caisse spawne automatiquement les munitions des armes que le joueur possède.
     *  Ex : Pistol → BP_AmmoPickup_Pistol, Rifle → BP_AmmoPickup_Rifle, etc.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Loot")
    TMap<EAmmoType, TSubclassOf<AActor>> AmmoPickupClasses;

    /**
     *  DA par type de munitions — assigné au pickup après spawn.
     *  Ex : Pistol → DA_Item_Ammo_Pistol, Rifle → DA_Item_Ammo_Rifle
     *  Permet d'avoir mesh + icone + nom corrects sans BPs enfants.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Loot")
    TMap<EAmmoType, TObjectPtr<UItemDefinition>> AmmoItemDefinitions;

    /**
     *  Soins disponibles — 1 tiré au hasard si aucune munition matchée.
     *  Chaque entrée = classe BP + DA (mesh, icône, nom).
     *  Ex : { BP_HealthPickup, DA_Item_Health_Small }, { BP_HealthPickup, DA_Item_Health_Large }
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Loot")
    TArray<FHealthDropEntry> HealthDrops;

    /** Ancien champ unique — ignoré si HealthFallbackClasses n'est pas vide */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Loot")
    TSubclassOf<AActor> HealthFallbackClass;

    // ── DEBRIS / FRAGMENTATION ─────────────────────────────────────────────

    /**
     *  Liste de meshes "morceaux" spawnés à la destruction.
     *  Chaque entrée devient un StaticMeshActor avec physique simulée + impulse.
     *  Ex : 4-8 fragments de planches/morceaux de bois.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Debris")
    TArray<TObjectPtr<UStaticMesh>> DebrisMeshes;

    /** Nombre de morceaux à spawner (sélectionnés aléatoirement dans DebrisMeshes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Debris",
              meta = (ClampMin = 0, ClampMax = 30))
    int32 DebrisCount = 6;

    /** Force d'explosion appliquée aux morceaux (impulsion radiale) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Debris",
              meta = (ClampMin = 0.f, ClampMax = 2000.f))
    float DebrisImpulseStrength = 500.f;

    /** Durée de vie des morceaux avant disparition (secondes) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Debris",
              meta = (ClampMin = 1.f, ClampMax = 30.f))
    float DebrisLifetime = 5.f;

    /** Échelle appliquée aux morceaux */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Debris")
    float DebrisScale = 0.5f;

    /**
     *  Délai avant Destroy() de l'acteur — laisse le temps aux fragments Chaos
     *  ou aux debris meshes de bouger avant que la caisse disparaisse.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Destructible|Debris",
              meta = (ClampMin = 0.f, ClampMax = 30.f))
    float DestroyDelay = 5.f;

    void SpawnDebris();

    // ── LOGIQUE INTERNE ────────────────────────────────────────────────────

    UFUNCTION()
    void HandleDeath(UHealthComponent* HealthComponent,
                     const AController* InstigatedBy, AActor* DamageCauser);

    UFUNCTION()
    void HandleDamage(UHealthComponent* HealthComponent, float Health,
                      float HealthDelta, const AController* InstigatedBy);

    void SpawnLoot();
    void ApplyExplosionDamage(AController* InstigatedBy);

    // ── HOOKS BLUEPRINT ────────────────────────────────────────────────────

    /** Effets de destruction (particules, son, décombres) */
    UFUNCTION(BlueprintImplementableEvent, Category = "Destructible",
              meta = (DisplayName = "On Destroyed"))
    void BP_OnDestroyed();

    /** Appelé quand HP passe sous DamagedThreshold (afficher cracks, sons, etc.) */
    UFUNCTION(BlueprintImplementableEvent, Category = "Destructible",
              meta = (DisplayName = "On Damaged State"))
    void BP_OnDamagedState();

public:
    UPROPERTY(BlueprintAssignable, Category = "Destructible|Events")
    FOnObjectDestroyed OnObjectDestroyed;
};
