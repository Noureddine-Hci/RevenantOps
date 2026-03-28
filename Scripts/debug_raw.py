import socket, json

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

# Minimal test
code = """
import unreal, json
bp = unreal.load_asset('/Game/Mercenaires/Weapons/BP_Pistol')
gen_class = bp.generated_class()
methods = [m for m in dir(gen_class) if 'default' in m.lower() or 'object' in m.lower()]
print(json.dumps({'methods': methods[:20], 'done': True}))
"""

raw = ue_raw(code)
print("FULL RAW RESPONSE:")
print(raw[:5000])
