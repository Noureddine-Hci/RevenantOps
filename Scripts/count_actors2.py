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
    return json.loads(m.group()) if m else {'raw': inner[:1000]}

r = ue("""
import unreal, json

results = {}

# Methode 1: EditorActorSubsystem (nouveau)
try:
    actor_sub = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
    actors = actor_sub.get_all_level_actors()
    sm = [a.get_actor_label() for a in actors if isinstance(a, unreal.StaticMeshActor)]
    results['method1_total'] = len(actors)
    results['method1_sm'] = len(sm)
    results['method1_sample'] = sm[:5]
except Exception as e:
    results['method1_error'] = str(e)

# Methode 2: world actors iterator
try:
    ue_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    world = ue_sub.get_editor_world()
    actor_it = unreal.ActorIterator(world)
    all_actors = list(actor_it)
    sm2 = [a.get_actor_label() for a in all_actors if isinstance(a, unreal.StaticMeshActor)]
    results['method2_total'] = len(all_actors)
    results['method2_sm'] = len(sm2)
    results['method2_sample'] = sm2[:5]
    results['method2_types'] = list(set([type(a).__name__ for a in all_actors]))[:10]
except Exception as e:
    results['method2_error'] = str(e)

print(json.dumps(results))
""")

print("Method 1 (EditorActorSubsystem):")
print(f"  Total: {r.get('method1_total', 'ERR')} | SM: {r.get('method1_sm', 'ERR')}")
print(f"  Error: {r.get('method1_error', 'none')}")
print(f"  Sample: {r.get('method1_sample', [])}")
print()
print("Method 2 (ActorIterator):")
print(f"  Total: {r.get('method2_total', 'ERR')} | SM: {r.get('method2_sm', 'ERR')}")
print(f"  Error: {r.get('method2_error', 'none')}")
print(f"  Types: {r.get('method2_types', [])}")
print(f"  Sample: {r.get('method2_sample', [])}")
