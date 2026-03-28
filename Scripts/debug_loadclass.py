import socket, json

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

results = {}

# Try load_class with _C suffix
try:
    pistol_class = unreal.load_class(None, '/Game/Mercenaires/Weapons/BP_Pistol.BP_Pistol_C')
    cdo = pistol_class.get_default_object()
    results['load_class_type'] = str(type(cdo))

    # Try get_editor_property on this CDO
    for prop in ['BaseDamage', 'base_damage', 'FireRate', 'fire_rate']:
        try:
            val = cdo.get_editor_property(prop)
            results[f'prop_{prop}'] = str(val)
        except Exception as e:
            results[f'prop_{prop}'] = f'FAIL: {str(e)[:80]}'
except Exception as e:
    results['load_class_error'] = str(e)

# Try spawning actor and reading/setting properties
try:
    bp = unreal.load_asset('/Game/Mercenaires/Weapons/BP_Pistol')
    gen_class = bp.generated_class()
    world = unreal.EditorLevelLibrary.get_editor_world()
    loc = unreal.Vector(0, 0, 50000)
    rot = unreal.Rotator(0, 0, 0)
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(gen_class, loc, rot)
    results['actor_type'] = str(type(actor))

    for prop in ['BaseDamage', 'base_damage', 'FireRate', 'fire_rate']:
        try:
            val = actor.get_editor_property(prop)
            results[f'actor_{prop}'] = str(val)
        except Exception as e:
            results[f'actor_{prop}'] = f'FAIL: {str(e)[:80]}'

    unreal.EditorLevelLibrary.destroy_actor(actor)
except Exception as e:
    results['spawn_error'] = str(e)

print(json.dumps(results))
"""

raw = ue_raw(code)
print("RESPONSE:")
print(raw[:5000])
