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
    return json.loads(m.group()) if m else {'raw': inner[:500]}

r = ue("""
import unreal, json
actors = unreal.EditorLevelLibrary.get_all_level_actors()
sm_actors = [a.get_actor_label() for a in actors if isinstance(a, unreal.StaticMeshActor)]
other = [str(type(a).__name__) + ':' + a.get_actor_label() for a in actors if not isinstance(a, unreal.StaticMeshActor)]
print(json.dumps({
    'total': len(actors),
    'static_meshes': len(sm_actors),
    'sm_sample': sm_actors[:10],
    'other': other[:20],
}))
""")

print(f"Total actors dans le level : {r.get('total', 0)}")
print(f"StaticMeshActors (geometry) : {r.get('static_meshes', 0)}")
print(f"Exemples : {r.get('sm_sample', [])[:5]}")
print(f"Autres actors : {r.get('other', [])}")
