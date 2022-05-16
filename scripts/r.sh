#! bin/bash

gcc -c ui.c -o ui.o
gcc -c auth.c -o auth.o
gcc -c main.c -o main.o
gcc main.o ui.o auth.o -o main -lpthread -lpam
