import socket, json, re

def ue(code, timeout=30):
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

# Chercher tous les StaticMesh dans le projet
all_assets = unreal.EditorAssetLibrary.list_assets('/Game', recursive=True, include_folder=False)
meshes = [a for a in all_assets if 'StaticMesh' in a or 'SM_' in a or '/Weapons/' in a or '/Mesh' in a]
# Filtrer par type reel
weapon_meshes = []
for path in all_assets:
    asset_name = path.split('/')[-1].split('.')[0]
    if any(kw in path.lower() for kw in ['weapon', 'gun', 'pistol', 'rifle', 'shotgun', 'sniper', 'smg', 'sword', 'knife', 'melee', 'sm_']):
        weapon_meshes.append(path)

print(json.dumps({'weapon_meshes': sorted(weapon_meshes)[:40]}))
"""

r = ue(code)
data = get(r)
print("=== Meshes armes trouvés ===")
for m in data.get('weapon_meshes', []):
    print(m)
