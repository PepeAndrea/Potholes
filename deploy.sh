#!/bin/bash

install() {
    gcc Setup/setup.c -o potholes_server_setup -lsqlite3
    ./potholes_server_setup

    gcc src/* -o potholes_server -lsqlite3 -pthread
    ./potholes_server &
}



if [ $# -lt 1 ]; then
    printf "Parametro richiesto\n\n Parametri validi:\n
    1)install:Esegue il processo di installazione.\n
    2)start:Avvia il server\n"
    exit;
fi

case $1 in

  "install")
    install
    ;;

  "start")
    if [[ -f "Database/database.db" ]] && [[ -f "potholes_server" ]]; then
        ./potholes_server &
    else
        install
    fi

    ;;
  *)
    printf "Parametro richiesto\n\n Parametri validi:\n
    1)install:Esegue il processo di installazione.\n
    2)start:Avvia il server\n"
    ;;
esac

