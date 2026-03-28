"""
RevenantOps — Remote Control Query Helper
Interroge l'état du jeu pendant PIE via Remote Control API (port 30010).

Usage:
    python Scripts/rc_query.py status          # état général du jeu
    python Scripts/rc_query.py score           # score actuel
    python Scripts/rc_query.py wave            # vague actuelle
    python Scripts/rc_query.py timer           # timer restant
    python Scripts/rc_query.py logs [n]        # n dernières lignes du log (défaut: 30)
    python Scripts/rc_query.py watch           # mode watch: refresh toutes les 2s
"""

import sys
import json
import time
import urllib.request
import urllib.error

RC_BASE = "http://localhost:30010"
LOG_PATH = r"C:\Users\Noureddine\.gemini\antigravity\scratch\RevenantOps\Saved\Logs\RevenantOps.log"

# Chemins des objets pendant PIE (prefix UEDPIE_0_)
GAME_STATE_PATH = "/Game/ThirdPerson/Lvl_ThirdPerson.UEDPIE_0_Lvl_ThirdPerson:PersistentLevel.BP_MercenairesGameState_C_0"


def rc_get_property(object_path: str, property_name: str):
    """Lit une propriété via Remote Control API."""
    url = f"{RC_BASE}/remote/object/property"
    payload = json.dumps({
        "objectPath": object_path,
        "propertyName": property_name,
        "access": "READ_ACCESS"
    }).encode()
    req = urllib.request.Request(url, data=payload,
                                  headers={"Content-Type": "application/json"},
                                  method="PUT")
    try:
        with urllib.request.urlopen(req, timeout=3) as resp:
            return json.loads(resp.read())
    except urllib.error.URLError as e:
        return {"error": str(e)}


def rc_call_function(object_path: str, function_name: str, params: dict = None):
    """Appelle une fonction Blueprint via Remote Control API."""
    url = f"{RC_BASE}/remote/object/call"
    payload = json.dumps({
        "objectPath": object_path,
        "functionName": function_name,
        "parameters": params or {},
        "generateTransaction": False
    }).encode()
    req = urllib.request.Request(url, data=payload,
                                  headers={"Content-Type": "application/json"},
                                  method="PUT")
    try:
        with urllib.request.urlopen(req, timeout=3) as resp:
            return json.loads(resp.read())
    except urllib.error.URLError as e:
        return {"error": str(e)}


def rc_list_objects(class_name: str = ""):
    """Liste les objets dans le monde PIE."""
    url = f"{RC_BASE}/remote/search/assets"
    payload = json.dumps({"query": class_name, "limit": 50}).encode()
    req = urllib.request.Request(url, data=payload,
                                  headers={"Content-Type": "application/json"},
                                  method="PUT")
    try:
        with urllib.request.urlopen(req, timeout=3) as resp:
            return json.loads(resp.read())
    except urllib.error.URLError as e:
        return {"error": str(e)}


def read_log_tail(n: int = 30) -> list[str]:
    """Lit les n dernières lignes du log UE."""
    try:
        with open(LOG_PATH, "r", encoding="utf-8", errors="replace") as f:
            lines = f.readlines()
        return lines[-n:]
    except Exception as e:
        return [f"[ERREUR lecture log] {e}"]


def print_status():
    """Affiche un tableau de bord complet."""
    print("\n" + "=" * 60)
    print("  RevenantOps — PIE Status")
    print("=" * 60)

    props = {
        "Score": "Score",
        "CurrentWave": "CurrentWave",
        "RemainingTime": "RemainingTime",
        "bMatchActive": "bMatchActive",
        "bMatchEnded": "bMatchEnded",
    }

    for label, prop in props.items():
        result = rc_get_property(GAME_STATE_PATH, prop)
        if "error" in result:
            print(f"  {label:<20} [PIE inactif ou RC non connecté]")
            break
        val = result.get("propertyValue", {}).get(prop, "?")
        print(f"  {label:<20} {val}")

    print("=" * 60 + "\n")


def watch_mode():
    """Refresh le status toutes les 2 secondes."""
    print("Mode watch actif (Ctrl+C pour stopper)\n")
    try:
        while True:
            print_status()
            time.sleep(2)
    except KeyboardInterrupt:
        print("\nWatch stoppé.")


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else "status"

    if cmd == "status":
        print_status()

    elif cmd == "score":
        r = rc_get_property(GAME_STATE_PATH, "Score")
        print(json.dumps(r, indent=2))

    elif cmd == "wave":
        r = rc_get_property(GAME_STATE_PATH, "CurrentWave")
        print(json.dumps(r, indent=2))

    elif cmd == "timer":
        r = rc_get_property(GAME_STATE_PATH, "RemainingTime")
        print(json.dumps(r, indent=2))

    elif cmd == "logs":
        n = int(sys.argv[2]) if len(sys.argv) > 2 else 30
        lines = read_log_tail(n)
        print(f"\n--- Dernières {n} lignes de RevenantOps.log ---")
        for line in lines:
            print(line, end="")
        print()

    elif cmd == "watch":
        watch_mode()

    else:
        print(__doc__)


if __name__ == "__main__":
    main()
