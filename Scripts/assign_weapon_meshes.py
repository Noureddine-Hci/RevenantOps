import unreal, json

BASE = '/Game/Mercenaires/Weapons/Meshes'

# (bp_path, label, mesh_asset_name)
WEAPONS = [
    ('/Game/Mercenaires/Weapons/BP_Pistol',       'Pistol',       'blaster-a'),
    ('/Game/Mercenaires/Weapons/BP_AssaultRifle', 'AssaultRifle', 'blaster-e'),
    ('/Game/Mercenaires/Weapons/BP_SMG',          'SMG',          'blaster-c'),
    ('/Game/Mercenaires/Weapons/BP_Shotgun',      'Shotgun',      'blaster-g'),
    ('/Game/Mercenaires/Weapons/BP_Sniper',       'Sniper',       'blaster-m'),
    ('/Game/Mercenaires/Weapons/BP_Melee',        'Melee',        'grenade-a'),
]

results = []

for (bp_path, label, mesh_name) in WEAPONS:
    try:
        mesh = unreal.load_asset(f'{BASE}/{mesh_name}')
        if not mesh:
            results.append({'weapon': label, 'status': 'mesh_not_found'})
            continue

        bp = unreal.load_asset(bp_path)
        if not bp:
            results.append({'weapon': label, 'status': 'bp_not_found'})
            continue

        # Pattern Phase 11 : CDO via generated_class()
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        cdo = unreal.get_default_object(bp.generated_class())

        # Trouver le composant WeaponMeshSM (natif C++)
        sm_comps = cdo.get_components_by_class(unreal.StaticMeshComponent)
        weapon_sm = next((c for c in sm_comps if 'WeaponMeshSM' in c.get_name()), None)
        if weapon_sm is None and sm_comps:
            weapon_sm = sm_comps[0]  # fallback premier SMC dispo

        if weapon_sm:
            with unreal.ScopedEditorTransaction(f'Assign {label} mesh') as _:
                bp.modify()
                cdo.modify()
                weapon_sm.modify()
                weapon_sm.set_editor_property('StaticMesh', mesh)
            unreal.BlueprintEditorLibrary.compile_blueprint(bp)
            unreal.EditorAssetLibrary.save_asset(bp_path, only_if_is_dirty=False)
            results.append({'weapon': label, 'status': 'ok', 'mesh': mesh_name,
                            'comp': weapon_sm.get_name()})
        else:
            results.append({'weapon': label, 'status': 'no_sm_component',
                            'sm_comps_found': len(sm_comps)})

    except Exception as e:
        results.append({'weapon': label, 'status': 'error', 'error': str(e)[:200]})

print(json.dumps({'results': results}))
