import socket, json, re

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

def extract_json(r):
    raw = r.get('raw_result', r.get('result', ''))
    m = re.search(r'\{.*\}', raw, re.DOTALL)
    return json.loads(m.group()) if m else {'raw': raw}

# Chercher le GameState dans le PIE world via GameplayStatics
code = """
import unreal, json

editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
game_world = editor_sub.get_game_world()

result = {}
if not game_world:
    result = {'error': 'No PIE world found'}
else:
    result['world_name'] = game_world.get_name()
    # Utiliser GameplayStatics pour trouver le GameState
    try:
        gs = unreal.GameplayStatics.get_game_state(game_world)
        if gs:
            result['gs_path'] = gs.get_path_name()
            result['gs_class'] = gs.get_class().get_name()
            for prop in ['bMatchActive', 'bMatchEnded', 'Score', 'CurrentWave', 'RemainingTime']:
                try:
                    result[prop] = str(gs.get_editor_property(prop))
                except:
                    pass
        else:
            result['gs'] = 'None (GameState not spawned yet?)'
    except Exception as e:
        result['gs_error'] = str(e)

    # Lister les acteurs du PIE world par classe
    try:
        actors = unreal.GameplayStatics.get_all_actors_of_class(game_world, unreal.GameStateBase)
        result['gamestates_found'] = [a.get_path_name() for a in actors]
    except Exception as e:
        result['gs_search_error'] = str(e)

print(json.dumps(result))
"""

r = ue(code)
data = extract_json(r)
print(json.dumps(data, indent=2))
