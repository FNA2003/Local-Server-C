#include "common.h"

#include <pthread.h> // Incluimos hilos para poder manejar las query's


/*
 * TODO: Si hay problemas luego de la función socket(), debemos
 * cerrar el file descriptor del servidor.
 * 		- Agregar el manejo de SIGINT, SIGQUIT, etc para cerrar todo.
 * 		- Agregar manejo de threads para atender a cada cliente
 * TOFIX: Por el momento, solamente se recibe un mensaje
 * del cliente, y, se retorna una confirmación (mostrando antes el mensaje del cliente)
 * esto es, escencialmente, para probar rapidamente el uso de sockets.
 */
int main(void) {
	int server_fd, client_fd;			// File Descriptor de los sockets cliente-servidor
	struct sockaddr_in direccionSocket; // Estructuras para actualizar y recibir direcciones de sockets
	socklen_t tamanioAddr;				// Tamaño de la estructura anterior
	
	int ret;							// Entero para almacenar valores de retorno de las funciones
	char buffer[BUFF_SIZE + 1];			// Mensaje de entrada
	int clientesAtendidos[MAX_BACKLOG]; // Acá, vamos a almacenar a cada file descriptor que saquemos del backlog, solamente agregamos si hay espacio y sacamos cuando termina la conexión


	
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

	// Ponemos al socket de nuestro servidor en modo 'Pasivo' (Escucha)
	ret = listen(server_fd, MAX_BACKLOG);
	if (ret < 0) print_exit("Error al poner en 'pasivo' al servidor", 3); 

	printf("Servidor esperando conexiones\n\tBacklog:%d\n\tPuerto:%d\n", MAX_BACKLOG, SERV_PORT);

	while (1) {
		memset(buffer, '\0', BUFF_SIZE + 1); // Vaciamos el buffer en cada lectura

		client_fd = accept(server_fd, (struct sockaddr*)&direccionSocket, &tamanioAddr);
		if (client_fd < 0) print_exit("Error al aceptar una conexión", 4);

		printf("Conexión aceptada!\n");

		recv(client_fd, buffer, BUFF_SIZE, 0);
		printf("\tMensaje recibido: %s\n", buffer);

		send(client_fd, "Mensaje recibido!\0", 18, 0);
		printf("\tConfirmación enviada al cliente!\n");

		if (strncmp(buffer, "chau", 4) == 0) break; /* Cuando se escriba "chau" cierra todo
													   nota; usamos strncmp() y no strcmp() para evitar
													   tener que remover saltos de línea o espacios al final */
	}
		
	close(client_fd);
	close(server_fd);
	return 0;
}
