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

# Lister TOUS les assets sous /Game/Characters
char_assets = unreal.EditorAssetLibrary.list_assets('/Game/Characters', recursive=True)
# Lister /Game/Mercenaires
merc_assets = unreal.EditorAssetLibrary.list_assets('/Game/Mercenaires', recursive=True)

print(json.dumps({
    'characters': sorted([str(p) for p in char_assets]),
    'mercenaires': sorted([str(p) for p in merc_assets])
}))
"""

r = ue(code)
data = get(r)

print("=== /Game/Characters ===")
for p in data.get('characters', []): print(p)
print("\n=== /Game/Mercenaires ===")
for p in data.get('mercenaires', []): print(p)
