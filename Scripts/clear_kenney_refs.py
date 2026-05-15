import unreal

WEAPONS = [
    '/Game/Mercenaires/Weapons/Pistol/BP_Pistol',
    '/Game/Mercenaires/Weapons/Fusil_assaut/BP_AssaultRifle',
    '/Game/Mercenaires/Weapons/Shotgun/BP_Shotgun',
    '/Game/Mercenaires/Weapons/Sniper/BP_Sniper',
    '/Game/Mercenaires/Weapons/SMG/BP_SMG',
    '/Game/Mercenaires/Weapons/BP_Melee',
]

for bp_path in WEAPONS:
    bp = unreal.load_asset(bp_path)
    if not bp:
        print(f'NOT FOUND: {bp_path}')
        continue

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())

    sm_comps = cdo.get_components_by_class(unreal.StaticMeshComponent)
    if not sm_comps:
        print(f'{bp_path.split("/")[-1]}: no StaticMeshComponent — skip')
        continue

    with unreal.ScopedEditorTransaction(f'Clear SM {bp_path}') as _:
        bp.modify()
        cdo.modify()
        for c in sm_comps:
            c.modify()
            c.set_editor_property('StaticMesh', None)

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(bp_path, only_if_is_dirty=False)
    print(f'{bp_path.split("/")[-1]}: cleared {len(sm_comps)} StaticMeshComponent(s)')
