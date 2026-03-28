"""Helper: envoie du code Python au serveur TCP UE5 (port 12029) et affiche le résultat."""
import socket, json, sys

def ue(code, timeout=30):
    cmd = json.dumps({'type': 'python', 'code': code})
    with socket.create_connection(('127.0.0.1', 12029), timeout=timeout) as s:
        s.sendall(cmd.encode())
        buf = b''
        while True:
            c = s.recv(16384)
            if not c: break
            buf += c
    return json.loads(buf.decode())

if __name__ == '__main__':
    code = sys.stdin.read() if len(sys.argv) < 2 else sys.argv[1]
    r = ue(code)
    print(json.dumps(r, indent=2, ensure_ascii=False))
