// hash.h
#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lib/xxhash/xxhash.h"
 // asegúrate de tener lib/xxhash/xxhash.h y ajustar include path en el Makefile
#include <sys/types.h>

#define TAM_TABLA 100003  // tamaño primo (ajustable)
#define CLAVE_MAX 128
#define LINEA_MAX 2048
#define RESP_MAX 2048

typedef struct Nodo {
    char clave[CLAVE_MAX];
    off_t offset;            // byte offset en el archivo CSV
    struct Nodo* siguiente;
} Nodo;

extern Nodo* tabla[TAM_TABLA];

void init_tabla();
unsigned long long calcular_hash64(const char *clave);
int indice_de_hash(const char *clave);
void insertar_indice(const char *clave, off_t offset);
void construir_indice(FILE *f);
char* buscar_por_clave(FILE *f, const char *clave, char *buffer_out);
void liberar_tabla();
#endif // HASH_H
