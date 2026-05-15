import unreal

PICKUPS = [
    '/Game/Mercenaires/Weapons/BP_AmmoBonusPickup',
    '/Game/Mercenaires/BP_AmmoBonusPickup',
    '/Game/Mercenaires/Pickups/BP_AmmoBonusPickup',
    '/Game/Mercenaires/Weapons/BP_InventoryDropPickup',
    '/Game/Mercenaires/BP_InventoryDropPickup',
    '/Game/Mercenaires/Pickups/BP_InventoryDropPickup',
]

for bp_path in PICKUPS:
    bp = unreal.load_asset(bp_path)
    if not bp:
        continue

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())

    sm_comps = cdo.get_components_by_class(unreal.StaticMeshComponent)
    cleared = 0
    if sm_comps:
        with unreal.ScopedEditorTransaction(f'Clear SM {bp_path}') as _:
            bp.modify()
            cdo.modify()
            for c in sm_comps:
                c.modify()
                c.set_editor_property('StaticMesh', None)
        cleared = len(sm_comps)

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(bp_path, only_if_is_dirty=False)
    print(f'{bp_path.split("/")[-1]}: cleared {cleared} SM comp(s)')
