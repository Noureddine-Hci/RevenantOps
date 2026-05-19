"""
assign_sounds.py — Sound design complet RevenantOps
Assigne FireSound/ReloadSound/EmptySound sur les weapon BPs
et DeathSound/HitSound/AmbientSound sur les enemy BPs.
"""
import unreal

SFX = '/Game/Mercenaires/Audio/SFX'
AUDIO_ROOT = '/Game/Mercenaires/Audio'
ZBP = '/Game/Audio/Zombies'
FLESH = '/Game/Audio/Impacts/Flesh'

# ──────────────────────────────────────────────────────────────────────────────
# WEAPON BPS
# (bp_path, label, fire_path, reload_path, empty_path)
# ──────────────────────────────────────────────────────────────────────────────
WEAPON_SOUNDS = [
    (
        '/Game/Mercenaires/Weapons/Pistol/BP_Pistol',
        'Pistol',
        f'{SFX}/SW_Fire_Light',
        f'{SFX}/SW_Reload_Pistol',
        f'{AUDIO_ROOT}/SW_Empty',
    ),
    (
        '/Game/Mercenaires/Weapons/Fusil_assaut/BP_AssaultRifle',
        'AssaultRifle',
        f'{SFX}/SW_Fire_Heavy',
        f'{SFX}/SW_Reload_Rifle',
        f'{AUDIO_ROOT}/SW_Empty',
    ),
    (
        '/Game/Mercenaires/Weapons/Shotgun/BP_Shotgun',
        'Shotgun',
        f'{SFX}/SW_Fire_Heavy',
        f'{AUDIO_ROOT}/SW_Reload',
        f'{AUDIO_ROOT}/SW_Empty',
    ),
    (
        '/Game/Mercenaires/Weapons/Sniper/BP_Sniper',
        'Sniper',
        f'{SFX}/SW_Fire_Heavy',
        f'{SFX}/SW_Reload_Rifle',
        f'{AUDIO_ROOT}/SW_Empty',
    ),
    (
        '/Game/Mercenaires/Weapons/SMG/BP_SMG',
        'SMG',
        f'{SFX}/SW_Fire_Light',
        f'{SFX}/SW_Reload_Pistol',
        f'{AUDIO_ROOT}/SW_Empty',
    ),
]

# ──────────────────────────────────────────────────────────────────────────────
# ENEMY BPS
# (bp_path, label, death_path, hit_path, ambient_path)
# ──────────────────────────────────────────────────────────────────────────────
GENERIC_DEATH = f'{ZBP}/Death/zombie_death_gurgle___1-1778537208292'
TANK_DEATH    = f'{ZBP}/Death/zombie_death_gurgle___4-1778536782403'
FLESH_HIT     = f'{FLESH}/impactSoft_medium_000'

ENEMY_SOUNDS = [
    (
        '/Game/Mercenaires/Zombies/BP_ZombieSlow',
        'ZombieSlow',
        GENERIC_DEATH,
        FLESH_HIT,
        f'{ZBP}/Idle/zombie_idle_moan__lo__1-1778532173625',
    ),
    (
        '/Game/Mercenaires/Zombies/BP_ZombieRunner',
        'ZombieRunner',
        GENERIC_DEATH,
        FLESH_HIT,
        f'{ZBP}/Attack/zombie_runner_shriek__1-1778536443981',
    ),
    (
        '/Game/Mercenaires/Zombies/BP_ZombieTank',
        'ZombieTank',
        TANK_DEATH,
        FLESH_HIT,
        f'{ZBP}/Attack/zombie_tank_deep_bas__3-1778536710513',
    ),
    (
        '/Game/Mercenaires/Zombies/BP_ZombieExploder',
        'ZombieExploder',
        GENERIC_DEATH,
        FLESH_HIT,
        f'{ZBP}/Alert/zombie_alert_sound____1-1778533029232',
    ),
]

# ──────────────────────────────────────────────────────────────────────────────
# HELPERS
# ──────────────────────────────────────────────────────────────────────────────
def load_sound(path):
    snd = unreal.load_asset(path)
    if snd is None:
        print(f'  [WARN] Son introuvable : {path}')
    return snd


def assign_cdo_props(bp_path, label, props: dict):
    """Set multiple editor properties on a BP CDO."""
    bp = unreal.load_asset(bp_path)
    if bp is None:
        print(f'[ERROR] BP introuvable : {bp_path}')
        return False

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())

    with unreal.ScopedEditorTransaction(f'Assign Sounds {label}') as _:
        bp.modify()
        cdo.modify()
        for prop_name, value in props.items():
            if value is not None:
                cdo.set_editor_property(prop_name, value)

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(bp_path, only_if_is_dirty=False)
    return True


# ──────────────────────────────────────────────────────────────────────────────
# ASSIGN WEAPON SOUNDS
# ──────────────────────────────────────────────────────────────────────────────
print('\n=== WEAPON SOUNDS ===')
for bp_path, label, fire, reload, empty in WEAPON_SOUNDS:
    props = {
        'FireSound':   load_sound(fire),
        'ReloadSound': load_sound(reload),
        'EmptySound':  load_sound(empty),
    }
    ok = assign_cdo_props(bp_path, label, props)
    status = 'OK' if ok else 'FAIL'
    print(f'{label}: {status}')

# ──────────────────────────────────────────────────────────────────────────────
# ASSIGN ENEMY SOUNDS
# ──────────────────────────────────────────────────────────────────────────────
print('\n=== ENEMY SOUNDS ===')
for bp_path, label, death, hit, ambient in ENEMY_SOUNDS:
    props = {
        'DeathSound':   load_sound(death),
        'HitSound':     load_sound(hit),
        'AmbientSound': load_sound(ambient),
    }
    ok = assign_cdo_props(bp_path, label, props)
    status = 'OK' if ok else 'FAIL'
    print(f'{label}: {status}')

print('\n=== DONE ===')
