import socket, json

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

# Lister tous les assets Mercenaires
code = """
import unreal, json
assets = unreal.EditorAssetLibrary.list_assets('/Game/Mercenaires', recursive=True, include_folder=False)
print(json.dumps({'assets': sorted(assets)}))
"""
r = ue(code)
raw = r.get('raw_result', r.get('result', ''))
# Extraire JSON du raw_result
import re
m = re.search(r'\{.*\}', raw, re.DOTALL)
if m:
    data = json.loads(m.group())
    print("=== Assets /Game/Mercenaires ===")
    for a in data.get('assets', []):
        print(a)

# Verifier le GameMode du level actuel
code2 = """
import unreal, json
world = unreal.EditorLevelLibrary.get_editor_world()
ws = world.get_world_settings()
gm = None
for prop in ['default_game_mode', 'game_mode_override', 'GameModeOverride']:
    try:
        gm = ws.get_editor_property(prop)
        if gm:
            break
    except: pass
print(json.dumps({'current_gamemode': str(gm) if gm else None}))
"""
r2 = ue(code2)
raw2 = r2.get('raw_result', r2.get('result', ''))
m2 = re.search(r'\{.*\}', raw2, re.DOTALL)
if m2:
    print("\n=== GameMode actuel du level ===")
    print(json.dumps(json.loads(m2.group()), indent=2))
