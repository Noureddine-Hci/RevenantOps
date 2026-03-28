import socket, json

def ue(code, timeout=10):
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
try:
    level_sub = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_sub.editor_request_end_play()
    print(json.dumps({'success': True, 'message': 'PIE stopped'}))
except Exception as e:
    print(json.dumps({'success': False, 'error': str(e)}))
"""
r = ue(code)
print(r)
