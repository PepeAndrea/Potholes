#!/bin/bash

install() {
    #Create file
    mkdir Database Log
    touch Database/database.db Log/log.txt

    #setup database
    gcc Setup/setup.c -o potholes_server_setup -lsqlite3
    ./potholes_server_setup

    #compile server
    gcc src/* -o potholes_server -lsqlite3 -pthread -lm
  
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
    if [[ -f "Database/database.db" ]] && [[ -f "Log/log.txt" ]] && [[ -f "potholes_server" ]]; then
        ./potholes_server &
    else
        install
        ./potholes_server &
    fi

    ;;
  *)
    printf "Parametro richiesto\n\n Parametri validi:\n
    1)install:Esegue il processo di installazione.\n
    2)start:Avvia il server\n"
    ;;
esac

