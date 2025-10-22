#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lib/xxhash/xxhash.h"
#include <sys/types.h>

#define TAM_TABLA 1000
#define CLAVE_MAX 128
#define LINEA_MAX 2048
#define RESP_MAX 2048

typedef struct {
    uint64_t hash;
    off_t offset;
    int32_t siguiente;
} Nodo;

extern int32_t tabla[TAM_TABLA];
extern Nodo *nodes;
extern int32_t nodes_capacity;
extern int32_t nodes_count;

void init_tabla(void);
unsigned long long calcular_hash64(const char *clave);
int indice_de_hash_from_u64(uint64_t h);
void reservar_pool_nodos(size_t expected);
void insertar_indice(const char *clave, off_t offset);
void construir_indice(FILE *f);
char* buscar_por_clave(FILE *f, const char *clave, char *buffer_out);
void liberar_tabla(void); 

#endif