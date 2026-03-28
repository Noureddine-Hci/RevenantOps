import socket, json, re

def ue(code, timeout=60):
    cmd = json.dumps({'type': 'python', 'code': code})
    with socket.create_connection(('127.0.0.1', 12029), timeout=timeout) as s:
        s.sendall(cmd.encode())
        buf = b''
        while True:
            c = s.recv(16384)
            if not c: break
            buf += c
    return json.loads(buf.decode())

def get(r):
    raw = r.get('raw_result', r.get('result', ''))
    m = re.search(r'\{.*\}', raw, re.DOTALL)
    return json.loads(m.group()) if m else {'raw': raw[:3000]}

code = """
import unreal, json

try:
    bp = unreal.load_asset('/Game/Mercenaires/Weapons/BP_Pistol')
    gen_class = bp.generated_class()

    # What methods does gen_class have?
    methods = [m for m in dir(gen_class) if 'default' in m.lower() or 'object' in m.lower() or 'cdo' in m.lower()]

    # Try unreal.find_object or EditorLevelLibrary to spawn temp actor
    # Method: use KismetSystemLibrary or find an existing actor of this class
    world = unreal.EditorLevelLibrary.get_editor_world()
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    weapon_actors = [str(type(a)) for a in actors if 'Weapon' in str(type(a))]

    # Try another way: use spawn_actor_from_class
    try:
        loc = unreal.Vector(0, 0, 10000)
        rot = unreal.Rotator(0, 0, 0)
        temp = unreal.EditorLevelLibrary.spawn_actor_from_class(gen_class, loc, rot)
        temp_type = str(type(temp))
        temp_props = []
        for prop in ['weapon_name', 'base_damage', 'fire_rate', 'magazine_size']:
            try:
                val = temp.get_editor_property(prop)
                temp_props.append(f'{prop}={val}')
            except Exception as e:
                temp_props.append(f'{prop}=ERROR:{e}')
        # Destroy it
        unreal.EditorLevelLibrary.destroy_actor(temp)
    except Exception as se:
        temp_type = f'SPAWN_FAIL: {se}'
        temp_props = []

    print(json.dumps({
        'gen_class_methods': methods,
        'weapon_actors_in_level': weapon_actors[:5],
        'spawn_test': temp_type,
        'temp_props': temp_props,
    }))
except Exception as e:
    print(json.dumps({'error': str(e)}))
"""

r = ue(code)
data = get(r)
if 'error' in data:
    print(f"ERROR: {data['error']}")
else:
    print("Methods with default/object/cdo:", data.get('gen_class_methods'))
    print("Weapon actors:", data.get('weapon_actors_in_level'))
    print("Spawn test:", data.get('spawn_test'))
    print("Temp props:", data.get('temp_props'))
