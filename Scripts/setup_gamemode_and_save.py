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

# 1. Chercher le bon chemin du BP_MercenairesGameState
find_bp = """
import unreal, json
# Chercher l'asset BP_MercenairesGameState
results = unreal.EditorAssetLibrary.list_assets('/Game', recursive=True, include_folder=False)
gm_assets = [r for r in results if 'MercenairesGameState' in r or 'GameMode' in r or 'GameState' in r]
print(json.dumps({'assets': gm_assets}))
"""

print("=== Recherche BP GameMode/GameState ===")
r = ue(find_bp)
raw = r.get('result', r.get('raw_result', ''))
try:
    data = json.loads(raw) if raw.startswith('{') else json.loads(raw[raw.index('{'):])
    for a in data.get('assets', []):
        print(a)
except:
    print(raw)
