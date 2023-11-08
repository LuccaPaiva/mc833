for backlog in {0..15}; do
    echo "Rodando servidor com backlog $backlog" | tee -a elapsed_time.txt 
    ./servidor 1025 $backlog &  # Roda o servidor em segundo plano
    sleep 1  # Dê tempo para o servidor iniciar

    # Execute o cliente Python
    python3 cliente_script.py

    # Aguarde algum tempo para que as conexões sejam estabelecidas
    sleep 2  # Você pode ajustar esse valor conforme necessário

    # Mate o servidor
    pkill servidor
    sleep 1  # Dê tempo para o servidor terminar
done