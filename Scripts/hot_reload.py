import socket, json, re, time

def ue(code, timeout=180):
    cmd = json.dumps({'type': 'python', 'code': code})
    with socket.create_connection(('127.0.0.1', 12029), timeout=timeout) as s:
        s.sendall(cmd.encode())
        buf = b''
        while True:
            c = s.recv(65536)
            if not c: break
            buf += c
    raw = buf.decode()
    outer = json.loads(raw)
    inner = outer.get('result', '') or outer.get('raw_result', '')
    m = re.search(r'\{.*\}', inner, re.DOTALL)
    return json.loads(m.group()) if m else {'raw': inner[:500]}

print("Lancement Hot Reload / Live Coding...")
r = ue("""
import unreal, json
try:
    # Demander une recompilation via le gestionnaire de modules
    result = unreal.SystemLibrary.execute_console_command(None, 'LiveCoding.Compile')
    print(json.dumps({'triggered': True, 'result': str(result)}))
except Exception as e:
    print(json.dumps({'triggered': False, 'error': str(e)}))
""")
print(f"Hot Reload : {r}")
