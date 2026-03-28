import socket, json, re, time

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

def get_raw(r):
    return r.get('raw_result', r.get('result', ''))

# 1. Sauvegarder le niveau
save_code = """
import unreal, json
try:
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    print(json.dumps({'success': True, 'message': 'Level saved'}))
except Exception as e:
    print(json.dumps({'success': False, 'error': str(e)}))
"""
print("=== Sauvegarde du niveau ===")
r = ue(save_code)
print(get_raw(r))

# 2. Lancer PIE
pie_code = """
import unreal, json
try:
    level_sub = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_sub.editor_request_begin_play()
    print(json.dumps({'success': True, 'message': 'PIE launched'}))
except Exception as e:
    print(json.dumps({'success': False, 'error': str(e)}))
"""
print("\n=== Lancement PIE ===")
r2 = ue(pie_code)
print(get_raw(r2))
print("\nPIE en cours — monitoring via logs...")
