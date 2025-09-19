#ifndef COMMON_H
#define COMMON_H


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/wait.h>
// Librerías necesarias para trabajo en sockets
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>


#define BUFF_SIZE   255  // Tamaño del buffer por cada envio y recepción de los sockets
#define SERV_PORT   3377 // Puerto (Modificable)
#define MAX_BACKLOG 5    // Procesos máximos en fila de espera (No es relevante, pues se asigna cada proceso a un hilo, por ende siempre se atiende al que llega)

// Información que le pasamos al thread hijo del canal de entrada comunicado
struct datos_cliente {
    struct sockaddr_in *socket_addr;
    int socket_fd;
};

void print_exit(char *errMsg, int extVal);
char **parse_command(const char *input);
void free_args(char **args);


#endif