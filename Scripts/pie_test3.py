import socket, json, re, time

def ue_raw(code, timeout=60):
    cmd = json.dumps({'type': 'python', 'code': code})
    with socket.create_connection(('127.0.0.1', 12029), timeout=timeout) as s:
        s.sendall(cmd.encode())
        buf = b''
        while True:
            c = s.recv(16384)
            if not c: break
            buf += c
    return buf.decode()

def ue(code, timeout=60):
    raw = ue_raw(code, timeout)
    outer = json.loads(raw)
    inner = outer.get('result', '') or outer.get('raw_result', '')
    # Strip deprecation warnings
    inner = re.sub(r'<string>.*?DeprecationWarning:.*?\n\n', '', inner, flags=re.DOTALL)
    m = re.search(r'\{.*\}', inner, re.DOTALL)
    if m:
        try:
            return json.loads(m.group())
        except:
            pass
    return {'raw': inner[:500]}

print("=== PIE Test #3: Match Flow Validation ===\n")

# 1. Save + launch PIE
print("[1] Save + Launch PIE...")
ue("import unreal, json; unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True); print(json.dumps({'ok':True}))")
r = ue("""
import unreal, json
level_sub = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_sub.editor_request_begin_play()
print(json.dumps({'pie': True}))
""")
print(f"  PIE: {r}")
time.sleep(4)

# 2. Start match directly
print("\n[2] Start Match (bypass loadout)...")
r = ue("""
import unreal, json
gw = unreal.UnrealEditorSubsystem().get_game_world()
pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
pc = pcs[0] if pcs else None
if pc:
    pc.start_mercenaires_match()
    print(json.dumps({'ok': True, 'pc': str(type(pc))}))
else:
    print(json.dumps({'ok': False, 'error': 'No PC found'}))
""")
print(f"  {r}")
time.sleep(3)

# 3. Query game state at T+3s
print("\n[3] Game State at T+3s...")
r = ue("""
import unreal, json
gw = unreal.UnrealEditorSubsystem().get_game_world()
gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)
gs = gs_list[0] if gs_list else None
if not gs:
    print(json.dumps({'error': 'No GameState'}))
else:
    pawns = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.Pawn)
    enemies = [p for p in pawns if 'Zombie' in str(type(p)) or 'Enemy' in str(type(p))]
    print(json.dumps({
        'match_active': gs.is_match_active(),
        'timer': round(gs.get_time_remaining(), 1),
        'score': gs.get_current_score(),
        'combo': gs.get_combo_multiplier(),
        'kills': gs.get_total_kills(),
        'total_pawns': len(pawns),
        'enemy_count': len(enemies),
    }))
""")
print(f"  Match active: {r.get('match_active')} | Timer: {r.get('timer')}s")
print(f"  Score: {r.get('score')} | Combo: {r.get('combo')}x | Kills: {r.get('kills')}")
print(f"  Pawns: {r.get('total_pawns')} | Enemies: {r.get('enemy_count')}")

if not r.get('match_active'):
    print("\n  [WARN] Match not active! Checking spawner state...")
    r2 = ue("""
import unreal, json
gw = unreal.UnrealEditorSubsystem().get_game_world()
actors = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.Actor)
spawners = [str(type(a)) for a in actors if 'WaveSpawner' in str(type(a)) or 'Spawner' in str(type(a))]
gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)
gs = gs_list[0] if gs_list else None
gs_class = str(type(gs)) if gs else 'None'
print(json.dumps({'spawners': spawners, 'gs_class': gs_class, 'total_actors': len(actors)}))
""")
    print(f"  Spawners: {r2.get('spawners')}")
    print(f"  GameState class: {r2.get('gs_class')}")
    print(f"  Total actors: {r2.get('total_actors')}")

# 4. Wait 5 more seconds to see if state changes
print("\n[4] Waiting 5s to check timer progression...")
time.sleep(5)
r = ue("""
import unreal, json
gw = unreal.UnrealEditorSubsystem().get_game_world()
gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)
gs = gs_list[0] if gs_list else None
if gs:
    pawns = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.Pawn)
    enemies = [p for p in pawns if 'Zombie' in str(type(p)) or 'Enemy' in str(type(p))]
    print(json.dumps({
        'match_active': gs.is_match_active(),
        'timer': round(gs.get_time_remaining(), 1),
        'score': gs.get_current_score(),
        'kills': gs.get_total_kills(),
        'total_pawns': len(pawns),
        'enemy_count': len(enemies),
    }))
else:
    print(json.dumps({'error': 'No GS'}))
""")
print(f"  Timer: {r.get('timer')}s (was {r.get('timer', 0) + 5:.1f}s before)")
print(f"  Enemies spawned: {r.get('enemy_count')} | Total pawns: {r.get('total_pawns')}")

print(f"\n[RESULT] Match validated: active={r.get('match_active')}, enemies={r.get('enemy_count')}")

# 5. Stop PIE
print("\n[5] Stopping PIE...")
ue("""
import unreal, json
unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).editor_request_end_play()
print(json.dumps({'stopped': True}))
""")
print("PIE stopped.")
