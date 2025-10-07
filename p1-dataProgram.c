#include <stdio.h>
#include <string.h>
#include "hash.h"

/* Programa de prueba: calcula el índice de una clave de ejemplo */
int main(void)
{
	const char* clave = "ejemplo";
	int idx = calcular_indice(clave, strlen(clave));
	printf("Índice para '%s' = %d\n", clave, idx);
	return 0;
}

