"""
Avance le flow du jeu via Python pendant PIE:
TitleScreen -> ShowLoadout -> ConfirmLoadout(Pistol) -> Match starts
"""
import socket, json, re, time

def ue(code, timeout=15):
    cmd = json.dumps({'type': 'python', 'code': code})
    with socket.create_connection(('127.0.0.1', 12029), timeout=timeout) as s:
        s.sendall(cmd.encode())
        buf = b''
        while True:
            c = s.recv(16384)
            if not c: break
            buf += c
    return json.loads(buf.decode())

def extract_json(r):
    raw = r.get('raw_result', r.get('result', ''))
    m = re.search(r'\{.*\}', raw, re.DOTALL)
    return json.loads(m.group()) if m else {'raw': raw[:300]}

# 1. Trouver le PlayerController dans le PIE world
code_find_pc = """
import unreal, json
editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
gw = editor_sub.get_game_world()
result = {}
if gw:
    pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
    if pcs:
        pc = pcs[0]
        result['pc_path'] = pc.get_path_name()
        result['pc_class'] = pc.get_class().get_name()
        # Lister les fonctions disponibles (check UFUNCTION)
        try:
            result['bLoadoutConfirmed'] = str(pc.get_editor_property('bLoadoutConfirmed'))
        except Exception as e:
            result['bLoadoutConfirmed_err'] = str(e)
        try:
            result['SelectedWeaponClass'] = str(pc.get_editor_property('SelectedWeaponClass'))
        except Exception as e:
            result['SelectedWeaponClass_err'] = str(e)
    else:
        result['error'] = 'No PlayerController found'
else:
    result['error'] = 'No game world'
print(json.dumps(result))
"""

print("=== PlayerController PIE ===")
r = ue(code_find_pc)
data = extract_json(r)
print(json.dumps(data, indent=2))

# 2. Appeler ShowLoadoutScreen puis ConfirmLoadout avec Pistol
code_trigger = """
import unreal, json
editor_sub = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
gw = editor_sub.get_game_world()
results = {}

if gw:
    # Trouver le PlayerController
    pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
    gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)

    if pcs and gs_list:
        pc = pcs[0]
        gs = gs_list[0]

        # Essayer d'appeler ShowLoadoutScreen sur le GameState
        try:
            gs.call_method('ShowLoadoutScreen')
            results['ShowLoadoutScreen'] = 'called'
        except Exception as e:
            results['ShowLoadoutScreen_err'] = str(e)

        # Essayer de definir SelectedWeaponClass sur le PC
        pistol_class = unreal.load_class(None, '/Game/Mercenaires/Weapons/BP_Pistol.BP_Pistol_C')
        if pistol_class:
            try:
                pc.set_editor_property('SelectedWeaponClass', pistol_class)
                results['SelectedWeaponClass'] = 'set to Pistol'
            except Exception as e:
                results['SelectedWeaponClass_err'] = str(e)

        # Appeler ConfirmLoadout
        try:
            pc.call_method('ConfirmLoadout')
            results['ConfirmLoadout'] = 'called'
        except Exception as e:
            results['ConfirmLoadout_err'] = str(e)

        # Appeler StartMercenairesMatch directement sur le GameState
        try:
            gs.call_method('StartMercenairesMatch')
            results['StartMercenairesMatch'] = 'called'
        except Exception as e:
            results['StartMercenairesMatch_err'] = str(e)

        # Lire etat final
        time_after = 0
        import time as t
        t.sleep(1)
        for prop in ['bMatchActive', 'Score', 'CurrentWave', 'RemainingTime']:
            try:
                results[f'final_{prop}'] = str(gs.get_editor_property(prop))
            except:
                pass
    else:
        results['error'] = f'PC={len(pcs)}, GS={len(gs_list)}'
else:
    results['error'] = 'No game world'

print(json.dumps(results))
"""

print("\n=== Trigger flow ===")
r2 = ue(code_trigger, timeout=20)
data2 = extract_json(r2)
print(json.dumps(data2, indent=2))
