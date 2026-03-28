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

# Chercher dans /Game + /Engine les meshes qui ressemblent à des armes
code = """
import unreal, json

# Lister les SkeletalMesh aussi
all_assets = unreal.EditorAssetLibrary.list_assets('/Game', recursive=True)

sk_meshes = []
for p in all_assets:
    name = p.split('/')[-1].split('.')[0].lower()
    if any(kw in name for kw in ['pistol','rifle','gun','weapon','smg','shotgun','sniper','melee','sword']):
        sk_meshes.append(p)

# Aussi chercher les assets qui contiennent "prop" ou sont des meshes de personnages
prop_meshes = []
for p in all_assets:
    if '/Props/' in p or '/Weapons/' in p or '/Items/' in p:
        prop_meshes.append(p)

# SkeletalMesh assets
sk_filter = [p for p in all_assets if 'SKM_' in p or 'SK_' in p]

print(json.dumps({
    'weapon_named': sk_meshes[:20],
    'props': prop_meshes[:20],
    'skeletal': sk_filter[:20],
    'total_assets': len(all_assets)
}))
"""

r = ue(code)
data = get(r)
print(f"Total assets: {data.get('total_assets')}")
print("\n=== weapon_named ===")
for p in data.get('weapon_named', []): print(p)
print("\n=== props ===")
for p in data.get('props', []): print(p)
print("\n=== skeletal ===")
for p in data.get('skeletal', []): print(p)
