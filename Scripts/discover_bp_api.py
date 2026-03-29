import socket, json, re

def ue(code, timeout=30):
    cmd = json.dumps({'type': 'python', 'code': code})
    with socket.create_connection(('127.0.0.1', 12029), timeout=timeout) as s:
        s.sendall(cmd.encode())
        buf = b''
        while True:
            c = s.recv(65536)
            if not c: break
            buf += c
    raw = buf.decode()
    outer = json.loads(raw)
    inner = outer.get('result', '') or outer.get('raw_result', '')
    m = re.search(r'\{.*\}', inner, re.DOTALL)
    return json.loads(m.group()) if m else {'raw': inner[:2000]}

r = ue("""
import unreal, json

bp = unreal.load_asset('/Game/Mercenaires/Weapons/BP_Pistol')

# Tous les attrs du Blueprint object
bp_attrs = [a for a in dir(bp) if not a.startswith('_')]

# Attrs contenant 'scs', 'component', 'construct', 'node', 'script'
relevant = [a for a in bp_attrs if any(k in a.lower() for k in
    ['scs','component','construct','node','script','add','create','variable'])]

# Methodes BlueprintEditorLibrary
bel_attrs = [a for a in dir(unreal.BlueprintEditorLibrary) if not a.startswith('_')]

print(json.dumps({
    'bp_relevant': relevant[:30],
    'bel_methods': sorted(bel_attrs)[:40],
}))
""")

print("=== BP relevant attrs ===")
for a in r.get('bp_relevant', []): print(f"  {a}")
print("\n=== BlueprintEditorLibrary methods ===")
for a in r.get('bel_methods', []): print(f"  {a}")
