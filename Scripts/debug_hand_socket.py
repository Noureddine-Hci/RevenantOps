import unreal

# hand_r socket sur le mesh du perso
mesh = unreal.load_asset('/Game/Characters/Mannequins/Meshes/SK_Mannequin')
for name in ['hand_r', 'hand_l', 'weapon', 'grip']:
    s = mesh.find_socket(name)
    if s:
        loc = s.get_editor_property('RelativeLocation')
        rot = s.get_editor_property('RelativeRotation')
        print(f'Perso [{name}]: loc=({loc.x:.1f},{loc.y:.1f},{loc.z:.1f}) rot=(p={rot.pitch:.1f},y={rot.yaw:.1f},r={rot.roll:.1f})')

# grip socket sur le Shotgun
mesh2 = unreal.load_asset('/Game/_WeaponsPacks/MilitaryWeapSilver/Weapons/Shotgun_A')
for name in ['grip', 'Grip_Bone', 'Muzzle']:
    s = mesh2.find_socket(name)
    if s:
        loc = s.get_editor_property('RelativeLocation')
        rot = s.get_editor_property('RelativeRotation')
        print(f'Shotgun [{name}]: loc=({loc.x:.1f},{loc.y:.1f},{loc.z:.1f}) rot=(p={rot.pitch:.1f},y={rot.yaw:.1f},r={rot.roll:.1f})')
