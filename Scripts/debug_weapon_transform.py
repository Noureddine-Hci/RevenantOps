import unreal

WEAPONS = [
    ('/Game/Mercenaires/Weapons/Pistol/BP_Pistol',             'Pistol'),
    ('/Game/Mercenaires/Weapons/Fusil_assaut/BP_AssaultRifle', 'AssaultRifle'),
    ('/Game/Mercenaires/Weapons/Shotgun/BP_Shotgun',           'Shotgun'),
]

for bp_path, label in WEAPONS:
    bp = unreal.load_asset(bp_path)
    if not bp:
        print(f'{label}: NOT FOUND')
        continue

    unreal.BlueprintEditorLibrary.compile_blueprint(bp)
    cdo = unreal.get_default_object(bp.generated_class())

    skm_comps = cdo.get_components_by_class(unreal.SkeletalMeshComponent)
    print(f'\n{label}: {len(skm_comps)} SkeletalMeshComponent(s)')

    for c in skm_comps:
        name = c.get_name()
        mesh = c.get_editor_property('SkeletalMesh')
        loc  = c.get_editor_property('RelativeLocation')
        rot  = c.get_editor_property('RelativeRotation')
        sc   = c.get_editor_property('RelativeScale3D')
        print(f'  [{name}] mesh={mesh} loc={loc} rot={rot} scale={sc}')
