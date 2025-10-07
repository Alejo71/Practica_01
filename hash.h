/* hash.h - interfaz mínima para la práctica 1 */
#ifndef HASH_H
#define HASH_H

#include <stddef.h>

/* Calcula un índice para una clave (key, len) en la tabla hash */
int calcular_indice(const char* key, size_t len);

#endif /* HASH_H */
