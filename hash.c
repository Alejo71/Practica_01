// ...existing code...
// hash.c
#include "hash.h"
#include <errno.h>
#include <ctype.h>

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

void insertar_indice(const char *clave, off_t offset) {
    int idx = indice_de_hash(clave);
    Nodo *n = (Nodo*) malloc(sizeof(Nodo));
    if (!n) { perror("malloc"); exit(1); }
    strncpy(n->clave, clave, CLAVE_MAX-1);
    n->clave[CLAVE_MAX-1] = '\0';
    n->offset = offset;
    n->siguiente = tabla[idx];
    tabla[idx] = n;
}

/* Extrae el primer campo CSV de 's' (maneja comillas y trim). Devuelve 1 si extrajo algo. */
static int extract_first_field(const char* s, char* out, size_t max) {
    size_t i = 0;
    if (!s || !out || max == 0) return 0;
    // Saltar espacios iniciales
    while (*s && (*s == ' ' || *s == '\t')) ++s;
    if (*s == '"') {
        ++s; // dentro de comillas
        while (*s && *s != '"' && i + 1 < max) { out[i++] = *s++; }
        out[i] = '\0';
        return (i > 0) ? 1 : 0;
    } else {
        while (*s && *s != ',' && *s != '\n' && *s != '\r' && i + 1 < max) { out[i++] = *s++; }
        // trim trailing spaces
        while (i > 0 && (out[i-1] == ' ' || out[i-1] == '\t')) --i;
        out[i] = '\0';
        return (i > 0) ? 1 : 0;
    }
}

void construir_indice(FILE *f) {
    init_tabla();

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    // Saltar cabecera (suponemos una línea de cabecera)
    if ((nread = getline(&line, &len, f)) == -1) {
        free(line);
        return; // archivo vacío o error
    }

    while (1) {
        off_t pos = ftello(f);
        if (pos == -1) break;
        nread = getline(&line, &len, f);
        if (nread == -1) break;

        char clave[CLAVE_MAX];
        if (extract_first_field(line, clave, sizeof(clave))) {
            insertar_indice(clave, pos);
        }
    }

    free(line);
}

char* buscar_por_clave(FILE *f, const char *clave, char *buffer_out) {
    int idx = indice_de_hash(clave);
    Nodo *n = tabla[idx];
    while (n) {
        if (strcmp(n->clave, clave) == 0) {
            // leer registro desde offset
            if (fseeko(f, n->offset, SEEK_SET) == 0) {
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