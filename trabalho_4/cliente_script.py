import socket
import threading
import os
import time

start = time.time()
# Função para estabelecer uma conexão com o servidor
def connect_to_server():
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.connect(("0.0.0.0", 1025))
        print(f"{threading.get_ident()} conectado a {client_socket.getpeername()}")
        client_socket.close()
    except Exception as e:
        print(f"Erro de conexão: {str(e)}")

# Número de clientes que você deseja conectar
num_clients = 10

# Crie uma lista de threads para as conexões dos clientes
threads = []

# Inicie as threads para conectar os clientes
for _ in range(num_clients):
    thread = threading.Thread(target=connect_to_server)
    threads.append(thread)
    thread.start()

# Aguarde todas as threads terminarem
for thread in threads:
    thread.join()

end = time.time()
f = open("elapsed_time.txt", "a")
delta_t = end - start
s = "elapsed time:\t\t\t" + str(round(delta_t, 3)) + " s\n\n"
f.write(s)
f.close()
print("Todas as conexões foram estabelecidas.")
