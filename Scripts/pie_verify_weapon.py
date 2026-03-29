import socket, json, re, time

def ue(code, timeout=60):
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
    inner = re.sub(r'<string>.*?DeprecationWarning.*?\n\n', '', inner, flags=re.DOTALL)
    m = re.search(r'\{.*\}', inner, re.DOTALL)
    return json.loads(m.group()) if m else {'raw': inner[:500]}

print("=== PIE Test - Verification Weapon Mesh ===\n")

# 1. Lancer PIE
print("[1] Lancement PIE...")
ue("""
import unreal, json
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_begin_play()
print(json.dumps({'ok': True}))
""")
time.sleep(4)

# 2. Demarrer le match
print("[2] Demarrage match...")
ue("""
import unreal, json
gw = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
if pcs: pcs[0].start_mercenaires_match()
print(json.dumps({'ok': True}))
""")
time.sleep(3)

# 3. Verifier la weapon et ses composants
print("[3] Verification weapon mesh...")
r = ue("""
import unreal, json
gw = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
pc = pcs[0] if pcs else None
char = pc.get_controlled_pawn() if pc else None

result = {}
if char:
    result['char'] = type(char).__name__
    # Trouver l'arme equipe
    pawns = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.Actor)
    weapon_actors = [a for a in pawns if 'Weapon' in type(a).__name__]
    result['weapon_count'] = len(weapon_actors)
    if weapon_actors:
        w = weapon_actors[0]
        result['weapon_type'] = type(w).__name__
        # Verifier composants de l'arme
        comps = w.get_components_by_class(unreal.StaticMeshComponent)
        sm_comps = []
        for c in comps:
            try:
                sm = c.get_editor_property('StaticMesh')
                sm_comps.append({'name': c.get_name(), 'mesh': str(sm)[:50] if sm else 'None'})
            except:
                sm_comps.append({'name': c.get_name(), 'mesh': 'error'})
        result['static_mesh_comps'] = sm_comps
        result['is_visible'] = w.is_actor_being_rendered()

# Compter ennemis
pawns_all = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.Pawn)
result['total_pawns'] = len(pawns_all)
result['enemies'] = len([p for p in pawns_all if 'Zombie' in type(p).__name__])

# GameState
gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)
gs = gs_list[0] if gs_list else None
result['match_active'] = gs.is_match_active() if gs else False
result['timer'] = round(gs.get_time_remaining(), 1) if gs else 0

print(json.dumps(result))
""")

print(f"\n=== RESULTATS ===")
print(f"Personnage      : {r.get('char', 'N/A')}")
print(f"Match actif     : {r.get('match_active')} | Timer: {r.get('timer')}s")
print(f"Ennemis         : {r.get('enemies')} / {r.get('total_pawns')} pawns")
print(f"Armes trouvees  : {r.get('weapon_count', 0)}")
print(f"Weapon type     : {r.get('weapon_type', 'N/A')}")
print(f"Weapon visible  : {r.get('is_visible')}")
print(f"StaticMeshComps : {r.get('static_mesh_comps', [])}")

if r.get('static_mesh_comps') and any(c.get('mesh') != 'None' for c in r.get('static_mesh_comps', [])):
    print("\n[OK] Weapon mesh VISIBLE dans le jeu!")
else:
    print("\n[WARN] Mesh non confirme - verifier en PIE manuellement")

# Stop PIE
print("\n[4] Arret PIE...")
time.sleep(3)
ue("""
import unreal, json
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_end_play()
print(json.dumps({'ok': True}))
""")
print("PIE arrete.")
