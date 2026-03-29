import socket, json, re, time

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

print("Attente 5s puis verification...")
time.sleep(5)

# Verifier si WeaponMeshSM existe sur le CDO d'un weapon BP
r = ue("""
import unreal, json

try:
    # Spawner un actor BP_Pistol et verifier ses composants
    bp = unreal.load_asset('/Game/Mercenaires/Weapons/BP_Pistol')
    gen_class = bp.generated_class()
    loc = unreal.Vector(0, 0, 50000)
    rot = unreal.Rotator(0, 0, 0)
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(gen_class, loc, rot)

    # Lister tous les components
    comps = actor.get_components_by_class(unreal.ActorComponent)
    comp_names = [c.get_name() for c in comps]
    comp_types = [type(c).__name__ for c in comps]

    # Chercher WeaponMeshSM
    has_sm = any('WeaponMeshSM' in n or 'StaticMesh' in t
                 for n, t in zip(comp_names, comp_types))
    # Chercher le StaticMeshComponent
    smc = next((c for c in comps if isinstance(c, unreal.StaticMeshComponent)), None)
    mesh_assigned = None
    if smc:
        try:
            sm = smc.get_editor_property('StaticMesh')
            mesh_assigned = str(sm) if sm else 'None'
        except:
            mesh_assigned = 'error_reading'

    unreal.EditorLevelLibrary.destroy_actor(actor)
    print(json.dumps({
        'comp_names': comp_names,
        'comp_types': comp_types,
        'has_sm_comp': has_sm,
        'mesh_assigned': mesh_assigned,
    }))
except Exception as e:
    print(json.dumps({'error': str(e)}))
""")

print(f"Components sur BP_Pistol: {r.get('comp_types', [])}")
print(f"Noms:  {r.get('comp_names', [])}")
print(f"Has StaticMeshComponent: {r.get('has_sm_comp')}")
print(f"Mesh assigné: {r.get('mesh_assigned')}")
if 'error' in r:
    print(f"ERREUR: {r['error']}")
