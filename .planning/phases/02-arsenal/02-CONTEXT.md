# Phase 2: Arsenal - Context

**Gathered:** 2026-03-22
**Status:** Ready for planning

<domain>
## Phase Boundary

Cette phase livre 5 armes a feu (Pistolet, Fusil d'Assaut, Shotgun, Sniper, SMG) avec comportements distincts et 1 arme de melee (couteau/machette), toutes switchables en jeu. Le WeaponBase existant est complet — chaque arme est une sous-classe C++ avec des valeurs par defaut differentes. La melee est un nouveau type qui utilise des sphere traces au lieu de hitscan.

</domain>

<decisions>
## Implementation Decisions

### Architecture Armes
- Chaque arme a feu est une sous-classe C++ de AWeaponBase avec des valeurs par defaut specifiques (fire mode, damage, fire rate, magazine, etc.)
- Le pistolet existant (BP_Pistol) sert de reference — les nouvelles armes suivent le meme pattern
- L'arme de melee herite aussi de AWeaponBase mais override FireShot() pour faire un sphere trace au lieu de hitscan
- Pas de munitions pour la melee (MagazineSize = 999, pas de reserve, pas de reload)

### Stats Armes (valeurs par defaut)
- Pistolet (WeaponPistol) : SemiAuto, 400 RPM, 25 dmg, mag 12, reserve 60, spread 1.5, recoil 1.0
- Fusil d'Assaut (WeaponAssaultRifle) : FullAuto, 700 RPM, 18 dmg, mag 30, reserve 120, spread 2.0, recoil 0.8
- Shotgun (WeaponShotgun) : SemiAuto, 80 RPM, 15 dmg x 8 pellets, mag 6, reserve 24, spread 5.0, reload 3.0s
- Sniper (WeaponSniper) : SemiAuto, 40 RPM, 120 dmg, mag 5, reserve 20, ADSFOV 30, spread 0.2, recoil 3.0
- SMG (WeaponSMG) : FullAuto, 1000 RPM, 12 dmg, mag 40, reserve 160, spread 3.0, recoil 0.5
- Melee (WeaponMelee) : pas de fire mode classique, 120 RPM equivalent, 35 dmg, sphere trace 100cm, pas de munitions

### Melee Design
- La melee utilise SphereTrace (rayon 50cm, distance 100cm depuis le personnage) au lieu de hitscan
- Override de FireShot() pour executer le sphere trace
- Pas de reload, pas de munitions, pas de spread/recoil
- Degats bons (35) pour compenser le risque du corps-a-corps
- La melee est toujours disponible (LOAD-02 dit "melee toujours equipee")

### Switch Armes
- Le systeme de switch existe deja (SwitchWeaponPressed cycle CurrentWeaponIndex)
- Le joueur a 3 slots : arme principale, arme secondaire, melee
- La melee est toujours en slot 3, les 2 armes a feu sont choisies au loadout (Phase 5)

### Claude's Discretion
- Valeurs exactes de tuning (ajustables plus tard via Blueprint)
- Animations (utiliser les montages Mannequin Pistol existants pour toutes les armes en placeholder)
- Noms exacts des classes Blueprint

</decisions>

<code_context>
## Existing Code Insights

### Reusable Assets
- `AWeaponBase` (Weapons/WeaponBase.h) : Systeme d'armes complet — fire modes, damage, recoil, spread, ADS, reload, hitscan
- `RevenantOpsCharacter` : Weapon inventory, SpawnDefaultWeapons(), EquipWeapon(), SwitchWeaponPressed() deja fonctionnels
- `BP_Pistol` : Blueprint existant du pistolet, template pour les nouvelles armes
- Input Actions : IA_Fire, IA_Aim, IA_Reload, IA_SwitchWeapon deja crees
- HUD : Affichage munitions, nom arme, crosshair dynamique deja branche

### Established Patterns
- WeaponBase a tous les UPROPERTY EditDefaultsOnly — les sous-classes C++ overrident dans le constructeur
- Delegates OnAmmoChanged et OnWeaponStateChanged — le HUD ecoute automatiquement
- BP hooks : BP_OnFire, BP_OnHit, BP_OnReloadStart, BP_OnReloadFinish pour VFX/SFX (Phase 8)
- Hitscan via LineTraceSingleByChannel (ECC_Visibility), damage via FPointDamageEvent

### Integration Points
- DefaultWeaponClasses dans RevenantOpsCharacter — ajouter les nouvelles armes au loadout par defaut
- EWeaponCategory enum deja defini (Pistol, SMG, AssaultRifle, Shotgun, Sniper, LMG, Launcher)
- WeaponMesh → MuzzleSocketName pour l'origine du tir

</code_context>

<specifics>
## Specific Ideas

- Chaque arme doit se SENTIR differente — le shotgun doit etre devastateur a courte portee, le sniper precis mais lent
- La melee est un choix de risque/recompense — gros degats mais il faut etre au contact des zombies
- Le switch d'arme doit etre rapide pour garder le rythme arcade

</specifics>

<deferred>
## Deferred Ideas

- VFX/SFX par arme (Phase 8: Audio & VFX)
- Selection du loadout pre-partie (Phase 5: Loadout)
- Animations custom par arme (post-v1)

</deferred>
