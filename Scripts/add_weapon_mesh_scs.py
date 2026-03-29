import socket, json, re

def ue_raw(code, timeout=120):
    cmd = json.dumps({'type': 'python', 'code': code})
    with socket.create_connection(('127.0.0.1', 12029), timeout=timeout) as s:
        s.sendall(cmd.encode())
        buf = b''
        while True:
            c = s.recv(65536)
            if not c: break
            buf += c
    return buf.decode()

def ue(code, timeout=120):
    raw = ue_raw(code, timeout)
    outer = json.loads(raw)
    inner = outer.get('result', '') or outer.get('raw_result', '')
    inner = re.sub(r'<string>.*?DeprecationWarning.*?\n\n', '', inner, flags=re.DOTALL)
    m = re.search(r'\{.*\}', inner, re.DOTALL)
    if m:
        try: return json.loads(m.group())
        except: pass
    return {'raw': inner[:1000]}

print("=== Ajout StaticMeshComponent placeholder dans chaque BP Arme ===\n")

code = """
import unreal, json

# Mesh placeholder: SM_ChamferCube
chamfer = unreal.load_asset('/Game/LevelPrototyping/Meshes/SM_ChamferCube')
cylinder = unreal.load_asset('/Game/LevelPrototyping/Meshes/SM_Cylinder')

# (path, label, mesh, scale_x, scale_y, scale_z, rel_loc_x, rel_loc_y, rel_loc_z)
# Orienté pour etre dans la main droite - taille en unreal units
weapons = [
    ('/Game/Mercenaires/Weapons/BP_Pistol',       'Pistol',        chamfer,  0.15, 0.06, 0.10,  5, 0, -3),
    ('/Game/Mercenaires/Weapons/BP_AssaultRifle', 'AssaultRifle',  chamfer,  0.30, 0.06, 0.08, 15, 0, -3),
    ('/Game/Mercenaires/Weapons/BP_SMG',          'SMG',           chamfer,  0.20, 0.06, 0.08, 10, 0, -3),
    ('/Game/Mercenaires/Weapons/BP_Shotgun',      'Shotgun',       chamfer,  0.28, 0.06, 0.10, 12, 0, -3),
    ('/Game/Mercenaires/Weapons/BP_Sniper',       'Sniper',        chamfer,  0.40, 0.05, 0.07, 20, 0, -3),
    ('/Game/Mercenaires/Weapons/BP_Melee',        'Melee',         cylinder, 0.05, 0.05, 0.40,  0, 0,-10),
]

results = []
for (path, label, mesh, sx, sy, sz, lx, ly, lz) in weapons:
    try:
        bp = unreal.load_asset(path)
        scs = bp.simple_construction_script

        # Verifier si un SMC placeholder existe deja
        existing = [n for n in scs.get_all_nodes()
                    if 'PlaceholderMesh' in (n.get_variable_name() or '')]
        if existing:
            results.append({'weapon': label, 'status': 'already_exists'})
            continue

        # Creer un nouveau noeud SCS avec StaticMeshComponent
        new_node = scs.create_node_with_created_component(unreal.StaticMeshComponent)
        new_node.set_variable_name('PlaceholderMesh')

        # Configurer le component template
        comp = new_node.component_template
        comp.set_editor_property('StaticMesh', mesh)
        comp.set_editor_property('RelativeScale3D', unreal.Vector(sx, sy, sz))
        comp.set_editor_property('RelativeLocation', unreal.Vector(lx, ly, lz))

        # Attacher au root
        root_node = scs.get_root_nodes()
        if root_node:
            root_node[0].add_child_node(new_node)
        else:
            scs.add_node(new_node)

        # Compiler le BP
        unreal.BlueprintEditorLibrary.compile_blueprint(bp)
        unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)

        results.append({'weapon': label, 'status': 'ok'})
    except Exception as e:
        results.append({'weapon': label, 'status': 'error', 'error': str(e)[:150]})

print(json.dumps({'results': results}))
"""

r = ue(code)
print("Resultats:")
for res in r.get('results', []):
    status = res.get('status', '?')
    icon = 'OK' if status == 'ok' else ('DEJA' if status == 'already_exists' else 'FAIL')
    print(f"  [{icon}] {res.get('weapon', '?'):15} {res.get('error', '')}")

if 'raw' in r:
    print("RAW:", r['raw'][:500])
