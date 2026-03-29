"""Lance un fichier Python directement via le MCP TCP UE5."""
import socket, json, sys

def ue_file(filepath, timeout=120):
    with open(filepath, 'r', encoding='utf-8') as f:
        code = f.read()
    cmd = json.dumps({'type': 'python', 'code': code})
    with socket.create_connection(('127.0.0.1', 12029), timeout=timeout) as s:
        s.sendall(cmd.encode())
        buf = b''
        while True:
            c = s.recv(65536)
            if not c: break
            buf += c
    return buf.decode()

if __name__ == '__main__':
    path = sys.argv[1]
    print(f"Exécution de {path}...")
    result = ue_file(path)
    # Afficher le raw_result (stdout de UE5)
    try:
        outer = json.loads(result)
        inner_str = outer.get('result', '') or outer.get('raw_result', '')
        try:
            inner = json.loads(inner_str)
            raw = inner.get('raw_result', inner_str)
        except:
            raw = inner_str
        # Nettoyer les DeprecationWarnings
        lines = [l for l in raw.split('\n') if 'DeprecationWarning' not in l and '<string>' not in l]
        print('\n'.join(lines))
    except:
        print(result[:3000])
