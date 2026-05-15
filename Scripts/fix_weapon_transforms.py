import unreal

# (bp_path, label, pitch, yaw, roll, loc_x, loc_y, loc_z)
WEAPONS = [
    ('/Game/Mercenaires/Weapons/Pistol/BP_Pistol',             'Pistol',       0, 90, -10,   0, 0, 0),
    ('/Game/Mercenaires/Weapons/Fusil_assaut/BP_AssaultRifle', 'AssaultRifle', 0, 90, -10,   0, 0, 0),
    ('/Game/Mercenaires/Weapons/Shotgun/BP_Shotgun',           'Shotgun',      0, 90, -10,   0, 0, 0),
    ('/Game/Mercenaires/Weapons/Sniper/BP_Sniper',             'Sniper',       0, 90, -10,   0, 0, 0),
    ('/Game/Mercenaires/Weapons/BP_Melee',                     'Melee',        0, 90, -10,   0, 0, 0),
]

for bp_path, label, pitch, yaw, roll, lx, ly, lz in WEAPONS:
    bp = unreal.load_asset(bp_path)
    if not bp:
        print(f'{label}: BP NOT FOUND')
        continue

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())

    offset = unreal.Transform(
        location=unreal.Vector(lx, ly, lz),
        rotation=unreal.Rotator(pitch=pitch, yaw=yaw, roll=roll),
        scale=unreal.Vector(1.0, 1.0, 1.0)
    )

    with unreal.ScopedEditorTransaction(f'Fix WeaponAttachOffset {label}') as _:
        bp.modify()
        cdo.modify()
        cdo.set_editor_property('WeaponAttachOffset', offset)

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    unreal.EditorAssetLibrary.save_asset(bp_path, only_if_is_dirty=False)
    print(f'{label}: OK — rot=({pitch},{yaw},{roll}) loc=({lx},{ly},{lz})')
