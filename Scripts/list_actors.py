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

code = """
import unreal, json
actors = unreal.EditorLevelLibrary.get_all_level_actors()
info = [{'label': a.get_actor_label(), 'class': a.get_class().get_name()} for a in actors]
info.sort(key=lambda x: x['class'])
print(json.dumps(info))
"""

result = ue(code)
print(json.dumps(result, indent=2, ensure_ascii=False))
