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

    # Try snake_case property names (UE Python convention)
    results = {}
    for prop in ['weapon_name', 'base_damage', 'fire_rate', 'magazine_size',
                 'weapon_category', 'fire_mode', 'pellets_per_shot']:
        try:
            val = cdo.get_editor_property(prop)
            results[prop] = str(val)
        except Exception as e:
            results[prop] = f'FAIL: {e}'

    # Also try accessing as attribute
    attr_results = {}
    for attr in ['weapon_name', 'base_damage', 'fire_rate', 'magazine_size']:
        try:
            val = getattr(cdo, attr)
            attr_results[attr] = str(val)
        except Exception as e:
            attr_results[attr] = f'FAIL: {e}'

    print(json.dumps({'get_editor_property': results, 'getattr': attr_results}))
except Exception as e:
    print(json.dumps({'error': str(e)}))
"""

r = ue(code)
data = get(r)
if 'error' in data:
    print(f"ERROR: {data['error']}")
else:
    print("=== get_editor_property ===")
    for k, v in data.get('get_editor_property', {}).items():
        print(f"  {k:25}: {v}")
    print("=== getattr ===")
    for k, v in data.get('getattr', {}).items():
        print(f"  {k:25}: {v}")
