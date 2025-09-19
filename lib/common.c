#include "common.h"

// Función para mostrar un mensaje de errno y hacer un exit con "ext_val"
void print_exit(char *err_msg, int ext_val) {
    perror(err_msg);
    exit(ext_val);
}

/* 
 * Función que separa una cadena, en tokens y los 
 * devuelve un arreglo NULL-terminado.
 * NOTA: Se debe llamar a free_args() para liberar el
 * arreglo y sus substrings.
 */
char **parse_command(const char *input) {
    char *copy;                   // Copia temporal de la string de entrada
    char *temp;                   // Token "separador"
    char *separadores = " \n\t\r"; // Delimitadores para strtok()
    size_t count = 0;             // Cantidad de elementos
    char **args;                  // Arreglo con los tokens


    // Copia de la cadena para tokenizar
    copy = strdup(input);
    if (!copy) return NULL;

    // Contar cuántos tokens hay
    temp = strtok(copy, separadores);
    while (temp) {
        count++;
        temp = strtok(NULL, separadores);
    }
    free(copy); // <- Se debe liberar las strings creadas con strdup()

    // Crear arreglo de punteros +1 para el NULL final
    args = malloc((count + 1) * sizeof(char *));
    if (!args) return NULL;

    // Tokenizar de nuevo y llenar el arreglo
    copy = strdup(input);
    if (!copy) {
        free(args); // Acá tenemos que liberar args en caso de error, no podemos retornar sin preocuparnos
        return NULL;
    }

    size_t i = 0;       // Índice
    args[count] = NULL; // El arreglo debe ser NULO-terminado
    temp = strtok(copy, separadores);
    while (temp) {
        args[i] = strdup(temp);
        if (!args[i]) { // En caso de que falle una sub-string, borramos todos y salimos
            free_args(args);
            free(copy);
            return NULL;
        }
        i++;
        temp = strtok(NULL, separadores);
    }

    // Liberamos la copia temporal y retornamos el arreglo nul terminado
    free(copy);
    return args;
}

// Función para liberar la memoria del arreglo
void free_args(char **args) {
    if (!args) return;
    for (size_t i = 0; args[i]; i++) {
        free(args[i]);
    }
    free(args);
}