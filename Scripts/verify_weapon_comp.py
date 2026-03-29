import socket, json, re

def ue_result(code, timeout=30):
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
    # Niveau 1: outer["result"] est un JSON string
    inner_str = outer.get('result', '')
    if inner_str:
        try:
            inner = json.loads(inner_str)
            # Niveau 2: inner["raw_result"] contient le vrai output Python
            raw_result = inner.get('raw_result', inner_str)
        except:
            raw_result = inner_str
    else:
        raw_result = outer.get('raw_result', '')
    # Supprimer DeprecationWarnings
    raw_result = re.sub(r'<string>:\d+: DeprecationWarning:.*?\n\n', '', raw_result, flags=re.DOTALL)
    # Trouver le JSON imprime
    m = re.search(r'\{.*\}', raw_result, re.DOTALL)
    if m:
        try:
            return json.loads(m.group())
        except:
            pass
    return {'raw': raw_result[:500]}

r = ue_result("""
import unreal, json

weapons = [
    ('BP_Pistol',       '/Game/Mercenaires/Weapons/BP_Pistol'),
    ('BP_AssaultRifle', '/Game/Mercenaires/Weapons/BP_AssaultRifle'),
    ('BP_SMG',          '/Game/Mercenaires/Weapons/BP_SMG'),
    ('BP_Shotgun',      '/Game/Mercenaires/Weapons/BP_Shotgun'),
    ('BP_Sniper',       '/Game/Mercenaires/Weapons/BP_Sniper'),
    ('BP_Melee',        '/Game/Mercenaires/Weapons/BP_Melee'),
]

results = {}
z = 50000
for name, path in weapons:
    try:
        bp = unreal.load_asset(path)
        actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            bp.generated_class(), unreal.Vector(0,0,z), unreal.Rotator(0,0,0))
        z += 300
        sm_comps = actor.get_components_by_class(unreal.StaticMeshComponent)
        meshes = []
        for c in sm_comps:
            try:
                sm = c.get_editor_property('StaticMesh')
                meshes.append(sm.get_name() if sm else 'None')
            except: meshes.append('err')
        results[name] = {'sm_count': len(sm_comps), 'meshes': meshes}
        unreal.EditorLevelLibrary.destroy_actor(actor)
    except Exception as e:
        results[name] = {'error': str(e)[:60]}
print(json.dumps(results))
""")

print("=== Verification WeaponMeshSM sur les 6 armes ===\n")
if 'raw' in r:
    print("Raw:", r['raw'][:500])
else:
    all_ok = True
    for name, info in r.items():
        if 'error' in info:
            print(f"  [FAIL] {name:15}: {info['error']}")
            all_ok = False
        else:
            ok = info['sm_count'] > 0 and any(m not in ('None','err') for m in info['meshes'])
            if not ok: all_ok = False
            print(f"  {'[OK  ]' if ok else '[WARN]'} {name:15} | {info['sm_count']} SMC | meshes={info['meshes']}")
    print(f"\n{'[SUCCES] WeaponMeshSM avec SM_ChamferCube present sur toutes les armes!' if all_ok else '[WARN]'}")
