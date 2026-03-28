import socket, json, re, time

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

# Step 1: ShowLoadout + StartMatch
step1 = """
import unreal, json
editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
gw = editor_sub.get_game_world()
pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)
pc = pcs[0]; gs = gs_list[0]
results = {}

# Chercher la propriete SelectedWeaponClass sur le PC
weapon_props = [p for p in dir(pc) if 'weapon' in p.lower() or 'class' in p.lower()]
results['weapon_props'] = weapon_props[:10]

# Appel direct du flow
try:
    pc.show_loadout_screen()
    results['show_loadout'] = 'ok'
except Exception as e:
    results['show_loadout_err'] = str(e)

try:
    pc.start_mercenaires_match()
    results['start_match'] = 'ok'
except Exception as e:
    results['start_match_err'] = str(e)

print(json.dumps(results))
"""

print("=== Step 1: ShowLoadout + StartMatch ===")
r1 = ue(step1)
d1 = get(r1)
print(json.dumps(d1, indent=2))

time.sleep(2)

# Step 2: Verifier etat du match apres
step2 = """
import unreal, json
editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
gw = editor_sub.get_game_world()
gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)
gs = gs_list[0] if gs_list else None
results = {}
if gs:
    results['is_match_active'] = gs.is_match_active()
    results['current_score'] = gs.get_current_score()
    results['total_kills'] = gs.get_total_kills()
    results['has_match_ended'] = gs.has_match_ended()
    try:
        results['match_duration_prop'] = str(gs.get_editor_property('match_duration'))
    except:
        pass
print(json.dumps(results))
"""

print("\n=== Step 2: Etat du match ===")
r2 = ue(step2)
d2 = get(r2)
print(json.dumps(d2, indent=2))
