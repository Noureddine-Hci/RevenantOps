"""
copy_megasample_sounds.py
Copie les sons MilitaryWeapSilver depuis ProjectMegaSample vers RevenantOps.
Run OUTSIDE UE5 (regular Python, pas MCP) :
    python Scripts\copy_megasample_sounds.py
"""
import shutil
import os

SRC_ROOT = r"D:\gt\extracted\ProjectMegaSample\Content\_WeaponsPacks\MilitaryWeapSilver\Sound"
DST_ROOT = r"C:\Users\Noureddine\Documents\Unreal Projects\Revenant OPS DZ\Content\_WeaponsPacks\MilitaryWeapSilver\Sound"

if not os.path.isdir(SRC_ROOT):
    print(f"[ERROR] Source introuvable : {SRC_ROOT}")
    exit(1)

if os.path.exists(DST_ROOT):
    print(f"[INFO] Dossier destination existe déjà — merge (nouveaux fichiers uniquement)")
    copied = 0
    skipped = 0
    for root, dirs, files in os.walk(SRC_ROOT):
        rel = os.path.relpath(root, SRC_ROOT)
        dst_dir = os.path.join(DST_ROOT, rel)
        os.makedirs(dst_dir, exist_ok=True)
        for f in files:
            src_f = os.path.join(root, f)
            dst_f = os.path.join(dst_dir, f)
            if not os.path.exists(dst_f):
                shutil.copy2(src_f, dst_f)
                print(f"  COPY {os.path.join(rel, f)}")
                copied += 1
            else:
                skipped += 1
    print(f"\nDone — {copied} copiés, {skipped} ignorés (déjà présents)")
else:
    print(f"Copie complète → {DST_ROOT}")
    shutil.copytree(SRC_ROOT, DST_ROOT)
    total = sum(len(files) for _, _, files in os.walk(DST_ROOT))
    print(f"Done — {total} fichiers copiés")
