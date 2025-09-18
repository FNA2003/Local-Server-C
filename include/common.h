#include <sys/socket.h>
#include <unistd.h>

/* Librerías necesarias para trabajo en sockets */
#include <sys/socket.h> // Las funciones de sockets
#include <sys/types.h>  // socklen_t
#include <sys/un.h>     // struct sockaddre_un


// Tamaño de los mensajes de los sockets
#define BUFF_SIZE 255
