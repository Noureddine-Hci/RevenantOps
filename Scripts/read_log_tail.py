import sys

LOG = r'C:\Users\Noureddine\.gemini\antigravity\scratch\RevenantOps\Saved\Logs\RevenantOps.log'
N = int(sys.argv[1]) if len(sys.argv) > 1 else 80

with open(LOG, 'r', encoding='utf-8', errors='replace') as f:
    lines = f.readlines()

for line in lines[-N:]:
    print(line, end='')
