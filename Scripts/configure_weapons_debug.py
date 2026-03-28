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

# Step 1: Check if enums exist with correct names
code = """
import unreal, json

# Discover enum values
cat_attrs = [a for a in dir(unreal.EWeaponCategory) if not a.startswith('_')]
fire_attrs = [a for a in dir(unreal.EWeaponFireMode) if not a.startswith('_')]

# Load one BP and inspect its CDO
bp = unreal.load_asset('/Game/Mercenaires/Weapons/BP_Pistol')
gen_class = bp.generated_class() if bp else None
cdo = gen_class.get_default_object() if gen_class else None
cdo_props = [p for p in dir(cdo) if not p.startswith('_')] if cdo else []

print(json.dumps({
    'cat_attrs': cat_attrs[:20],
    'fire_attrs': fire_attrs[:20],
    'bp_found': bp is not None,
    'cdo_found': cdo is not None,
    'cdo_has_weapon_name': 'weapon_name' in cdo_props,
    'cdo_has_base_damage': 'base_damage' in cdo_props,
    'cdo_props_sample': sorted(cdo_props)[:30],
}))
"""

r = ue(code)
raw = r.get('raw_result', r.get('result', ''))
print("RAW RESPONSE:")
print(raw[:3000])
