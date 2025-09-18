#ifndef COMMON_H
#define COMMON_H


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Librerías necesarias para trabajo en sockets */
#include <sys/socket.h> // Las funciones de sockets
#include <netinet/in.h> // Para sockaddr_in
#include <arpa/inet.h>  // Para funciones como inet_addr


// Tamaño de los mensajes de los sockets
#define BUFF_SIZE 255

#define SERV_PORT 3377
#define MAX_BACKLOG 1


// Función para mostrar un mensaje de errno y hacer un exit para finalizar el programa
void print_exit(char *errMsg, int extVal);


#endif