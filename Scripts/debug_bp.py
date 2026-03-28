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

code = """
import unreal, json
try:
    bp = unreal.load_asset('/Game/Mercenaires/Weapons/BP_Pistol')
    msg = str(type(bp))
    gen_class = bp.generated_class()
    msg2 = str(type(gen_class))
    cdo = gen_class.get_default_object()
    msg3 = str(type(cdo))
    print(json.dumps({'bp': msg, 'gen_class': msg2, 'cdo': msg3, 'ok': True}))
except Exception as e:
    print(json.dumps({'ok': False, 'error': str(e)}))
"""

r = ue(code)
raw = r.get('raw_result', r.get('result', r))
print("RAW:", raw[:2000])
