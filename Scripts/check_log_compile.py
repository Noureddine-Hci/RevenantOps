import os, time

# Lire les 50 dernières lignes du log UE5
log_path = r"C:\Users\Noureddine\.gemini\antigravity\scratch\RevenantOps\Saved\Logs\RevenantOps.log"

if os.path.exists(log_path):
    with open(log_path, 'r', encoding='utf-8', errors='replace') as f:
        lines = f.readlines()
    # Chercher les lignes de compilation des 200 dernières
    recent = lines[-200:]
    compile_lines = [l.strip() for l in recent if any(k in l for k in
        ['LiveCoding', 'Compile', 'HotReload', 'WeaponBase', 'WeaponMeshSM',
         'error', 'Error', 'warning', 'SUCCESS', 'FAILED', 'Build'])]
    print(f"Total lignes log: {len(lines)}")
    print("\nLignes compilation pertinentes:")
    for l in compile_lines[-30:]:
        print(f"  {l}")
else:
    print(f"Log non trouve: {log_path}")
