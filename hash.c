// hash.c
#include "hash.h"

Nodo* tabla[TAM_TABLA];

void init_tabla() {
    for (int i = 0; i < TAM_TABLA; ++i) tabla[i] = NULL;
}

unsigned long long calcular_hash64(const char *clave) {
    // Usamos XXH64 (seed = 0)
    return (unsigned long long) XXH64(clave, strlen(clave), 0);
}

int indice_de_hash(const char *clave) {
    unsigned long long h = calcular_hash64(clave);
    return (int)(h % TAM_TABLA);
}

void insertar_indice(const char *clave, long offset) {
    int idx = indice_de_hash(clave);
    Nodo *n = (Nodo*) malloc(sizeof(Nodo));
    if (!n) { perror("malloc"); exit(1); }
    strncpy(n->clave, clave, CLAVE_MAX-1);
    n->clave[CLAVE_MAX-1] = '\0';
    n->offset = offset;
    n->siguiente = tabla[idx];
    tabla[idx] = n;
}

void construir_indice(FILE *f) {
    init_tabla();

    char linea[LINEA_MAX];
    long offset;

    // Saltar cabecera (suponemos una línea de cabecera)
    offset = ftell(f);
    if (!fgets(linea, sizeof(linea), f)) return; // archivo vacío
    offset = ftell(f);

    while (1) {
        long pos = offset; // posición donde empieza la próxima línea
        if (!fgets(linea, sizeof(linea), f)) break;

        // parsear primer campo (antes de la primera coma)
        char clave[CLAVE_MAX];
        // si la línea tiene menos de 1 char, continuar
        if (sscanf(linea, " %127[^,\n\r]", clave) >= 1) {
            insertar_indice(clave, pos);
        }
        offset = ftell(f);
    }
}

char* buscar_por_clave(FILE *f, const char *clave, char *buffer_out) {
    int idx = indice_de_hash(clave);
    Nodo *n = tabla[idx];
    while (n) {
        if (strcmp(n->clave, clave) == 0) {
            // leer registro desde offset
            if (fseek(f, n->offset, SEEK_SET) == 0) {
                if (fgets(buffer_out, RESP_MAX, f)) {
                    // quitar newline final
                    size_t L = strlen(buffer_out);
                    while (L > 0 && (buffer_out[L-1] == '\n' || buffer_out[L-1] == '\r')) {
                        buffer_out[L-1] = '\0';
                        --L;
                    }
                    return buffer_out;
                }
            }
            // si no pudo leer, devolver NULL
            return NULL;
        }
        n = n->siguiente;
    }
    return NULL;

    
}
void liberar_tabla() {
    for (int i = 0; i < TAM_TABLA; ++i) {
        Nodo *actual = tabla[i];
        while (actual) {
            Nodo *temp = actual;
            actual = actual->siguiente;
            free(temp);
        }
        tabla[i] = NULL;
    }
}

