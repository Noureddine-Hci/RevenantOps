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
    m = re.search(r'\{.*\}', inner, re.DOTALL)
    if m:
        return json.loads(m.group())
    return {'raw': inner[:500]}

print("=== PIE Test #2: Full Game Flow ===\n")

# STEP 1: Save level
print("[1/6] Saving level...")
r = ue("""
import unreal, json
unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
print(json.dumps({'saved': True}))
""")
print(f"  Saved: {r.get('saved', False)}\n")

# STEP 2: Launch PIE
print("[2/6] Launching PIE...")
r = ue("""
import unreal, json
try:
    level_sub = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    level_sub.editor_request_begin_play()
    print(json.dumps({'pie_started': True}))
except Exception as e:
    print(json.dumps({'pie_started': False, 'error': str(e)}))
""")
print(f"  PIE started: {r.get('pie_started')}")
if not r.get('pie_started'):
    print(f"  ERROR: {r.get('error')}")

print("  Waiting 3s for PIE init...")
time.sleep(3)

# STEP 3: Verify world + show loadout
print("\n[3/6] Show Loadout Screen...")
r = ue("""
import unreal, json
try:
    gw = unreal.UnrealEditorSubsystem().get_game_world()
    pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
    pc = pcs[0] if pcs else None
    if pc:
        pc.show_loadout_screen()
        print(json.dumps({'ok': True, 'pc_class': str(type(pc))}))
    else:
        print(json.dumps({'ok': False, 'error': 'No PlayerController'}))
except Exception as e:
    print(json.dumps({'ok': False, 'error': str(e)}))
""")
print(f"  Loadout screen: {r.get('ok')} | PC: {r.get('pc_class', r.get('error', ''))}")

time.sleep(1)

# STEP 4: Confirm loadout (select pistol) + start match
print("\n[4/6] Confirm Loadout + Start Match...")
r = ue("""
import unreal, json
try:
    gw = unreal.UnrealEditorSubsystem().get_game_world()
    pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
    pc = pcs[0] if pcs else None
    if pc:
        # Confirm loadout with Pistol (index 0)
        pc.confirm_loadout(0)
        print(json.dumps({'loadout_confirmed': True}))
    else:
        print(json.dumps({'loadout_confirmed': False, 'error': 'No PC'}))
except Exception as e:
    print(json.dumps({'loadout_confirmed': False, 'error': str(e)}))
""")
print(f"  Loadout confirmed: {r.get('loadout_confirmed')} {r.get('error','')}")

time.sleep(1)

# STEP 5: Check game state
print("\n[5/6] Check Game State (3s after match start)...")
time.sleep(2)
r = ue("""
import unreal, json
try:
    gw = unreal.UnrealEditorSubsystem().get_game_world()
    gs_list = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.GameStateBase)
    gs = gs_list[0] if gs_list else None
    if gs:
        pawns = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.Pawn)
        match_active = gs.is_match_active() if hasattr(gs, 'is_match_active') else 'N/A'
        timer = gs.get_remaining_time() if hasattr(gs, 'get_remaining_time') else 'N/A'
        score = gs.get_total_score() if hasattr(gs, 'get_total_score') else 'N/A'
        print(json.dumps({
            'gs_type': str(type(gs)),
            'match_active': str(match_active),
            'timer': str(timer),
            'score': str(score),
            'pawn_count': len(pawns),
        }))
    else:
        print(json.dumps({'error': 'No GameState'}))
except Exception as e:
    print(json.dumps({'error': str(e)}))
""")
print(f"  GameState: {r.get('gs_type', r.get('error'))}")
print(f"  Match Active: {r.get('match_active')} | Timer: {r.get('timer')} | Score: {r.get('score')}")
print(f"  Pawns in world: {r.get('pawn_count', 0)}")

# STEP 6: Check character has weapon
print("\n[6/6] Check Character Weapon...")
r = ue("""
import unreal, json
try:
    gw = unreal.UnrealEditorSubsystem().get_game_world()
    pcs = unreal.GameplayStatics.get_all_actors_of_class(gw, unreal.PlayerController)
    pc = pcs[0] if pcs else None
    char = pc.get_controlled_pawn() if pc else None
    char_info = {}
    if char:
        char_info['type'] = str(type(char))
        # Try to get current weapon
        try:
            weapon = char.get_editor_property('CurrentWeapon')
            char_info['weapon'] = str(type(weapon)) if weapon else 'None'
        except:
            # Try current_weapon
            try:
                weapon = char.current_weapon
                char_info['weapon'] = str(type(weapon)) if weapon else 'None'
            except Exception as we:
                char_info['weapon'] = f'N/A: {we}'
    print(json.dumps({'char': char_info}))
except Exception as e:
    print(json.dumps({'error': str(e)}))
""")
char = r.get('char', {})
print(f"  Character: {char.get('type', r.get('error'))}")
print(f"  Weapon equipped: {char.get('weapon', 'unknown')}")

print("\n=== PIE Test #2 Complete ===")
print("Stopping PIE in 3s...")
time.sleep(3)

# Stop PIE
r = ue("""
import unreal, json
level_sub = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level_sub.editor_request_end_play()
print(json.dumps({'stopped': True}))
""")
print(f"PIE stopped: {r.get('stopped')}")
