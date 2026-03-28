import socket, json, re

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

def get_json(r):
    raw = r.get('raw_result', r.get('result', ''))
    m = re.search(r'\{.*\}', raw, re.DOTALL)
    return json.loads(m.group()) if m else {}

# Verifier la GameStateClass du BP_ThirdPersonGameMode
check_code = """
import unreal, json
bp = unreal.EditorAssetLibrary.load_asset('/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode.BP_ThirdPersonGameMode')
result = {}
if bp:
    # Lire les CDO properties du GameMode
    cdo = unreal.get_default_object(bp.generated_class())
    try:
        gs_class = cdo.get_editor_property('GameStateClass')
        result['GameStateClass'] = str(gs_class) if gs_class else None
    except Exception as e:
        result['GameStateClass_error'] = str(e)
    try:
        pc_class = cdo.get_editor_property('PlayerControllerClass')
        result['PlayerControllerClass'] = str(pc_class) if pc_class else None
    except Exception as e:
        result['PC_error'] = str(e)
else:
    result['error'] = 'BP not found'
print(json.dumps(result))
"""

print("=== GameMode CDO properties ===")
r = ue(check_code)
data = get_json(r)
print(json.dumps(data, indent=2))

# Si GameStateClass n'est pas MercenairesGameState, le setter
set_gs_code = """
import unreal, json
bp = unreal.EditorAssetLibrary.load_asset('/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode.BP_ThirdPersonGameMode')
gs_bp = unreal.EditorAssetLibrary.load_asset('/Game/Mercenaires/Gameplay/BP_MercenairesGameState.BP_MercenairesGameState')

if not bp:
    print(json.dumps({'success': False, 'message': 'GameMode BP not found'}))
elif not gs_bp:
    print(json.dumps({'success': False, 'message': 'GameState BP not found'}))
else:
    gs_class = gs_bp.generated_class()
    cdo = unreal.get_default_object(bp.generated_class())
    try:
        cdo.set_editor_property('GameStateClass', gs_class)
        unreal.EditorAssetLibrary.save_asset(bp.get_path_name())
        # Verify
        check = cdo.get_editor_property('GameStateClass')
        print(json.dumps({'success': True, 'GameStateClass': str(check)}))
    except Exception as e:
        print(json.dumps({'success': False, 'error': str(e)}))
"""

print("\n=== Configuration GameStateClass ===")
r2 = ue(set_gs_code)
data2 = get_json(r2)
print(json.dumps(data2, indent=2))
