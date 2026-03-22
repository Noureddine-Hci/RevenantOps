# Phase 1: Zombies - Context

**Gathered:** 2026-03-22
**Status:** Ready for planning

<domain>
## Phase Boundary

Cette phase livre 5 types de zombies avec comportements distincts (lent, rapide, tank, cracheur, explosif) et un systeme de waves progressives. Les zombies heritent de la base EnemyBase existante et s'integrent au WaveSpawner existant. Pas d'arene custom ni de score dans cette phase — uniquement les ennemis et leur spawning.

</domain>

<decisions>
## Implementation Decisions

### Architecture Zombie
- Creer AZombieBase heritant de AEnemyBase pour reutiliser perception, alert states, patrol, health integration
- Chaque type de zombie est une sous-classe de AZombieBase avec des overrides de comportement
- Les zombies melee override FireAtPlayer() pour des attaques corps-a-corps au lieu de tirer
- Le zombie cracheur utilise le systeme de projectile (spawn AProjectile au lieu de hitscan)

### Types et Stats
- Zombie Lent (ZombieSlow) : vitesse 200, HP 80, degats 10, spawn en groupes de 3-5
- Zombie Rapide (ZombieRunner) : vitesse 600, HP 50, degats 15, sprint vers le joueur
- Zombie Tank (ZombieTank) : vitesse 150, HP 300, degats 30, recul reduit
- Zombie Cracheur (ZombieSpitter) : vitesse 250, HP 100, degats 20 (projectile), engage a distance
- Zombie Explosif (ZombieExploder) : vitesse 350, HP 60, degats 50 (zone), explose a proximite et meurt

### Comportements IA
- Les zombies utilisent le systeme d'alert state existant (Idle → Alert) mais simplifie : detection joueur → charge directe
- Pas de comportement Flanker/Sniper/Defensive — les zombies sont agressifs uniquement
- Le tank a un comportement special : continue d'avancer meme sous les tirs (pas de stagger facile)
- L'explosif a une zone de detonation (rayon 300cm) et se detruit en explosant
- Le cracheur maintient sa distance (IdealEngagementRange = 1000) et tire des projectiles

### Wave System
- Reutiliser EnemyWaveSpawner existant avec les nouvelles classes zombie
- Configurer 10 waves progressives : les premieres waves = zombies lents uniquement, puis mix progressif
- MaxAliveEnemies monte progressivement (8 → 12 → 15)
- Wave 1-3 : lents uniquement, Wave 4-5 : lents + rapides, Wave 6-7 : + tanks, Wave 8-9 : + cracheurs, Wave 10 : tous types

### Claude's Discretion
- Choix des animations et montages (placeholder Mannequin disponibles)
- Implementation details du projectile du cracheur (vitesse, arc, VFX placeholder)
- Noms exacts des sockets d'attaque melee
- Valeurs exactes de tuning (ajustables plus tard)

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `AEnemyBase` (AI/EnemyBase.h) : Classe de base complete avec perception, alert states, combat, patrol, health integration, ragdoll death
- `AEnemyWaveSpawner` (AI/EnemyWaveSpawner.h) : Systeme de waves complet avec FWaveEnemyEntry, trigger volume, MaxAliveEnemies, infinite mode
- `UHealthComponent` (Weapons/HealthComponent.h) : Systeme de vie avec shield, regen, damage multiplier, events OnHealthChanged/OnDeath
- `AWeaponBase` (Weapons/WeaponBase.h) : Systeme d'armes complet — les zombies n'en ont pas besoin mais le damage system est compatible
- `ACombatEnemy` (Variant_Combat) : Exemple de melee combat avec combo/charged attacks — inspiration pour les attaques zombie melee

### Established Patterns
- Delegates multicast pour la mort (FOnEnemyBaseDied) — le spawner ecoute
- Blueprint hooks (BP_OnDeath, BP_OnDamageReceived) pour les effets visuels
- UGameplayStatics::ApplyDamage() pour infliger des degats
- HealthComponent ecoute OnTakeAnyDamage automatiquement

### Integration Points
- EnemyWaveSpawner::SpawnWaveEntry() spawn les ennemis — il suffit de configurer les TSubclassOf avec les classes zombie
- EnemyBase::HandleDeath() broadcast FOnEnemyBaseDied — le spawner track deja les morts
- Le joueur (RevenantOpsCharacter) a deja un HealthComponent pour recevoir les degats zombie

</code_context>

<specifics>
## Specific Ideas

- Reference directe : mode Mercenaires RE4 — les zombies sont agressifs, pas subtils
- Les zombies lents forment des hordes (spawn en groupe) pour creer la pression
- Le zombie explosif doit etre visuellement distinct (a defaut de modele custom, utiliser une echelle differente ou un material)
- Les waves doivent monter en difficulte naturellement sans pic brutal

</specifics>

<deferred>
## Deferred Ideas

- Sons et VFX zombie (Phase 8: Audio & VFX)
- Score par type de zombie (Phase 3: Timer & Score)
- Placement des spawn points dans l'arene (Phase 4: Arene)
- Animations custom zombie (post-v1, assets custom)

</deferred>
