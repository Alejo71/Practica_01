#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "shm_defs.h"
#include "hash.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s dataset.csv\n", argv[0]);
        return 1;
    }

    const char *ruta_csv = argv[1];

    // Conectar con la memoria compartida
    int fd = shm_open(SHM_NAME, O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); exit(1); }

    shm_data *data = mmap(NULL, sizeof(shm_data),
                          PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    // Abrir semáforos existentes
    sem_t *sem_req = sem_open(SEM_REQ, 0);
    sem_t *sem_res = sem_open(SEM_RES, 0);

    if (sem_req == SEM_FAILED || sem_res == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    // Cargar dataset e índice hash
    FILE *f = fopen(ruta_csv, "r");
    if (!f) { perror("abrir dataset"); exit(1); }

    construir_indice(f);
    printf("Buscador listo. Índice construido.\n");

    while (1) {
        // Esperar a que la interfaz envíe una consulta
        sem_wait(sem_req);

        // Verificar si debe salir
        if (strcmp(data->query, "<<EXIT>>") == 0) break;

        char temp[RESP_MAX];
        char *res = buscar_por_clave(f, data->query, temp);

        if (res)
            strncpy(data->result, res, BUF_SZ);
        else
            strcpy(data->result, "NA");

        // Notificar que la respuesta está lista
        sem_post(sem_res);
    }

    liberar_tabla();
    fclose(f);
    munmap(data, sizeof(shm_data));
    close(fd);

    // Eliminar recursos al salir
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_REQ);
    sem_unlink(SEM_RES);
    return 0;
}
