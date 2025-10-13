/*
 * p1-dataProgram.c
 * 
 * Programa principal de la Práctica 1 - Sistemas Operativos
 * Autor: Alejandro Ramírez
 * Universidad Nacional de Colombia
 *
 * Este archivo explica la estructura del proyecto:
 * - "ui.c" implementa la interfaz de usuario.
 * - "buscador.c" implementa el proceso de búsqueda con hash.
 * 
 * Ambos procesos se comunican mediante memoria compartida y semáforos POSIX,
 * cumpliendo el requisito de procesos no emparentados.
 *
 * Para ejecutar:
 *   1. Compilar con: make
 *   2. En una terminal: ./buscador dataset.csv
 *   3. En otra terminal: ./ui
 */

#include <stdio.h>

int main(void) {
    printf("Práctica 1 - Comunicación entre procesos no emparentados\n");
    printf("Ejecute los siguientes pasos:\n");
    printf("1. En una terminal: ./buscador dataset.csv\n");
    printf("2. En otra terminal: ./ui\n");
    printf("\nAmbos programas se comunicarán mediante memoria compartida.\n");
    return 0;
}
