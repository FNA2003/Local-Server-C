#include "common.h"

// Función para mostrar un mensaje de errno y hacer un exit para finalizar el programa
void print_exit(char *errMsg, int extVal) {
    perror(errMsg);
    exit(extVal);
}

// Función que separa una cadena en tokens y devuelve un arreglo NULL-terminado
char **parse_command(const char *input) {
    // Copia de la cadena para tokenizar
    char *copy = strdup(input);
    if (!copy) return NULL;

    // Contar cuántos tokens hay
    size_t count = 0;
    char *temp = strtok(copy, " \n\t\r");
    while (temp) {
        count++;
        temp = strtok(NULL, " \n\t\r");
    }
    free(copy);

    // Crear arreglo de punteros +1 para el NULL final
    char **args = malloc((count + 1) * sizeof(char *));
    if (!args) return NULL;

    // Tokenizar de nuevo y llenar el arreglo
    copy = strdup(input);
    if (!copy) {
        free(args);
        return NULL;
    }

    size_t i = 0;
    temp = strtok(copy, " \n\t\r");
    while (temp) {
        args[i++] = strdup(temp);
        temp = strtok(NULL, " \n\t\r");
    }
    args[i] = NULL; // Terminar con NULL

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