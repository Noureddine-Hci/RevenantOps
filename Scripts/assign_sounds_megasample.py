"""
assign_sounds_megasample.py
Assigne les sons MilitaryWeapSilver + Zombies sur tous les BPs.
Run via MCP TCP dans UE5 Editor (pas en PIE) :
    python Scripts/run_via_mcp.py Scripts/assign_sounds_megasample.py 120

PRÉREQUIS : avoir d'abord lancé copy_megasample_sounds.py,
puis dans UE5 Content Browser → clique droit → Rescan.
"""
import unreal

# ── Paths Silver Sound ─────────────────────────────────────────────────────────
SIL = '/Game/_WeaponsPacks/MilitaryWeapSilver/Sound'
PISTOL   = f'{SIL}/Pistol/Cues'
RIFLE    = f'{SIL}/Rifle/Cues'
SHOTGUN  = f'{SIL}/Shotgun/Cues'
SNIPER   = f'{SIL}/SniperRifle/Cues'
KNIFE    = f'{SIL}/Knife/Cues'

# Son Empty générique (déjà dans le projet)
EMPTY = '/Game/Mercenaires/Audio/SW_Empty'

# ── Paths Zombies (déjà importés) ──────────────────────────────────────────────
ZBP   = '/Game/Audio/Zombies'
FLESH = '/Game/Audio/Impacts/Flesh'

GENERIC_DEATH = f'{ZBP}/Death/zombie_death_gurgle___1-1778537208292'
TANK_DEATH    = f'{ZBP}/Death/zombie_death_gurgle___4-1778536782403'
FLESH_HIT     = f'{FLESH}/impactSoft_medium_000'

# ── Helpers ────────────────────────────────────────────────────────────────────
def load_sound(path):
    snd = unreal.load_asset(path)
    if snd is None:
        print(f'  [WARN] Son introuvable : {path}')
    return snd


def assign_cdo(bp_path, label, props: dict):
    bp = unreal.load_asset(bp_path)
    if bp is None:
        print(f'[ERROR] BP introuvable : {bp_path}')
        return False
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())
    with unreal.ScopedEditorTransaction(f'Sound {label}') as _:
        bp.modify()
        cdo.modify()
        for prop, val in props.items():
            if val is not None:
                cdo.set_editor_property(prop, val)
    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(bp_path, only_if_is_dirty=False)
    return True


# ── Scan UE5 pour détecter les nouveaux assets ────────────────────────────────
print('Scan des nouveaux assets...')
reg = unreal.AssetRegistryHelpers.get_asset_registry()
reg.scan_paths_synchronous(['/Game/_WeaponsPacks/MilitaryWeapSilver/Sound'], True)
print('Scan terminé.\n')

# ── WEAPON BPS ─────────────────────────────────────────────────────────────────
# (bp_path, label, fire_cue, reload_cue, empty_path)
WEAPONS = [
    (
        '/Game/Mercenaires/Weapons/Pistol/BP_Pistol', 'Pistol',
        f'{PISTOL}/PistolA_Fire_Cue',
        f'{PISTOL}/Pistol_ReloadEject_Cue',
        EMPTY,
    ),
    (
        '/Game/Mercenaires/Weapons/Fusil_assaut/BP_AssaultRifle', 'AssaultRifle',
        f'{RIFLE}/RifleA_Fire_Cue',
        f'{RIFLE}/Rifle_Reload_Cue',
        EMPTY,
    ),
    (
        '/Game/Mercenaires/Weapons/Shotgun/BP_Shotgun', 'Shotgun',
        f'{SHOTGUN}/ShotgunA_Fire_Cue',
        f'{SHOTGUN}/Shotgun_Reload_Cue',
        EMPTY,
    ),
    (
        '/Game/Mercenaires/Weapons/Sniper/BP_Sniper', 'Sniper',
        f'{SNIPER}/SniperRifleA_Fire_Cue',
        f'{SNIPER}/SniperRifle_Reload_Cue',
        EMPTY,
    ),
    (
        '/Game/Mercenaires/Weapons/SMG/BP_SMG', 'SMG',
        # Pas de SMG dans le pack — Rifle (light) est le plus proche
        f'{RIFLE}/RifleA_Fire_Cue',
        f'{RIFLE}/Rifle_Reload_Cue',
        EMPTY,
    ),
    (
        '/Game/Mercenaires/Weapons/BP_Melee', 'Melee',
        # Swing = "attaque" pour le Melee
        f'{KNIFE}/KnifeA_Swing_Cue',
        None,   # pas de reload sur une arme mêlée
        None,
    ),
]

print('=== WEAPON SOUNDS ===')
for bp_path, label, fire, reload, empty in WEAPONS:
    props = {
        'FireSound':   load_sound(fire),
        'ReloadSound': load_sound(reload) if reload else None,
        'EmptySound':  load_sound(empty) if empty else None,
    }
    # Retirer les None pour ne pas écraser avec None
    props = {k: v for k, v in props.items() if v is not None}
    ok = assign_cdo(bp_path, label, props)
    print(f'{label}: {"OK" if ok else "FAIL"}')

# ── ENEMY BPS ──────────────────────────────────────────────────────────────────
ENEMIES = [
    (
        '/Game/Mercenaires/Zombies/BP_ZombieSlow', 'ZombieSlow',
        GENERIC_DEATH, FLESH_HIT,
        f'{ZBP}/Idle/zombie_idle_moan__lo__1-1778532173625',
    ),
    (
        '/Game/Mercenaires/Zombies/BP_ZombieRunner', 'ZombieRunner',
        GENERIC_DEATH, FLESH_HIT,
        f'{ZBP}/Attack/zombie_runner_shriek__1-1778536443981',
    ),
    (
        '/Game/Mercenaires/Zombies/BP_ZombieTank', 'ZombieTank',
        TANK_DEATH, FLESH_HIT,
        f'{ZBP}/Attack/zombie_tank_deep_bas__3-1778536710513',
    ),
    (
        '/Game/Mercenaires/Zombies/BP_ZombieExploder', 'ZombieExploder',
        GENERIC_DEATH, FLESH_HIT,
        f'{ZBP}/Alert/zombie_alert_sound____1-1778533029232',
    ),
]

print('\n=== ENEMY SOUNDS ===')
for bp_path, label, death, hit, ambient in ENEMIES:
    props = {
        'DeathSound':   load_sound(death),
        'HitSound':     load_sound(hit),
        'AmbientSound': load_sound(ambient),
    }
    props = {k: v for k, v in props.items() if v is not None}
    ok = assign_cdo(bp_path, label, props)
    print(f'{label}: {"OK" if ok else "FAIL"}')

print('\n=== DONE ===')
