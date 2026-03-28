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

code = """
import unreal, json
# Recuperer le game world (PIE)
editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
game_world = editor_sub.get_game_world()
if game_world:
    gs = game_world.get_game_state()
    result = {
        'game_world': game_world.get_name(),
        'game_state_class': gs.get_class().get_name() if gs else None,
        'game_state_path': gs.get_path_name() if gs else None,
    }
    # Lire quelques proprietes du GameState
    if gs:
        for prop in ['bMatchActive', 'bMatchEnded', 'Score', 'CurrentWave', 'RemainingTime']:
            try:
                val = gs.get_editor_property(prop)
                result[prop] = str(val)
            except Exception as e:
                result[prop] = f'ERROR: {e}'
    print(json.dumps(result))
else:
    print(json.dumps({'error': 'No game world (PIE not running?)'}))
"""

r = ue(code)
raw = r.get('raw_result', r.get('result', ''))
m = re.search(r'\{.*\}', raw, re.DOTALL)
if m:
    data = json.loads(m.group())
    print(json.dumps(data, indent=2))
else:
    print(raw)
