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
    return json.loads(m.group()) if m else {'raw': raw[:2000]}

code = """
import unreal, json

try:
    bp = unreal.load_asset('/Game/Mercenaires/Weapons/BP_Pistol')
    gen_class = bp.generated_class()
    cdo = gen_class.get_default_object()

    # Try get_editor_property on known UPROPERTY names
    results = {}

    # Try different property names
    for prop in ['WeaponName', 'BaseDamage', 'FireRate', 'MagazineSize', 'WeaponCategory', 'FireMode']:
        try:
            val = cdo.get_editor_property(prop)
            results[prop] = str(val)
        except Exception as e:
            results[prop] = f'ERROR: {e}'

    # Also check weapon_mesh
    try:
        wm = cdo.weapon_mesh
        results['weapon_mesh'] = str(type(wm))
    except Exception as e:
        results['weapon_mesh'] = f'ERROR: {e}'

    # Try set_editor_property
    try:
        cdo.set_editor_property('BaseDamage', 35.0)
        val2 = cdo.get_editor_property('BaseDamage')
        results['set_test'] = f'OK: BaseDamage -> {val2}'
    except Exception as e:
        results['set_test'] = f'FAIL: {e}'

    print(json.dumps({'props': results}))
except Exception as e:
    print(json.dumps({'error': str(e)}))
"""

r = ue(code)
data = get(r)
if 'error' in data:
    print(f"ERROR: {data['error']}")
else:
    for k, v in data.get('props', {}).items():
        print(f"  {k:20}: {v}")
