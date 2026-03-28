import socket, json, re

def ue(code, timeout=60):
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
    return json.loads(m.group()) if m else {'raw': raw[:2000]}

code = """
import unreal, json

# Placeholder SKM (visible in hand, ugly but functional for playtest)
placeholder_skm = unreal.load_asset('/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple')

# Weapon configs: (path, name, category_str, fire_mode_str, damage, fire_rate, mag_size, pellets, reload_time, max_reserve)
weapons = [
    ('/Game/Mercenaires/Weapons/BP_Pistol',
     'Pistol', 'Pistol', 'SemiAuto', 35.0, 300.0, 12, 1, 1.5, 72),
    ('/Game/Mercenaires/Weapons/BP_AssaultRifle',
     'Assault Rifle', 'AssaultRifle', 'FullAuto', 25.0, 750.0, 30, 1, 2.2, 180),
    ('/Game/Mercenaires/Weapons/BP_SMG',
     'SMG', 'SMG', 'FullAuto', 18.0, 900.0, 25, 1, 2.0, 150),
    ('/Game/Mercenaires/Weapons/BP_Shotgun',
     'Shotgun', 'Shotgun', 'SemiAuto', 18.0, 90.0, 6, 8, 2.5, 36),
    ('/Game/Mercenaires/Weapons/BP_Sniper',
     'Sniper Rifle', 'Sniper', 'SemiAuto', 150.0, 60.0, 5, 1, 3.0, 20),
    ('/Game/Mercenaires/Weapons/BP_Melee',
     'Melee', 'Melee', 'SemiAuto', 80.0, 120.0, 1, 1, 0.3, 1),
]

cat_map = {
    'Pistol': unreal.EWeaponCategory.PISTOL,
    'SMG': unreal.EWeaponCategory.SMG,
    'AssaultRifle': unreal.EWeaponCategory.ASSAULT_RIFLE,
    'Shotgun': unreal.EWeaponCategory.SHOTGUN,
    'Sniper': unreal.EWeaponCategory.SNIPER,
    'Melee': unreal.EWeaponCategory.MELEE,
}
fire_map = {
    'SemiAuto': unreal.EWeaponFireMode.SEMI_AUTO,
    'FullAuto': unreal.EWeaponFireMode.FULL_AUTO,
}

results = []
for (path, name, cat_str, fire_str, dmg, fire_rate, mag, pellets, reload_t, reserve) in weapons:
    try:
        bp = unreal.load_asset(path)
        if bp is None:
            results.append({'weapon': name, 'ok': False, 'error': 'BP not found'})
            continue

        gen_class = bp.generated_class()
        cdo = gen_class.get_default_object()

        # Set weapon identity
        cdo.set_editor_property('WeaponName', unreal.Text(name))
        cdo.set_editor_property('WeaponCategory', cat_map[cat_str])
        cdo.set_editor_property('FireMode', fire_map[fire_str])

        # Set damage
        cdo.set_editor_property('BaseDamage', dmg)
        cdo.set_editor_property('PelletsPerShot', pellets)

        # Set fire stats
        cdo.set_editor_property('FireRate', fire_rate)

        # Set ammo
        cdo.set_editor_property('MagazineSize', mag)
        cdo.set_editor_property('MaxReserveAmmo', reserve)
        cdo.set_editor_property('ReloadTime', reload_t)

        # Try to assign placeholder mesh to WeaponMesh component
        mesh_ok = False
        try:
            weapon_mesh_comp = cdo.weapon_mesh
            if weapon_mesh_comp and placeholder_skm:
                weapon_mesh_comp.set_editor_property('SkeletalMesh', placeholder_skm)
                mesh_ok = True
        except Exception as me:
            mesh_ok = False

        # Mark BP dirty + save
        unreal.EditorAssetLibrary.save_asset(path, only_if_is_dirty=False)

        results.append({'weapon': name, 'ok': True, 'mesh_ok': mesh_ok})
    except Exception as e:
        results.append({'weapon': name, 'ok': False, 'error': str(e)})

print(json.dumps({'results': results}))
"""

r = ue(code)
data = get(r)
print("=== Weapon Configuration ===")
for res in data.get('results', []):
    status = "OK" if res.get('ok') else f"FAIL: {res.get('error')}"
    mesh = " | mesh: OK" if res.get('mesh_ok') else " | mesh: skipped"
    print(f"  {res.get('weapon','?'):20} {status}{mesh}")
if 'raw' in data:
    print("RAW:", data['raw'])
