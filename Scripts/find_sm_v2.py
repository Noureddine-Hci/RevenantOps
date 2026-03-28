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
    return json.loads(m.group()) if m else {'raw': raw[:1000]}

# UE5.5+ approach: class_paths
code = """
import unreal, json

reg = unreal.AssetRegistryHelpers.get_asset_registry()

# UE5 uses TopLevelAssetPath
try:
    filter_obj = unreal.ARFilter(
        class_paths=['/Script/Engine.StaticMesh'],
        recursive_paths=True,
        package_paths=['/Game']
    )
    assets = reg.get_assets(filter_obj)
    paths = []
    for a in assets:
        try:
            p = str(a.get_asset().get_path_name()) if a.is_asset_loaded() else str(a.package_name) + '.' + str(a.asset_name)
        except:
            p = str(a.package_name)
        paths.append(p)
    weapon_kws = ['pistol','rifle','smg','shotgun','sniper','melee','sword','gun','weapon']
    weapon = [p for p in paths if any(kw in p.lower() for kw in weapon_kws)]
    print(json.dumps({'total': len(paths), 'weapon': sorted(weapon), 'sample': sorted(paths)[:20]}))
except Exception as e:
    # Fallback: list_assets + check type
    all_paths = unreal.EditorAssetLibrary.list_assets('/Game', recursive=True)
    sm_paths = []
    for p in all_paths:
        name = p.split('/')[-1].split('.')[0]
        if name.startswith('SM_') or '_SM_' in name:
            sm_paths.append(p)
    print(json.dumps({'total': len(sm_paths), 'weapon': sm_paths[:30], 'error': str(e)}))
"""

r = ue(code)
data = get(r)
print(f"Total StaticMesh: {data.get('total', 0)}")
print("\n=== Armes ===")
for p in data.get('weapon', []):
    print(p)
print("\n=== Sample ===")
for p in data.get('sample', [])[:15]:
    print(p)
if 'error' in data:
    print(f"\nERROR: {data['error']}")
