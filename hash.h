// hash.h
#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "lib/xxhash/xxhash.h"
#include <sys/types.h>

#define TAM_TABLA 100003  // tamaño primo (ajustable)
#define CLAVE_MAX 128
#define LINEA_MAX 2048
#define RESP_MAX 2048

// Nodo compacto: solo hash, offset y índice del siguiente (int32 para ahorrar)
typedef struct {
    uint64_t hash;
    off_t offset;
    int32_t siguiente; // -1 = null
} Nodo;

extern int32_t tabla[TAM_TABLA]; // cada entrada guarda índice en 'nodes' o -1
extern Nodo *nodes;              // pool contiguo de nodos
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

#endif // HASH_H
