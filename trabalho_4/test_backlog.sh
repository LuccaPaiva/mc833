#!/bin/bash

# Loop para o número de clientes (5 a 15)
for clients in {5..15}; do
    # Loop para o valor de backlog (0 a 12)
    for backlog in {0..12}; do
        echo "Rodando servidor com $clients clientes e backlog $backlog"
        ./servidor 1024 $backlog &  # Roda o servidor em segundo plano
        sleep 1  # Dê tempo para o servidor iniciar

        # Execute o cliente Python
        python3 cliente_script.py $clients

        # Aguarde algum tempo para que as conexões sejam estabelecidas
        sleep 5  # Você pode ajustar esse valor conforme necessário

        # Mate o servidor
        pkill servidor
        sleep 1  # Dê tempo para o servidor terminar
    done
done
