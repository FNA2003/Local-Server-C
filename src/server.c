#include "common.h"


volatile sig_atomic_t running = 1; // Variable global para 'frenar' el bucle de main_loop() -> handler_senial()
								   // se usa el entero definido en <signal.h> para garantizar la atomicidad de este
volatile int server_sock = -1; 	   // File descriptor del socket del servidor

static int init_socket_server(void);
static void main_loop();
static void* hilo_cliente(void*);
static void handler_senial(int);


int main(void) {
	/* Configuramos la salida del servidor */
	signal(SIGINT, handler_senial);
	signal(SIGQUIT, handler_senial);
	

	printf("Preparando servidor...\n");
	// Inicializamos y preparamos el socket, si hay error la función lo maneja por si misma
	server_sock = init_socket_server();

	printf("Servidor activo... Esperando conexiones\n"
		   "\tBacklog:%d\n\tPuerto:%d\n"
		   "Para terminar pulse 'Ctrl+C'\n",
		   MAX_BACKLOG, SERV_PORT);

	// Bucle principal
	main_loop();

	
	// Solamente se llega a este punto cuando se interrumpe el bucle o hay
	// un error en la configuración de los sockets pues, la
	// salida se maneja en handler_senial()
	close(server_sock);
	return 0;
}


/* 
 * Función que retorna un entero correspondiente a un fd de un socket 
 * asignado a una comunicación TCP con IPv4, en caso de error, cerrará el programa
 * con un mensaje de error. Nota; el socket será configurado como socket pasivo
 * es decir, para la lectura desde el lado de un servidor.
 */
static int init_socket_server() {
	int sock_fd; 				  // File descriptor del socket
	struct sockaddr_in sock_addr; // Información para el kernel sobre el socket
	int opt = 1;

	/* Si podemos, creamos un file descriptor para el socket IPv4, sino, cerramos */
	if ( (sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
		print_exit("Error al crear socket", EXIT_FAILURE);

	/* Configuramos la API del socket para poder reiniciarlo más rápido */
	if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(sock_fd);
		print_exit("Error en setsockopt", EXIT_FAILURE);
	}

	/* Completamos la información del socket para pasarlo al kernel */
	memset(&sock_addr, 0, sizeof(sock_addr));
	sock_addr.sin_family = AF_INET;
	sock_addr.sin_port = htons(SERV_PORT);  // Variable global, vea common.h
	sock_addr.sin_addr.s_addr = INADDR_ANY; // Escuchar en cualquier IP local

	/* Le asignamos al file descriptor la información configurada anteriormente, o retornamos por error */
	if (bind(sock_fd, (struct sockaddr*)&sock_addr, sizeof(sock_addr)) < 0) {
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

static void main_loop() {
	while(running) {
		// Creamos una dirección para cada socket de cada cliente
		struct sockaddr_in *client_addr = malloc(sizeof(struct sockaddr_in));
		if (!client_addr) { // Si hay un error en la asignación del socket, no podemos interrumpir el servicio, tratamos nuevamente
			perror("malloc client_addr");
			continue;
		}

		socklen_t addr_len = sizeof(struct sockaddr_in);
		int cliente_sock = accept(server_sock, (struct sockaddr*)client_addr,&addr_len); // Esperamos a recibir un cliente. Nota: accept() es bloqueante hasta recibir una solicitud o cierre del servidor

		/* Hubo un error en accept */
		if (cliente_sock < 0) {
			free(client_addr);
			if (errno == EINTR) continue; // Se recibe la señal de handler_senial()
			
			// Si hubo un error en accept() por la señal, se estima error en el socket del servidor para recibir datos
			perror("Error en accept()");
			break;
		}

		struct datos_cliente *data = malloc(sizeof(struct datos_cliente));
		if (!data) {
			// La petición se perderá
			perror("malloc de datos_cliente");
			close(cliente_sock);
			free(client_addr);
			continue;
		}

		data->socket_addr = client_addr;
		data->socket_fd = cliente_sock;

		/* Creamos un hilo, si podemos, y luego lo "desligamos" para que se libere la memoria correspondiente a este al finalizar su función (sin que tengamos que intervenir) */
		pthread_t hilo;
		if (pthread_create(&hilo, NULL, hilo_cliente, data) != 0) {
			perror("pthread_create");
			close(cliente_sock);
			free(client_addr);
			free(data);
			continue;
		}
		pthread_detach(hilo);
	}
}

/*
 * Función para recibir un mensaje (comando linux)
 * por cada conexión que tengamos a nuestro servidor y,
 * enviarle al cliente el resultado de su mensaje. 
 */
static void* hilo_cliente(void *args) {
	struct datos_cliente *data = args; // Datos del cliente; file descriptor de su socket y datos de su dirección
	char buffer[BUFF_SIZE + 1]; 	   // Buffer de comunicación de entrada y salida con el cliente

	// Comunicación indefinida
	while(1) {
		ssize_t n = recv(data->socket_fd, buffer, BUFF_SIZE, 0);
		if (n <= 0) break; // error ó desconexión

		buffer[n] = '\0';
		printf("Se recibió el mensaje: %s", buffer);		
		
		// Revisamos si se envió la cadena de fin de comunicación
		if (strncmp(buffer, "chau", 4) == 0) break;
		
		// Pasamos la string recibida a una lista nulo terminada
		char **argumentos = parse_command(buffer);
		if (!argumentos) continue; // Si no se pudo parsear la cadena, perdemos este segmento dado por buffer

		// Pipe para la comunicación padre (hilo handler del cliente) - hijo (comando linux recibido)
		int p[2];
		if (pipe(p) < 0) {
			// Si falla la creación del pipe perdemos el comando a ejecutar
			perror("pipe");
			free_args(argumentos);
			continue;
		}

		pid_t pid = fork();
		if (pid == 0) { // Hijo
			close(p[0]);               // El proceso hijo no lee del pipe
			dup2(p[1], STDOUT_FILENO); // Redirigimos la salida estandar al pipe
			close(p[1]);               // Una vez redirigida la salida al pipe, podemos cerrar su referencia
			// Ejecución desde el hijo
			execvp(argumentos[0], argumentos);
			// Si se sigue ejecutando, hubo un error al cambiar la imágen
			perror("execvp");
			exit(EXIT_FAILURE);
		} else if (pid > 0){  // Padre
			close(p[1]);

			while((n = read(p[0], buffer, BUFF_SIZE)) > 0) {
				send(data->socket_fd, buffer, n, 0); // Enviamos el resultado del comando
				// Habría que hacer un chequeo de send (?
			}
			// Una vez finalizada la lectura, cerramos el pipe y esperamos al hijo
			close(p[0]);
			waitpid(pid, NULL, 0);
		} else {
			perror("fork");
		}

		// Liberamos la memoria usada por la lista nulo-terminada
		free_args(argumentos);
	}


	printf("Cliente desconectado\n");
	// Liberamos la memoria dinámica y cerramos el file descriptor del cliente
	close(data->socket_fd);
	free(data->socket_addr);
	free(data);

	// Salida del hilo
	pthread_exit(NULL);
}

static void handler_senial(int sig) {
	(void)sig;

	printf("\nCerando el servidor...\n");

	// 'Frenamos' el bucle principal
	running = 0;
	if (server_sock != -1) {
		// Esto "cerrará" accept() y dará error para recv()
		close(server_sock);
		server_sock = -1;
	}
	
	// Ahora, se "deja" que cierren todos los hilos y que no se pueda seguir recibiendo comunicación
}