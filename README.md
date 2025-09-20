# Local Server
 Este programa para entornos __unix__, es una implementación con sockets de un servidor TCP (con IPv4). El servidor recibirá, mediante el comando ```telnet```, comandos <u>Linux</u> y, los ejecutará (si es posible) retornando a la interfaz del cliente la respuesta de este comando.
 A medida que el servidor recibe peticiones, las retira de la cola y las maneja usando hilos (```<pthread.h>```).
 No está de más decir que el puerto usado y, la cantidad de peticiones "encolables" se hallan definidas en ```lib/common.h```.


<div style="background:rgb(220,220,220);padding:0.5rem;">

<sup>**Nota:** Este repositorio es parte de un conjunto de soluciones a problemas típicos presentes en la programación de 'bajo nivel' en C.</sup>

</div>

## Uso
- Una vez tenga este repositorio, verifique si ud. tiene el comando ```cmake``` y, si es así, que su versión sea al menos la referida en ```CMakeLists.txt```. Para verficar su versión puede ejecutar en consola:
```
cmake --version
```
- Para poder compilar, unicamente ejecute los siguientes comandos en orden (posicione su shell en su directorio local de este repositorio):
```
mkdir build
```
```
cd build
```
```
cmake ../
```
```
make
```
- Luego, debería ver dentro de su carpeta 'build' los siguientes archivos:
```
-rw-r--r-- 1 franco franco ? 12445 sep 18 20:54 CMakeCache.txt
drwxr-xr-x 5 franco franco ?  4096 sep 19 15:56 CMakeFiles
-rw-r--r-- 1 franco franco ?  1660 sep 18 20:54 cmake_install.cmake
-rw-r--r-- 1 franco franco ?   656 sep 18 18:33 compile_commands.json
-rwxr-xr-x 1 franco franco ? 17808 sep 19 15:56 local_server
-rw-r--r-- 1 franco franco ?  6177 sep 19 15:56 Makefile
```
- Para poder ejecutar el servidor unicamente ejecute ```./local_host``` (Sea libre de modificar cualquier archivo, si lo hace, para volver a compilar, únicamente ejecute en su carpeta 'build' el comando ```make```). Una vez ejecutado, debería ver algo similar a esto:
```
Preparando servidor...
Servidor activo... Esperando conexiones
        Backlog:5
        Puerto:3377
Para terminar pulse 'Ctrl+C'
```

- Significando que el servidor se encuentra activo y a la espera de nuevas conexiones. Ahora, para hacer peticiones, le recomendamos usar el siguiente comando en otra terminal:

```
telnet localhost -4 [PUERTO]
```

- Este, abrirá una interfaz de usuario (por defecto instalada en la mayoría de sistemas linux) que se conectará a su servidor en ejecución, dando un mensaje similar a:

```
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.

```

- Dejandonos un cursor para poder escribir y al, presionar enter, enviar el mensaje a nuestro servidor.

- Podrá ver, al ejecutar comandos desde la interfaz propuesta, que solo se enviará la respuesta de los comandos <u>Linux</u> bien ejecutados (es decir, sin errores) o un cierre de la interfaz si envía el mensaje "__chau__". Las excepciones y errores (además de TODOS los mensajes recibidos y clientes desconectados) serán solo visibles en la terminal del servidor.

---

### Seguridad
 Ahora, en estas últimas secciones vamos a hablar de temas totalmente serios y, posiblemente, necesarios al implementar este programa.
 Vamos por el mas importante a la hora de dar este servicio a internet: La __seguridad__. Cabe decir que, la misma premisa del programa es "peligrosa", es decir, podríamos ejecutar casi cualquier comando linux en nuestra PC remotamente solamente con ejecutar el programa (los comandos ```sudo``` solamente serán accesibles si se ejecuta el archivo como ```sudo ./local_server``` o, si se provee la contraseña cada vez que se envié un comando de este tipo) asi que <u>__se recomienda no usar este programa para conexiones con internet__</u> (a menos de que lo modifique con otro intermediario).
  Siguiendo el tema de seguridad, notará que no podrá acceder desde su navegador (u otro) al servidor y esto es debido a que toda la comunicación con el servidor es "plana", es decir, no se adopta ningún protocolo HTTP.

### Memoria
 Por último tenemos un tema que, al menos a mí me es más importante: La __memoria__. Y es que, si juega un poco con el comando ```valgrind``` en el servidor, verá ciertos bloques de memoria con "Possible Lost", en su mayoría, fueron identificados como el bloque de control que usa POSIX para la gestión de hilos (el hilo principal/servidor se desliga de los hilos usados para atender a cada cliente).
 Otro tipo de pérdida de memoria <u>no fue identificada</u> hasta la fecha del commit de este archivo... Por ende, si halla otra, le pido que me avise.

---

> **Advertencia:**
> Cuando desde el cliente se ejecutan comandos como "Ctrl+C" u otros, hace que el servidor no se comporte como es debido. Hasta el momento, no se halló una posible solución a este incoveniente.

---

<span>Franco N. Angeletti</span>
