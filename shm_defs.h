#ifndef SHM_DEFS_H
#define SHM_DEFS_H

#include <semaphore.h>

#define SHM_NAME "/shm_practica"   // nombre del segmento de memoria compartida
#define SEM_REQ "/sem_request"     // semáforo para señal de "hay consulta"
#define SEM_RES "/sem_response"    // semáforo para señal de "hay respuesta"
#define BUF_SZ 2048

typedef struct {
    char query[BUF_SZ];   // consulta enviada por el usuario
    char result[BUF_SZ];  // respuesta generada por el buscador
} shm_data;

#endif
