import socket
import threading
import os

# Função para estabelecer uma conexão com o servidor
def connect_to_server(isTCP: bool=True):
    if isTCP:
        connect_TCP()
    else:
        connect_UDP()

def connect_TCP():
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect(("0.0.0.0", 1024))
        print(f"TCP {threading.get_ident()} conectado a {client_socket.getpeername()}")
        client_socket.close()
    except Exception as e:
        print(f"Erro de conexão: {str(e)}")

def connect_UDP():
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        client_socket.connect(("0.0.0.0", 1024))
        print(f"UDP {threading.get_ident()} conectado a {client_socket.getpeername()}")
        client_socket.close()
    except Exception as e:
        print(f"Erro de conexão: {str(e)}")
# Número de clientes que você deseja conectar
num_clients = 10

# Crie uma lista de threads para as conexões dos clientes
threads = []

# Inicie as threads para conectar os clientes
for i in range(num_clients):
    thread = threading.Thread(target=connect_to_server, args=[i % 2 == 0])
    threads.append(thread)
    thread.start()

# Aguarde todas as threads terminarem
for thread in threads:
    thread.join()

print("Todas as conexões foram estabelecidas.")
