# Phase 11: DataTables - Context

**Gathered:** 2026-03-29
**Status:** Ready for planning
**Source:** Auto-generated from STATE.md + code analysis (user opted to skip discuss-phase)

<domain>
## Phase Boundary

Les stats des armes et des ennemis sont éditables via DataTable dans l'éditeur UE5 et s'appliquent au runtime sans recompiler. Périmètre : C++ structs + DT assets + lecture BeginPlay dans WeaponBase et ZombieBase. Aucun nouveau gameplay, aucune UI de stats.

</domain>

<decisions>
## Implementation Decisions

### Structs C++ DataTable

- **D-01:** `FWeaponTableRow : public FTableRowBase` défini dans `Weapons/WeaponTableRow.h` avec champs : `float Damage`, `float FireRate`, `int32 MaxAmmo`, `float Range`
- **D-02:** `FEnemyTableRow : public FTableRowBase` défini dans `AI/EnemyTableRow.h` avec champs : `float MaxHP`, `float MeleeDamage`, `float MovementSpeed`
- **D-03:** Les structs sont dans des headers dédiés (pas dans WeaponBase.h / EnemyBase.h) pour éviter les circular includes
- **D-04:** Les deux headers incluent uniquement `Engine/DataTable.h` — aucun autre include gameplay

### Identification de la ligne DT (Row Key Strategy)

- **D-05:** Utiliser `UPROPERTY(EditDefaultsOnly) FDataTableRowHandle WeaponDataRow` dans WeaponBase — bundle DT asset + row name dans un seul UPROPERTY, configurable dans le Details panel de chaque BP arme
- **D-06:** Utiliser `UPROPERTY(EditDefaultsOnly) FDataTableRowHandle EnemyDataRow` dans ZombieBase (pas EnemyBase) — car MeleeDamage n'existe que dans ZombieBase
- **D-07:** Chaque BP arme (BP_Pistol, etc.) configure WeaponDataRow = {DT_WeaponStats, "Pistol"} dans ses Defaults. Row names : "Pistol", "AssaultRifle", "SMG", "Shotgun", "Sniper", "Melee"
- **D-08:** Chaque BP zombie configure EnemyDataRow = {DT_EnemyStats, "Slow"} etc. Row names : "Slow", "Runner", "Tank", "Spitter", "Exploder"

### Lecture des stats au BeginPlay

- **D-09:** `WeaponBase::BeginPlay()` appelle `ApplyWeaponDataRow()` après `Super::BeginPlay()` — lit BaseDamage, FireRate, MagazineSize, MaxRange depuis la DT row
- **D-10:** `ZombieBase::BeginPlay()` appelle `ApplyEnemyDataRow()` après `Super::BeginPlay()` — lit MaxHP (via HealthComp->SetMaxHealth), MeleeDamage, MaxWalkSpeed (via GetCharacterMovement())
- **D-11:** Champs mappés pour armes : `Damage` → `BaseDamage`, `FireRate` → `FireRate`, `MaxAmmo` → `MagazineSize` et initialise `CurrentAmmo`, `Range` → `MaxRange`
- **D-12:** Champs mappés pour ennemis : `MaxHP` → `HealthComp->SetMaxHealth() + ResetHealth()`, `MeleeDamage` → `this->MeleeDamage`, `MovementSpeed` → `GetCharacterMovement()->MaxWalkSpeed`

### Fallback si ligne manquante

- **D-13:** Si le FDataTableRowHandle est invalide (DataTable non assignée ou RowName introuvable) → `UE_LOG(LogTemp, Warning, ...)` + conserver les valeurs du constructeur C++. Pas de crash, pas d'ensure. Comportement de jeu préservé avec les defaults de code.

### Scope des stats DataTable

- **D-14:** Seuls les champs balance (damage, fireRate, ammo, range pour armes; HP, damage, speed pour ennemis) vont en DT. Les paramètres de "feel" (recoil, spread, ADS, ReloadTime, MeleeRange, MeleeAttackCooldown) restent en `EditDefaultsOnly` dans les BPs — ils ne font pas partie du scope DATA-01/02/03/04.

### Assets DataTable dans l'éditeur

- **D-15:** Créer `/Game/Mercenaires/Data/DT_WeaponStats` (row struct : FWeaponTableRow) et `/Game/Mercenaires/Data/DT_EnemyStats` (row struct : FEnemyTableRow) — dossier Data/ à créer si inexistant
- **D-16:** Peupler DT_WeaponStats avec 6 lignes (valeurs initiales dérivées des constructors C++ existants) : Pistol(20,300,12,8000), AssaultRifle(15,600,30,12000), SMG(12,900,40,8000), Shotgun(50,60,8,3000), Sniper(120,45,5,50000), Melee(35,0,0,150)
- **D-17:** Peupler DT_EnemyStats avec 5 lignes : Slow(100,10,200), Runner(60,12,550), Tank(400,25,180), Spitter(150,15,300), Exploder(80,80,350)

### Claude's Discretion
- Nom exact des fonctions helper (ApplyWeaponDataRow, ApplyEnemyDataRow)
- Header guards et includes exact
- Ordre des includes dans Build.cs (Engine module déjà présent)

</decisions>

<specifics>
## Specific Ideas

- STATE.md note : "DataTables UE5 nécessitent FTableRowBase C++ struct + recompilation avant création des DT assets dans l'éditeur" — le plan doit ordonner : C++ + build AVANT création des DT assets
- Valeurs initiales DT = valeurs des constructors C++ actuels (pas de changement de gameplay, migration propre)
- MagazineSize=0 et FireRate=0 pour Melee dans DT_WeaponStats sont ignorés au BeginPlay (WeaponMelee override FireShot avec sphere trace, pas de magazine)

</specifics>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Code à modifier
- `Source/RevenantOps/Weapons/WeaponBase.h` — structure actuelle WeaponBase, champs à mapper depuis DT
- `Source/RevenantOps/Weapons/WeaponBase.cpp` — BeginPlay actuel à modifier
- `Source/RevenantOps/AI/ZombieBase.h` — MeleeDamage field, BeginPlay à modifier
- `Source/RevenantOps/AI/ZombieBase.cpp` — BeginPlay actuel
- `Source/RevenantOps/AI/EnemyBase.h` — HealthComponent API (SetMaxHealth, ResetHealth)

### Requirements
- `.planning/REQUIREMENTS.md` §DATA-01/02/03/04 — scope exact des DataTables v2.0

### State
- `.planning/STATE.md` §"Phase 11: DataTables" — prerequisites techniques listés

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `UHealthComponent::SetMaxHealth(float)` — setter public déjà exposé en BlueprintCallable (ligne 133 HealthComponent.h), utilisable dans ZombieBase::BeginPlay
- `UHealthComponent::ResetHealth()` — à appeler après SetMaxHealth pour synchroniser CurrentHealth
- `GetCharacterMovement()->MaxWalkSpeed` — float public, writable directement

### Established Patterns
- Constructor-defaults pattern (Phase 02) : stats dans le constructeur C++, même approche mais les defaults seront écrasés par la DT au BeginPlay
- `EWeaponCategory` enum déjà sur WeaponBase — NON utilisé pour le row key (on utilise FDataTableRowHandle à la place — plus flexible, pas de couplage enum→row name)
- WeaponBase::BeginPlay actuel : `CurrentAmmo = MagazineSize; CurrentReserveAmmo = MaxReserveAmmo;` — doit s'exécuter APRÈS ApplyWeaponDataRow() pour que CurrentAmmo = nouvelle MagazineSize DT

### Integration Points
- WeaponBase::BeginPlay() : insérer ApplyWeaponDataRow() AVANT les initialisations CurrentAmmo/CurrentReserveAmmo/CurrentSpread
- ZombieBase::BeginPlay() : insérer ApplyEnemyDataRow() APRÈS Super::BeginPlay() (qui bind les events HealthComp)
- Build.cs : module `Engine` déjà présent (contient DataTable/FTableRowBase) — aucun ajout nécessaire

</code_context>

<deferred>
## Deferred Ideas

- Stats avancées en DT (recoil, spread, ReloadTime) — hors scope DATA-01/04, éventuellement v3+
- DataTable pour les Pickups (TimeBonusPickup, AmmoBonusPickup) — hors scope Phase 11
- Hot-reload DT en cours de partie (re-lecture sans restart PIE) — hors scope, engineering non trivial

</deferred>

---

*Phase: 11-datatables*
*Context gathered: 2026-03-29 — auto-generated from STATE.md + code analysis*
