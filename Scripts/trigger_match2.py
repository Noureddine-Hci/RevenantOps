import socket, json, re

def ue(code, timeout=20):
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
    return json.loads(m.group()) if m else {'raw': raw[:600]}

code = """
import unreal, json

editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
gw = editor_sub.get_game_world()
results = {}

pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)
pc = pcs[0] if pcs else None
gs = gs_list[0] if gs_list else None

# Lister les methodes disponibles sur le PC (chercher confirm/loadout/match)
if pc:
    methods = [m for m in dir(pc) if 'loadout' in m.lower() or 'match' in m.lower() or 'title' in m.lower() or 'game' in m.lower()]
    results['pc_methods'] = methods

if gs:
    methods_gs = [m for m in dir(gs) if 'match' in m.lower() or 'start' in m.lower() or 'kill' in m.lower() or 'score' in m.lower()]
    results['gs_methods'] = methods_gs

print(json.dumps(results))
"""

r = ue(code)
print(json.dumps(get(r), indent=2))
