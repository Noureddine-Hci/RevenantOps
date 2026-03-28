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
    return json.loads(m.group()) if m else {'raw': raw[:800]}

code = """
import unreal, json

# Filtrer par classe StaticMesh
reg = unreal.AssetRegistryHelpers.get_asset_registry()
filter = unreal.ARFilter(
    class_names=['StaticMesh'],
    recursive_paths=True,
    package_paths=['/Game']
)
assets = reg.get_assets(filter)
paths = [a.object_path for a in assets]
# Filtrer ceux qui ressemblent a des armes
weapon_kws = ['pistol','rifle','smg','shotgun','sniper','melee','sword','gun','weapon','knife','bat','axe']
weapon_meshes = [str(p) for p in paths if any(kw in str(p).lower() for kw in weapon_kws)]
all_sm_short = [str(p) for p in paths]
print(json.dumps({
    'weapon_meshes': sorted(weapon_meshes),
    'total_sm': len(paths),
    'sample_all': sorted(all_sm_short)[:30]
}))
"""

r = ue(code)
data = get(r)

print("=== StaticMeshes armes ===")
for m in data.get('weapon_meshes', []):
    print(m)

print(f"\nTotal SM dans projet: {data.get('total_sm', 0)}")
print("\n=== Echantillon tous SM ===")
for s in data.get('sample_all', [])[:20]:
    print(s)
