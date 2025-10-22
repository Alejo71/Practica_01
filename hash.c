#include "hash.h"
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

int32_t tabla[TAM_TABLA];
Nodo *nodes = NULL;
int32_t nodes_capacity = 0;
int32_t nodes_count = 0;

void init_tabla(void) {
    for (int i = 0; i < TAM_TABLA; ++i)
        tabla[i] = -1;
        nodes_count = 0;
}

static size_t max_nodes_for_limit(size_t limit_bytes) {
    size_t per = sizeof(Nodo);
    if (per == 0) 
        return 0;
    return limit_bytes / per;
}

void reservar_pool_nodos(size_t expected) {
    const size_t MEM_LIM_MB = 9;
    size_t max_nodes = max_nodes_for_limit(MEM_LIM_MB * 1024 * 1024);
    size_t want = expected ? expected : 1024; 
    if (want > max_nodes)
        want = max_nodes;
    if (want < 16)
        want = 16;
    nodes = (Nodo*) calloc(want,sizeof(Nodo));
    if (!nodes){
        perror("calloc nodes");
        exit(-1);
    }
    nodes_capacity = (int32_t) want;
    nodes_count = 0;
}

unsigned long long calcular_hash64(const char *clave) {
    return (unsigned long long) XXH64(clave, strlen(clave), 0);
}

int indice_de_hash_from_u64(uint64_t h) {
    return (int) (h % TAM_TABLA);
}

static void to_lower_str(char *s) {
    for (; *s; ++s) *s = (char) tolower((unsigned char)*s);
}

static int extract_nth_field(const char* s, int n, char* out, size_t max) {
    if(!s || !out || max == 0 || n <= 0)
        return 0;

    const char *p = s;
    int field = 1;

    while (*p && field <= n) {
        size_t i = 0;

        while(*p == ' ' || *p == '\t')
            ++p;

        if(*p == '"') {
            //campo entre comillas
            ++p;

        while(*p) {
            if (*p == '"' && *(p+1) == '"'){
                if(i + 1 < max) 
                out[i++] = '"';
                p += 2;
            } else if (*p == '"') {
                ++p;
                break;
            } else {
                if (i + 1 < max)
                    out[i++] = *p;
                    ++p;
                }
            
        }
        while (*p && *p != ',');
    } else {
        while (*p && *p != ',' && *p != '\n' && *p != '\r') {
            if(i + 1 < max) out[i++] = *p;
            ++p;
        }
        while (i > 0 && (out[i-1] == ' ' || out[i-1] == '\t'))
            --i;
    }

    out[i] = '\0';

    if (field == n) {
        return 1;
    }

    if (*p == ',')
        ++p;
        ++field;
    }

    return 0;
}

void insertar_indice(const char *clave_orig, off_t offset){
    char clave[CLAVE_MAX];
    strncpy(clave, clave_orig, CLAVE_MAX-1);
    clave[CLAVE_MAX-1] = '\0';
    to_lower_str(clave);

    uint64_t h = calcular_hash64(clave);
    int idx = indice_de_hash_from_u64(h);

    if(!nodes || nodes_count >= nodes_capacity) {
        return;
    }

    int32_t id = nodes_count++;
    nodes[id].hash = h;
    nodes[id].offset = offset;
    nodes[id].siguiente = tabla[idx];
    tabla[idx] = id; 
}

void construir_indice(FILE *f) {
    init_tabla();

    //Se estima el numero de lineas
    struct stat st;
    size_t expected =0;
    if (fstat(fileno(f),&st) == 0 && st.st_size > 0) {
        size_t avg_line = 120;
        expected = (size_t)(st.st_size / avg_line);
    }
    reservar_pool_nodos(expected +16);

    char *line = NULL;
    size_t len = 0;
    ssize_t nread;

    //Se salta la cabecera
    if ((nread = getline(&line, &len, f)) == -1) {
        free(line);
        return;
    }

    const int COL_A_INDEXAR = 2;

    while (1) {
        off_t pos = ftello(f);
        if (pos == -1)
            break;
        nread = getline(&line, &len, f);
        if (nread == -1)
            break;

        char clave[CLAVE_MAX];
        if (extract_nth_field(line, COL_A_INDEXAR, clave, sizeof(clave))) {
            if (strlen(clave) > 0) {
                insertar_indice(clave, pos);
            }
        }
    }

    free(line);
}


//Buscar por 'clave' (user input). Compara hashes y verifica leyendo el registro.

char* buscar_por_clave(FILE *f, const char *clave_orig, char *buffer_out){
    char clave_norm[CLAVE_MAX];
    strncpy(clave_norm, clave_orig, CLAVE_MAX-1);
    clave_norm[CLAVE_MAX-1] = '\0';
    to_lower_str(clave_norm);
    
    uint64_t h = calcular_hash64(clave_norm);
    int idx = indice_de_hash_from_u64(h);
    int32_t cur = tabla[idx];

    while (cur != -1) {
        if (nodes[cur].hash == h) {
            if (fseeko(f, nodes[cur].offset, SEEK_SET) == 0){
                if (fgets(buffer_out, RESP_MAX, f)) {
                    size_t L = strlen(buffer_out);
                    while (L > 0 && (buffer_out[L-1] == '\n' || buffer_out[L -1] == '\r')) {
                        buffer_out[--L] = '\0';
                    }
                    
                    // extraer la clave real y comparar normalizada
                    char clave_leida[CLAVE_MAX];
                    if (extract_nth_field(buffer_out,2,clave_leida,sizeof(clave_leida))) {
                        to_lower_str(clave_leida);
                        if (strcmp(clave_leida, clave_norm) == 0) {
                            return buffer_out;
                        }
                    }

                }
            }
        }
        cur = nodes[cur].siguiente;
    }
    return NULL;
}

void liberar_tabla(void) {
    if(nodes) {free(nodes); nodes =NULL;}
    nodes_capacity = nodes_count = 0;
    for (int i = 0; i < TAM_TABLA; ++i) tabla[i] = -1;
}