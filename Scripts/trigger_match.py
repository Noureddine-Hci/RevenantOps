import socket, json, re

def ue(code, timeout=20):
    cmd = json.dumps({'type': 'python', 'code': code})
    with socket.create_connection(('127.0.0.1', 12029), timeout=timeout) as s:
        s.sendall(cmd.encode())
        buf = b''
        while True:
            c = s.recv(16384)
            if not c: break
            buf += c
    return json.loads(buf.decode())

def get(r):
    raw = r.get('raw_result', r.get('result', ''))
    m = re.search(r'\{.*\}', raw, re.DOTALL)
    return json.loads(m.group()) if m else {'raw': raw[:500]}

code = """
import unreal, json, time

editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
gw = editor_sub.get_game_world()
results = {}

pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)

pc = pcs[0] if pcs else None
gs = gs_list[0] if gs_list else None

results['pc'] = pc.get_class().get_name() if pc else None
results['gs'] = gs.get_class().get_name() if gs else None

# Selectionner le Pistol comme arme
pistol_cls = unreal.load_class(None, '/Game/Mercenaires/Weapons/BP_Pistol.BP_Pistol_C')
results['pistol_cls_found'] = pistol_cls is not None

# Appel via unreal.RevenantOpsPlayerController methods
if pc:
    # Essai 1: via le nom C++ de la methode
    pc_cast = unreal.cast(pc, unreal.RevenantOpsPlayerController)
    if pc_cast:
        results['cast_ok'] = True
        # Assigner l'arme
        if pistol_cls:
            try:
                pc_cast.set_editor_property('SelectedWeaponClass', pistol_cls)
                results['weapon_set'] = True
            except Exception as e:
                results['weapon_set_err'] = str(e)
        # ShowLoadout
        try:
            pc_cast.show_loadout_screen()
            results['show_loadout'] = 'ok'
        except Exception as e:
            results['show_loadout_err'] = str(e)
        # StartMatch
        try:
            pc_cast.start_mercenaires_match()
            results['start_match'] = 'ok'
        except Exception as e:
            results['start_match_err'] = str(e)
    else:
        results['cast_ok'] = False

# Check GameState apres
if gs:
    gs_cast = unreal.cast(gs, unreal.MercenairesGameState)
    if gs_cast:
        try:
            gs_cast.start_match()
            results['gs_start_match'] = 'ok'
        except Exception as e:
            results['gs_start_match_err'] = str(e)

    time.sleep(0.5)
    try:
        results['bMatchActive'] = str(gs.get_editor_property('bMatchActive'))
    except:
        pass

print(json.dumps(results))
"""

r = ue(code)
print(json.dumps(get(r), indent=2))
