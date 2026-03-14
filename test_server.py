import socket
import struct

server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
server.bind(('127.0.0.1', 4433))
server.listen(1)

print("[*] Python Server: Waiting for C++ client on 127.0.0.1:4433...")

conn, addr = server.accept()
print(f"[*] Connection from: {addr}")

try:
    while True:
        data_len = conn.recv(2)
        if not data_len: break
        
        length = struct.unpack('!H', data_len)[0]
        
        packet = conn.recv(length)
        if packet:
            version = packet[0] >> 4
            print(f"[+] Received IP Packet: {length} bytes, IPv{version}")
except Exception as e:
    print(f"Error: {e}")
finally:
    conn.close()

