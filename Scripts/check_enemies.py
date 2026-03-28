import socket, json, re, time

def ue(code, timeout=15):
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

# Attendre 5s que les ennemis spawne
time.sleep(5)

code = """
import unreal, json
editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
gw = editor_sub.get_game_world()
results = {}

# GameState
gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)
gs = gs_list[0] if gs_list else None
if gs:
    results['match_active'] = gs.is_match_active()
    results['score'] = gs.get_current_score()
    results['kills'] = gs.get_total_kills()
    results['match_ended'] = gs.has_match_ended()

# Chercher des ennemis (EnemyBase)
try:
    enemies = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.Pawn)
    results['all_pawns'] = len(enemies)
    results['pawn_classes'] = list(set(e.get_class().get_name() for e in enemies))
except Exception as e:
    results['pawn_err'] = str(e)

# Chercher le WaveSpawner
try:
    all_actors = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.Actor)
    spawners = [a for a in all_actors if 'WaveSpawner' in a.get_class().get_name() or 'EnemyWave' in a.get_class().get_name()]
    results['spawners'] = [{'label': s.get_actor_label(), 'class': s.get_class().get_name()} for s in spawners]
except Exception as e:
    results['spawner_err'] = str(e)

print(json.dumps(results))
"""

print("=== Etat match + ennemis (apres 5s) ===")
r = ue(code)
print(json.dumps(get(r), indent=2))
