import unreal, json

MON = '/Game/Mercenaires/Anims/Montages'
PISTOL_FIRE = '/Game/Characters/Mannequins/Anims/Pistol/MM_Pistol_Fire_Montage'

# (bp_path, label, fire_path, reload_path, equip_path)
WEAPONS = [
    ('/Game/Mercenaires/Weapons/BP_Pistol',
     'Pistol',
     PISTOL_FIRE,
     f'{MON}/AM_Pistol_Reload',
     f'{MON}/AM_Pistol_Equip'),

    ('/Game/Mercenaires/Weapons/BP_AssaultRifle',
     'AssaultRifle',
     f'{MON}/AM_Rifle_Fire',
     f'{MON}/AM_Rifle_Reload',
     f'{MON}/AM_Rifle_Equip'),

    ('/Game/Mercenaires/Weapons/BP_SMG',
     'SMG',
     f'{MON}/AM_Rifle_Fire',
     f'{MON}/AM_Rifle_Reload',
     f'{MON}/AM_Rifle_Equip'),

    ('/Game/Mercenaires/Weapons/BP_Shotgun',
     'Shotgun',
     f'{MON}/AM_Rifle_Fire',
     f'{MON}/AM_Rifle_Reload',
     f'{MON}/AM_Rifle_Equip'),

    ('/Game/Mercenaires/Weapons/BP_Sniper',
     'Sniper',
     f'{MON}/AM_Rifle_Fire',
     f'{MON}/AM_Rifle_Reload',
     f'{MON}/AM_Rifle_Equip'),

    ('/Game/Mercenaires/Weapons/BP_Melee',
     'Melee',
     None,
     None,
     f'{MON}/AM_Pistol_Equip'),
]

results = []

for (bp_path, label, fire_p, reload_p, equip_p) in WEAPONS:
    try:
        bp = unreal.load_asset(bp_path)
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        cdo = unreal.get_default_object(bp.generated_class())

        fire_m   = unreal.load_asset(fire_p)   if fire_p   else None
        reload_m = unreal.load_asset(reload_p) if reload_p else None
        equip_m  = unreal.load_asset(equip_p)  if equip_p  else None

        with unreal.ScopedEditorTransaction(f'Assign montages {label}') as _:
            bp.modify()
            cdo.modify()
            if fire_m:
                cdo.set_editor_property('CharacterFireMontage', fire_m)
            if reload_m:
                cdo.set_editor_property('ReloadMontage', reload_m)
            if equip_m:
                cdo.set_editor_property('EquipMontage', equip_m)

        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        unreal.EditorAssetLibrary.save_asset(bp_path, only_if_is_dirty=False)

        results.append({
            'weapon': label,
            'status': 'ok',
            'fire':   fire_m.get_name()   if fire_m   else None,
            'reload': reload_m.get_name() if reload_m else None,
            'equip':  equip_m.get_name()  if equip_m  else None,
        })

    except Exception as e:
        results.append({'weapon': label, 'status': 'error', 'error': str(e)[:150]})

print(json.dumps({'results': results}, indent=2))
