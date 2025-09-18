#include "common.h"
#include <signal.h>
#include <pthread.h> // Incluimos hilos para poder manejar las query's


volatile int running = 1; // Variable global para 'frenar' el bucle de mainLoop() -> handlerSenial()
volatile int server_sock; // File descriptor del socket del servidor


int incializarServSocket(void);
void mainLoop();
void* handlerHilo(void*);
void handlerSenial(int);


int main(void) {
	/* Configuramos la salida del servidor */
	signal(SIGINT, handlerSenial);
	signal(SIGQUIT, handlerSenial);
	

	printf("Preparando servidor...\n");
	// Inicializamos y preparamos el socket, si hay error la función lo maneja por si misma
	server_sock = incializarServSocket();

	printf("Servidor activo... Esperando conexiones\n\tBacklog:%d\n\tPuerto:%d\nPara terminar pulse 'Ctrl+C'\n", MAX_BACKLOG, SERV_PORT);

	// Bucle principal, solamente "sale" de esta función cuando finaliza la comunicación o se interrumpe el servidor
	mainLoop();

	printf("Cerrando conexión!\n");
	close(server_sock);

	return 0;
}


/* 
 * Función que retorna un entero correspondiente a un fd de un socket 
 * asignado a una comunicación TCP con IPv4, en caso de error, cerrará el programa
 * con un mensaje de error. Nota; el socket será configurado como socket pasivo
 * es decir, para la lectura desde el lado de un servidor.
 */
int incializarServSocket() {
	int sock_fd;
	struct sockaddr_in sockInfo;
	socklen_t sockLen;
	

	/* Si podemos, creamos un file descriptor para el socket IPv4, sino, cerramos */
	if ( (sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		print_exit("Error al crear un socket fd para el servidor", EXIT_FAILURE);

	/* Completamos la información del socket para pasarlo al kernel */
	sockInfo.sin_family = AF_INET;
	sockInfo.sin_port = htons(SERV_PORT);  // Variable global, vea common.h
	sockInfo.sin_addr.s_addr = INADDR_ANY; // Escuchar en cualquier IP local

	/* Tamaño de la estructura que tiene la información del socket */
	sockLen = sizeof(sockInfo);

	/* Le asignamos al file descriptor la información configurada anteriormente, o retornamos por error */
	if (bind(sock_fd, (struct sockaddr*)&sockInfo, sockLen) < 0) {
		close(sock_fd);
		print_exit("Error en bind()", EXIT_FAILURE);
	}
	/* Ponemos al socket en modo 'pasivo' (Escucha), si no podemos, cerramos con error */
	if (listen(sock_fd, MAX_BACKLOG) < 0) {
		close(sock_fd);
		print_exit("Error en listen()", EXIT_FAILURE);
	}

	/* Si todo sale bien, retornamos el número del fd asignado al socket del servidor */
	return sock_fd;
}

void mainLoop() {
	struct sockaddr_in *client_addr;
	socklen_t addr_len;
	int cliente_sock;
	pthread_t hilo;

	while(running) {
		// Creamos una dirección para cada socket de cada cliente
		client_addr = malloc(sizeof(struct sockaddr_in));
		addr_len = sizeof(struct sockaddr_in);
		// Esperamos a recibir un cliente 
		cliente_sock = accept(server_sock, (struct sockaddr*)client_addr,&addr_len);

		if (cliente_sock < 0) {
			free(client_addr);
			close(server_sock);
			print_exit("Error en accept(), servidor", EXIT_FAILURE);
		}

		struct datosCliente *data = malloc(sizeof(struct datosCliente));
		data->socketAddr = client_addr;
		data->socketFD = cliente_sock;

		pthread_create(&hilo, NULL, handlerHilo, data);
		pthread_detach(hilo);
	}
}

void* handlerHilo(void *args) {
	struct datosCliente *data = (struct datosCliente*) args;
	char buffer[BUFF_SIZE + 1];
	char **argumentos;


	/* Leemos siempre y cuando no haya un error = -1 ó haya un cierre ordenado del cliente = 0 */
	while(recv(data->socketFD, buffer, BUFF_SIZE, 0) > 0) {
		printf("Se recibió el mensaje: %s", buffer);		
		
		// Revisamos si no se envió el mensaje de salida
		if (strncmp(buffer, "chau", 4) == 0)
			break;
		
		// Pasamos la string recibida a una lista nulo terminada
		argumentos = parse_command(buffer);
		int p[2]; // Pipe para la comunicación padre-proceso
		pid_t pid;
		size_t n; // Cantidad de caracteres leídos

		pipe(p);
		
		if (!(pid = fork())) { // Proceso hijo, ejecuta los comandos y los transmite al pipe

			close(p[0]);               // El proceso hijo no lee del pipe
			dup2(p[1], STDOUT_FILENO); // Redirigimos la salida estandar al pipe
			close(p[1]);               // Una vez redirigida la salida al pipe, podemos cerrar su referencia

			
			// Ejecución desde el hijo
			execvp(argumentos[0], argumentos);

			perror("'execvp' en Hijo");
			exit(EXIT_FAILURE);

		} else {  // Proceso padre, lee cada salida del hijo por el pipe
			close(p[1]);

			while((n=read(p[0], buffer, BUFF_SIZE-1)) > 0) {
				buffer[n] = '\0';
				send(data->socketFD, buffer, BUFF_SIZE+1,0);
			}

			close(p[0]);
			waitpid(pid, 0, 0);
		}

		// Liberamos la memoria usada por la lista
		free_args(argumentos);

		// Simpre limpiamos el buffer 
		memset(buffer, '\0', BUFF_SIZE+1);
	}

	printf("Cerrando canal de comunicación con cliente\n");

	// Liberamos la memoria dinámica y cerramos el file descriptor del cliente
	free(data->socketAddr);
	close(data->socketFD);
	free(data);

	// Salida del hilo
	pthread_exit(NULL);
}

void handlerSenial(int senialN) {
	printf("\nCerando el servidor...\n");
	
	running = 0; 		// 'Frenamos' el bucle principal
	close(server_sock); // Esto cerrará accept() y dará error para recv()
	sleep(4);			// TOFIX: Damos tiempo a que los hilos, al no poder leer/escribir, liberen su memoria
	
	exit(EXIT_SUCCESS);
}