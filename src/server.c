#include "common.h"

#include <pthread.h> // Incluimos hilos para poder manejar las query's


/*
 * TODO: Si hay problemas luego de la función socket(), debemos
 * cerrar el file descriptor del servidor.
 * TOFIX: Por el momento, solamente se recibe un mensaje
 * del cliente, y, se retorna una confirmación (mostrando antes el mensaje del cliente)
 * esto es, escencialmente, para probar rapidamente el uso de sockets.
 */
int main(void) {
	int server_fd, client_fd;			// File Descriptor de los sockets cliente-servidor
	int flag = 1;						// Flag del bucle "demonio" infinito, permite cerrarlo
	int ret;							// Entero para almacenar valores de retorno de las funciones
	struct sockaddr_in direccionSocket; // Estructuras de las direcciones
	socklen_t tamanioAddr;
	char buffer[BUFF_SIZE + 1];			// Mensaje de entrada


	buffer[BUFF_SIZE] = '\0';

	server_fd = socket(AF_INET, SOCK_STREAM, 0);  // Hacemos un socket IPv4 y no local por necesidad de usar TCP
	if (server_fd < 0) print_exit("Error en la asignación del fd del socket del servidor", 1);

	/* Configuración del socket del servidor */
	direccionSocket.sin_family = AF_INET;
	direccionSocket.sin_port = htons(SERV_PORT);  // Puerto de ejemplo
	direccionSocket.sin_addr.s_addr = INADDR_ANY; // Escuchar en cualquier IP local

	tamanioAddr = sizeof(direccionSocket);

	// Vinculamos el socket a la dirección del file descriptor
	ret = bind(server_fd, (struct sockaddr*)&direccionSocket, tamanioAddr);
	if (ret < 0) print_exit("Server socket binding error", 2);

	// Asumimos que el servidor escuchará unicamente a nuestro cliente de ejemplo
	ret = listen(server_fd, MAX_BACKLOG);
	if (ret < 0) print_exit("Error al poner en 'pasivo' al servidor", 3); 

	printf("Servidor esperando conexiones\n\tBacklog:%d\n\tPuerto:%d\n", MAX_BACKLOG, SERV_PORT);

	while (flag) {
		client_fd = accept(server_fd, (struct sockaddr*)&direccionSocket, &tamanioAddr);
		if (client_fd < 0) print_exit("Error al aceptar una conexión", 4);

		printf("Conexión aceptada!\n");

		recv(client_fd, buffer, BUFF_SIZE, 0);
		printf("\tMensaje recibido: %s\n", buffer);

		send(client_fd, "Mensaje recibido!\0", 18, 0);
		printf("\tConfirmación enviada al cliente!\n");

		close(client_fd);
	}

	close(server_fd);
	return 0;
}
