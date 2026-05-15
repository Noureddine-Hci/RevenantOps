import unreal

BASE = '/Game/_WeaponsPacks/MilitaryWeapSilver/Weapons'

WEAPONS = [
    ('/Game/Mercenaires/Weapons/Pistol/BP_Pistol',             'Pistol',       'Pistols_A'),
    ('/Game/Mercenaires/Weapons/Fusil_assaut/BP_AssaultRifle', 'AssaultRifle', 'Assault_Rifle_A'),
    ('/Game/Mercenaires/Weapons/Shotgun/BP_Shotgun',           'Shotgun',      'Shotgun_A'),
    ('/Game/Mercenaires/Weapons/Sniper/BP_Sniper',             'Sniper',       'Sniper_Rifle_A'),
]

for bp_path, label, mesh_name in WEAPONS:
    mesh = unreal.load_asset(f'{BASE}/{mesh_name}')
    bp = unreal.load_asset(bp_path)

    if not mesh or not bp:
        print(f'{label}: NOT FOUND')
        continue

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())

    comps = cdo.get_components_by_class(unreal.SkeletalMeshComponent)
    skm = next((c for c in comps if 'WeaponMesh' in c.get_name()), comps[0] if comps else None)

    if skm:
        with unreal.ScopedEditorTransaction(f'Silver {label}') as _:
            bp.modify()
            cdo.modify()
            skm.modify()
            skm.set_editor_property('SkeletalMesh', mesh)
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        unreal.EditorAssetLibrary.save_asset(bp_path, only_if_is_dirty=False)
        print(f'{label}: OK -> {mesh_name}')
    else:
        print(f'{label}: no SkeletalMeshComponent (comps={len(comps)})')
