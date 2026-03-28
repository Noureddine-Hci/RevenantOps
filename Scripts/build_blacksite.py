"""
BLACKSITE — Script de construction MCP
RevenantOps, Phase 10

Usage: Coller dans l'éditeur Python UE5 (mode Editor, PAS en PIE)
       OU exécuter via unreal-mcpython

Ce script:
  1. Supprime tous les StaticMeshActors existants
  2. Reconstruit la map Blacksite depuis zéro
  3. Replace le PlayerStart
  4. Affiche les positions pour les spawners et pickups
"""

import unreal

# ─── HELPERS ─────────────────────────────────────────────────────────────────

_cube_mesh = None
_cyl_mesh  = None

def get_cube():
    global _cube_mesh
    if _cube_mesh is None:
        _cube_mesh = unreal.load_asset('/Engine/BasicShapes/Cube.Cube')
    return _cube_mesh

def get_cylinder():
    global _cyl_mesh
    if _cyl_mesh is None:
        _cyl_mesh = unreal.load_asset('/Engine/BasicShapes/Cylinder.Cylinder')
    return _cyl_mesh

def spawn(label, mesh_fn, x, y, z, sx, sy, sz, pitch=0.0, yaw=0.0, roll=0.0):
    loc = unreal.Vector(x, y, z)
    rot = unreal.Rotator(pitch=pitch, yaw=yaw, roll=roll)
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor, loc, rot
    )
    actor.set_actor_label(label)
    actor.static_mesh_component.set_static_mesh(mesh_fn())
    actor.set_actor_scale3d(unreal.Vector(sx, sy, sz))
    return actor

def cube(label, x, y, z, sx, sy, sz, pitch=0.0, yaw=0.0, roll=0.0):
    return spawn(label, get_cube, x, y, z, sx, sy, sz, pitch, yaw, roll)

def cyl(label, x, y, z, sx, sy, sz):
    return spawn(label, get_cylinder, x, y, z, sx, sy, sz)

# ─── STEP 1: CLEAR ───────────────────────────────────────────────────────────

print("=" * 60)
print("[BLACKSITE] Étape 1/4 : Suppression des anciens actors...")
print("=" * 60)

removed = 0
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if isinstance(a, unreal.StaticMeshActor):
        unreal.EditorLevelLibrary.destroy_actor(a)
        removed += 1

print(f"[BLACKSITE] {removed} actors supprimés.")

# ─── CONSTANTES ──────────────────────────────────────────────────────────────
# Épaisseur mur = 30cm → scale = 0.3
T  = 0.3
# HANGAR: hauteur 800cm → scale_z=8, centre z=400
HH, HZ = 8.0, 400
# CORRIDOR: hauteur 400cm → scale_z=4, centre z=200
CH, CZ = 4.0, 200
# SERVER ROOM: hauteur 500cm → scale_z=5, centre z=250
SH, SZ = 5.0, 250
# LOADING DOCK: hauteur 600cm → scale_z=6, centre z=300
LH, LZ = 6.0, 300
# CTRL ROOM: Z plateforme=600, hauteur 400cm → murs centre z=600+200=800
KH, KZ = 4.0, 800

# ─── STEP 2: SOLS & PLAFONDS ─────────────────────────────────────────────────

print("[BLACKSITE] Étape 2/4 : Construction sols et plafonds...")

# Sols (centre Z=-50, scale_z=1, top surface à Z=0)
# HANGAR: X:-4000→+1000 (5000), Y:-2000→+1000 (3000) → centre (-1500, -500)
cube("FLOOR_Hangar",       -1500,  -500, -50,   50,  30,  1)
# CORRIDOR: X:+1000→+2000 (1000), Y:-300→+300 (600) → centre (1500, 0)
cube("FLOOR_Corridor",      1500,     0, -50,   10,   6,  1)
# SERVER ROOM: X:+2000→+4000 (2000), Y:-2000→+2000 (4000) → centre (3000, 0)
cube("FLOOR_ServerRoom",    3000,     0, -50,   20,  40,  1)
# LOADING DOCK: X:-4000→+1000 (5000), Y:+1000→+2500 (1500) → centre (-1500, 1750)
cube("FLOOR_LoadingDock",  -1500,  1750, -50,   50,  15,  1)
# N_CONNECT: X:+1000→+2000 (1000), Y:+1000→+2000 (1000) → centre (1500, 1500)
cube("FLOOR_NConnect",      1500,  1500, -50,   10,  10,  1)
# CTRL ROOM plateforme: X:-1500→0 (1500), Y:+1400→+2400 (1000) → centre (-750, 1900)
# Plateforme à Z=600: centre Z = 600-50 = 550
cube("FLOOR_CtrlRoom",      -750,  1900, 550,   15,  10,  1)

# Plafonds (bottom face = hauteur intérieure, centre = hauteur + 50)
cube("CEIL_Hangar",        -1500,  -500,  850,  50,  30,  1)  # 800cm → centre 850
cube("CEIL_Corridor",       1500,     0,  450,  10,   6,  1)  # 400cm → centre 450
cube("CEIL_ServerRoom",     3000,     0,  550,  20,  40,  1)  # 500cm → centre 550
cube("CEIL_LoadingDock",   -1500,  1750,  650,  50,  15,  1)  # 600cm → centre 650
cube("CEIL_NConnect",       1500,  1500,  650,  10,  10,  1)  # 600cm → centre 650
cube("CEIL_CtrlRoom",       -750,  1900, 1050,  15,  10,  1)  # 400cm au-dessus plateforme → 600+400+50=1050

# ─── STEP 3: MURS EXTÉRIEURS ─────────────────────────────────────────────────

print("[BLACKSITE] Étape 3/4 : Construction murs...")

# ── SUD ──
# Hangar sud: Y=-2000, X:-4000→+1000, centre X=-1500
cube("WALL_S_Hangar",      -1500, -2015,   HZ,  50,   T,  HH)
# Server room sud: Y=-2000, X:+2000→+4000, centre X=+3000
cube("WALL_S_Server",       3000, -2015,   SZ,  20,   T,  SH)

# ── OUEST ──
# Hangar ouest: X=-4000, Y:-2000→+1000, centre Y=-500
cube("WALL_W_Hangar",      -4015,  -500,   HZ,   T,  30,  HH)
# Loading dock ouest: X=-4000, Y:+1000→+2500, centre Y=+1750
cube("WALL_W_LoadDock",    -4015,  1750,   LZ,   T,  15,  LH)

# ── NORD ──
# Loading dock nord: Y=+2500, X:-4000→+1000, centre X=-1500
cube("WALL_N_LoadDock",    -1500,  2515,   LZ,  50,   T,  LH)
# Server room nord: Y=+2000, X:+2000→+4000, centre X=+3000
cube("WALL_N_Server",       3000,  2015,   SZ,  20,   T,  SH)

# ── EST ──
# Server room est: X=+4000, Y:-2000→+2000, centre Y=0
cube("WALL_E_Server",       4015,     0,   SZ,   T,  40,  SH)

# ─── MURS INTÉRIEURS ─────────────────────────────────────────────────────────

# Hangar EST (séparateur hangar/corridor, porte à Y:-300→+300)
# Segment sud: Y:-2000→-300 (1700 → scale 17), centre Y=-1150
cube("WALL_HE_Sud",         1015, -1150,   HZ,   T,  17,  HH)
# Segment nord: Y:+300→+1000 (700 → scale 7), centre Y=+650
cube("WALL_HE_Nord",        1015,   650,   HZ,   T,   7,  HH)

# Corridor murs nord/sud
cube("WALL_Corr_N",         1500,   315,   CZ,  10,   T,  CH)
cube("WALL_Corr_S",         1500,  -315,   CZ,  10,   T,  CH)

# Hangar/Loading Dock (Y=+1000) — 3 segments avec 2 ouvertures
# Porte principale: X:-3200→-2200 (1000cm)
# Accès escalier: X:-1500→0 (ouverture, l'escalier remplit l'espace)
# Segment O: X:-4000→-3200 (800 → scale 8), centre X=-3600
cube("WALL_HL_O",          -3600,  1015,   HZ,   8,   T,  HH)
# Segment M: X:-2200→-1500 (700 → scale 7), centre X=-1850
cube("WALL_HL_M",          -1850,  1015,   HZ,   7,   T,  HH)
# Segment E: X:0→+1000 (1000 → scale 10), centre X=+500
cube("WALL_HL_E",            500,  1015,   HZ,  10,   T,  HH)

# N_Connect SUD (bloque accès direct hangar→N_Connect)
cube("WALL_NC_S",           1500,  1015,   LZ,  10,   T,  LH)

# Server Room OUEST — corridor door (Y:-300→+300 ouvert)
# Segment sud: Y:-2000→-300 (1700 → scale 17), centre Y=-1150
cube("WALL_SRV_W_S",        2015, -1150,   SZ,   T,  17,  SH)
# Segment nord: Y:+300→+1000 (700 → scale 7), centre Y=+650
cube("WALL_SRV_W_N",        2015,   650,   SZ,   T,   7,  SH)

# Server Room/N_Connect (X=+2000, Y:+1000→+2000) — porte à Y:+1300→+1700
# Segment bas: Y:+1000→+1300 (300 → scale 3), centre Y=+1150
cube("WALL_SRV_NC_S",       2015,  1150,   LZ,   T,   3,  LH)
# Segment haut: Y:+1700→+2000 (300 → scale 3), centre Y=+1850
cube("WALL_SRV_NC_N",       2015,  1850,   LZ,   T,   3,  LH)

# Loading Dock EST (X=+1000, Y:+2000→+2500, au-dessus de N_Connect)
cube("WALL_LD_E",           1015,  2250,   LZ,   T,   5,  LH)

# ── CTRL ROOM (plateforme Z=600) ──
# Murs de la plateforme (au sol, bloquent accès sans escalier)
# Base ouest: X=-1500, Y:+1400→+2400 (1000 → scale 10), centre Y=+1900
cube("WALL_Ctrl_Base_O",   -1515,  1900,   LZ,   T,  10,  LH)
# Base est: X=0, Y:+1400→+2400
cube("WALL_Ctrl_Base_E",     15,  1900,   LZ,   T,  10,  LH)
# Base nord: Y=+1400, X:-1500→0 (1500 → scale 15), centre X=-750
# Ouverture escalier: X:-300→+300 — Segment O: X:-1500→-300 (scale 12, centre -900)
cube("WALL_Ctrl_Base_N_O", -900,  1415,   LZ,  12,   T,  LH)
# Segment E: X:+300→0 — oups, X=0 est la limite est, X:300→0 est vide, pas de mur ici
# (L'escalier est centré en X=0 côté est du control room, donc...)
# Escalier à X:-200→+200, centré X=0. Base nord: X:-1500→-200 + X:+200→0
cube("WALL_Ctrl_Base_N_E",  100,  1415,   LZ,   2,   T,  LH)  # X:0→+200 (200 → scale 2)

# Murs élevés de la salle (sur la plateforme Z=600)
cube("WALL_Ctrl_O",         -1515, 1900,   KZ,   T,  10,  KH)
cube("WALL_Ctrl_E",           15, 1900,   KZ,   T,  10,  KH)
cube("WALL_Ctrl_N",          -750, 2415,   KZ,  15,   T,  KH)
# Parapet sud (100cm de hauteur, les joueurs tirent par dessus)
cube("WALL_Ctrl_Para",       -750, 1415,  650,  15,   T, 1.0)

# ─── ESCALIER: HANGAR → CTRL ROOM ────────────────────────────────────────────
# 3 marches, chacune monte de 200cm, s'enfonce de 200→130→80cm vers le nord
# Marche 1: Z:0→200, Y:+1000→+1200 (200cm prof), centre (0, 1100, 100), scale_y=2
cube("STAIR_1",                0,  1100,  100,   4, 2.0,  2)
# Marche 2: Z:200→400, Y:+1000→+1130, centre (0, 1065, 300), scale_y=1.3
cube("STAIR_2",                0,  1065,  300,   4, 1.3,  2)
# Marche 3: Z:400→600, Y:+1000→+1080, centre (0, 1040, 500), scale_y=0.8
cube("STAIR_3",                0,  1040,  500,   4, 0.8,  2)
# Gardes-corps de l'escalier
cube("RAIL_Stair_O",         -215,  1075, 500,   T, 2.0,  4)
cube("RAIL_Stair_E",          215,  1075, 500,   T, 2.0,  4)

# ─── FEATURES HANGAR ─────────────────────────────────────────────────────────

# 4 piliers béton (cylindres 200cm diamètre, 800cm haut)
cyl("PILLAR_H_SO",  -3000, -1400,  400,  2,  2,  8)
cyl("PILLAR_H_SE",   -500, -1400,  400,  2,  2,  8)
cyl("PILLAR_H_NO",  -3000,   600,  400,  2,  2,  8)
cyl("PILLAR_H_NE",   -500,   600,  400,  2,  2,  8)

# 2 caisses équipement (150×100×100cm), centre Z=50
cube("CRATE_H_1",   -2200, -1600,   50, 1.5, 1.0, 1.0)
cube("CRATE_H_2",   -2200,   500,   50, 1.5, 1.0, 1.0)

# ─── FEATURES SERVER ROOM ────────────────────────────────────────────────────

# 8 server racks : 100×300×450cm → scale (1, 3, 4.5), centre Z=225
# Colonne ouest X=2500 / est X=3500, 4 rangées en Y
for i, y_pos in enumerate([-1500, -650, 200, 1050]):
    cube(f"RACK_O_{i+1}", 2500, y_pos, 225, 1.0, 3.0, 4.5)
    cube(f"RACK_E_{i+1}", 3500, y_pos, 225, 1.0, 3.0, 4.5)

# ─── FEATURES LOADING DOCK ───────────────────────────────────────────────────

# 4 containers : 300×120×200cm → scale (3, 1.2, 2), centre Z=100
cube("CONT_1",      -3500,  1300,  100, 3.0, 1.2, 2.0)  # face au joueur
cube("CONT_2",      -3500,  2100,  100, 3.0, 1.2, 2.0)  # coin NW
cube("CONT_3",      -2100,  2300,  100, 1.2, 3.0, 2.0)  # perpendiculaire
cube("CONT_4",       -800,  1400,  100, 3.0, 1.2, 2.0)  # près de l'escalier

# ─── STEP 4: PLAYER START & RÉSUMÉ ───────────────────────────────────────────

print("[BLACKSITE] Étape 4/4 : Repositionnement PlayerStart...")

ps_found = False
for a in unreal.EditorLevelLibrary.get_all_level_actors():
    if isinstance(a, unreal.PlayerStart):
        a.set_actor_location(unreal.Vector(-2000, -500, 100), False, True)
        a.set_actor_rotation(unreal.Rotator(pitch=0, yaw=90, roll=0), True)
        ps_found = True
        print("[BLACKSITE] PlayerStart repositionné à (-2000, -500, 100), face Est.")
        break

if not ps_found:
    print("[BLACKSITE] ATTENTION: PlayerStart non trouvé — placer manuellement à (-2000, -500, 100).")

print()
print("=" * 60)
print("[BLACKSITE] BUILD TERMINÉ!")
print("=" * 60)
print()
print("ACTIONS MANUELLES RESTANTES:")
print()
print("─ ENEMY SPAWNERS (ajouter BP_EnemySpawnPoint):")
print("  SP1: (-3800, -1800, 100)  SW hangar")
print("  SP2: (-3800,   600, 100)  NW hangar")
print("  SP3: (  200, -1800, 100)  S hangar")
print("  SP4: ( 3800, -1800, 100)  SE server room")
print("  SP5: ( 3800,  1800, 100)  NE server room")
print("  SP6: ( 3800,     0, 100)  E server room")
print("  SP7: (-3800,  2200, 100)  NW loading dock")
print("  SP8: (    0,  2400, 100)  N loading dock")
print()
print("─ PICKUPS:")
print("  TB1 +30s: ( -750,  1900,  700)  CTRL ROOM [risque max]")
print("  TB2 +20s: ( 3000,     0,  100)  SERVER ROOM center")
print("  TB3 +15s: (-3600,  2200,  100)  LOADING DOCK coin NW")
print("  A1 Ammo:  ( 1500,     0,  100)  CORRIDOR [chokepoint]")
print("  A2 Ammo:  (-2000,  -800,  100)  HANGAR [libre accès]")
print()
print("─ NAVMESH: NavMeshBoundsVolume center=(0, 250, 300) scale=(82, 52, 10)")
print()
print("─ LIGHTING: Ajouter PointLights rouges (emergencyred) partout.")
print("  Hangar: 4 lights aux piliers + 1 centrale")
print("  Corridor: 1 light rouge au centre")
print("  Server room: 4 lights bleues/froides entre les racks")
print("  Loading dock: 2 lights orangées")
print("  Ctrl room: 1 light rouge intense")
