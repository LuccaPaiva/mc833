#!/bin/bash

# Verifique se foram fornecidos argumentos
if [ "$#" -ne 2 ]; then
    echo "Uso: $0 <backlog> <numero_cliente>"
    exit 1
fi

# Argumentos para o servidor
num_clientes="$1"

# Inicie o servidor em segundo plano com os argumentos fornecidos
./servidor 1024 "$num_clientes" &

# Aguarde um breve atraso para garantir que o servidor tenha tempo de inicializar
sleep 2

# Argumento para o cliente
parametro_cliente="$2"

# Inicie o cliente em segundo plano com o argumento fornecido
python3 cliente_script.py "$parametro_cliente"
