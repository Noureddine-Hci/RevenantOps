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

code = """
import unreal, json

# Search Engine content for weapon/prop meshes
reg = unreal.AssetRegistryHelpers.get_asset_registry()

# List engine content
try:
    engine_assets = unreal.EditorAssetLibrary.list_assets('/Engine', recursive=True)
    weapon_kws = ['pistol','rifle','gun','weapon','smg','shotgun','sniper','melee','sword','knife','prop','cube','cylinder','sphere']
    weapon_engine = [str(p) for p in engine_assets if any(kw in str(p).lower() for kw in weapon_kws)]
    sample_engine = [str(p) for p in engine_assets if 'SM_' in str(p) or 'Mesh' in str(p)][:20]
except Exception as e:
    weapon_engine = []
    sample_engine = [str(e)]

# Also check /Game for any SKM weapon mesh
game_assets = unreal.EditorAssetLibrary.list_assets('/Game', recursive=True)
skm_assets = [str(p) for p in game_assets if 'SKM_' in str(p) or ('SK_' in str(p) and 'Weapon' in str(p))]
sm_game = [str(p) for p in game_assets if str(p).split('/')[-1].split('.')[0].startswith('SM_')]

print(json.dumps({
    'weapon_engine': sorted(weapon_engine)[:30],
    'sample_engine_mesh': sorted(sample_engine)[:20],
    'skm_game': skm_assets[:20],
    'sm_game': sorted(sm_game)[:30],
    'total_engine': len(engine_assets) if engine_assets else 0
}))
"""

r = ue(code)
data = get(r)
print(f"Total engine assets: {data.get('total_engine', 0)}")
print("\n=== weapon_engine ===")
for p in data.get('weapon_engine', []): print(p)
print("\n=== engine SM_ meshes ===")
for p in data.get('sample_engine_mesh', []): print(p)
print("\n=== SKM game ===")
for p in data.get('skm_game', []): print(p)
print("\n=== SM_ game ===")
for p in data.get('sm_game', []): print(p)
