import time, sys

LOG = r'C:\Users\Noureddine\.gemini\antigravity\scratch\RevenantOps\Saved\Logs\RevenantOps.log'
DURATION = int(sys.argv[1]) if len(sys.argv) > 1 else 25
KEYWORDS = sys.argv[2:] if len(sys.argv) > 2 else []

with open(LOG, 'r', encoding='utf-8', errors='replace') as f:
    f.seek(0, 2)
    deadline = time.time() + DURATION
    while time.time() < deadline:
        line = f.readline()
        if line:
            stripped = line.strip()
            if not stripped:
                continue
            if KEYWORDS:
                if any(k.lower() in stripped.lower() for k in KEYWORDS):
                    print(stripped, flush=True)
            else:
                print(stripped, flush=True)
        else:
            time.sleep(0.15)
