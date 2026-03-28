import socket, json, re

def ue_raw(code, timeout=60):
    cmd = json.dumps({'type': 'python', 'code': code})
    with socket.create_connection(('127.0.0.1', 12029), timeout=timeout) as s:
        s.sendall(cmd.encode())
        buf = b''
        while True:
            c = s.recv(16384)
            if not c: break
            buf += c
    return buf.decode()

code = """
import unreal, json

weapons = [
    ('BP_Pistol',       '/Game/Mercenaires/Weapons/BP_Pistol'),
    ('BP_AssaultRifle', '/Game/Mercenaires/Weapons/BP_AssaultRifle'),
    ('BP_SMG',          '/Game/Mercenaires/Weapons/BP_SMG'),
    ('BP_Shotgun',      '/Game/Mercenaires/Weapons/BP_Shotgun'),
    ('BP_Sniper',       '/Game/Mercenaires/Weapons/BP_Sniper'),
    ('BP_Melee',        '/Game/Mercenaires/Weapons/BP_Melee'),
]

results = {}
z_offset = 0

for (name, path) in weapons:
    try:
        bp = unreal.load_asset(path)
        gen_class = bp.generated_class()
        loc = unreal.Vector(0, 0, 50000 + z_offset)
        rot = unreal.Rotator(0, 0, 0)
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(gen_class, loc, rot)
        z_offset += 500

        props = {}
        for prop in ['BaseDamage', 'FireRate', 'MagazineSize', 'PelletsPerShot',
                     'ReloadTime', 'MaxReserveAmmo', 'WeaponCategory', 'FireMode']:
            try:
                props[prop] = str(actor.get_editor_property(prop))
            except:
                props[prop] = 'N/A'

        try:
            wm = actor.weapon_mesh
            skm = wm.get_editor_property('SkeletalMesh')
            props['has_mesh'] = str(skm) != 'None'
        except:
            props['has_mesh'] = False

        results[name] = props
        unreal.EditorLevelLibrary.destroy_actor(actor)
    except Exception as e:
        results[name] = {'error': str(e)[:100]}

print(json.dumps({'weapons': results}))
"""

raw = ue_raw(code)
# The server returns: {"success": bool, "message": str, "result": str_containing_json}
# or {"success": false, "raw_result": str_containing_json}
outer = json.loads(raw)

# Get inner JSON string
inner_str = outer.get('result', '') or outer.get('raw_result', '')

# Parse the weapon JSON from the inner string
m = re.search(r'\{"weapons".*\}', inner_str, re.DOTALL)
if m:
    data = json.loads(m.group())
    print("=== Current Weapon Stats ===")
    for weapon, props in data['weapons'].items():
        print(f"\n{weapon}:")
        if 'error' in props:
            print(f"  ERROR: {props['error']}")
        else:
            for k, v in props.items():
                print(f"  {k:20}: {v}")
else:
    print("Could not parse JSON, raw response:")
    print(inner_str[:3000])
