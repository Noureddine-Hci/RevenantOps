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

# Acteurs cibles
SPAWNER_LABEL = 'MercenairesWaveSpawner'
SPAWN_POINT_LABELS = [
    'SP1_SW_Hangar',
    'SP2_NW_Hangar',
    'SP3_S_Hangar',
    'SP4_SE_ServerRoom',
    'SP5_NE_ServerRoom',
    'SP6_E_ServerRoom',
    'SP7_NW_LoadingDock',
    'SP8_N_LoadingDock',
]

actors = unreal.EditorLevelLibrary.get_all_level_actors()
actor_map = {a.get_actor_label(): a for a in actors}

spawner = actor_map.get(SPAWNER_LABEL)
if not spawner:
    print(json.dumps({'success': False, 'message': f'Spawner {SPAWNER_LABEL} not found'}))
else:
    points = []
    missing = []
    for label in SPAWN_POINT_LABELS:
        a = actor_map.get(label)
        if a:
            points.append(a)
        else:
            missing.append(label)

    if missing:
        print(json.dumps({'success': False, 'message': f'Missing points: {missing}'}))
    else:
        spawner.set_editor_property('SpawnPoints', points)
        # Verifier
        assigned = spawner.get_editor_property('SpawnPoints')
        print(json.dumps({
            'success': True,
            'spawner': SPAWNER_LABEL,
            'assigned_count': len(assigned),
            'points': [p.get_actor_label() for p in assigned]
        }))
"""

result = ue(code)
print(json.dumps(result, indent=2, ensure_ascii=False))
