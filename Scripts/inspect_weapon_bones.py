import unreal

MESHES = [
    ('/Game/_WeaponsPacks/MilitaryWeapSilver/Weapons/Pistols_A',       'Pistol'),
    ('/Game/_WeaponsPacks/MilitaryWeapSilver/Weapons/Assault_Rifle_A', 'AssaultRifle'),
    ('/Game/_WeaponsPacks/MilitaryWeapSilver/Weapons/Shotgun_A',       'Shotgun'),
    ('/Game/_WeaponsPacks/MilitaryWeapSilver/Weapons/Sniper_Rifle_A',  'Sniper'),
    ('/Game/_WeaponsPacks/MilitaryWeapSilver/Weapons/Knife_A',         'Melee'),
]

for path, label in MESHES:
    mesh = unreal.load_asset(path)
    if not mesh:
        print(f'{label}: NOT FOUND')
        continue

    print(f'\n=== {label} ({type(mesh).__name__}) ===')

    # Tenter d'acceder aux sockets du mesh
    try:
        sockets = mesh.get_editor_property('Sockets')
        print(f'  Sockets ({len(sockets)}): {[str(s.get_editor_property("SocketName")) for s in sockets]}')
    except Exception as e:
        print(f'  Sockets: ERREUR — {e}')

    # Bones via skeleton
    try:
        skel = mesh.skeleton
        ref_skel = skel.get_editor_property('ReferenceSkeleton')
        print(f'  Bones: ERREUR acces direct')
    except Exception as e:
        pass

    # Tenter via PhysicsAsset ou autre
    try:
        lod = mesh.get_editor_property('LodSettings')
        print(f'  LOD: {lod}')
    except:
        pass
