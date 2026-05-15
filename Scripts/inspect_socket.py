import unreal

skel = unreal.load_asset('/Game/Characters/Mannequins/Meshes/SK_Mannequin')
if not skel:
    print('Skeleton NOT FOUND')
else:
    sockets = skel.get_editor_property('Sockets')
    for s in sockets:
        name = s.get_editor_property('SocketName')
        loc  = s.get_editor_property('RelativeLocation')
        rot  = s.get_editor_property('RelativeRotation')
        sc   = s.get_editor_property('RelativeScale')
        print(f'{name}: loc={loc} rot={rot} scale={sc}')
