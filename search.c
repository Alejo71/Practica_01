#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <time.h>       // ← necesario para clock_gettime
#include "shm_defs.h"
#include "hash.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s dataset.csv\n", argv[0]);
        return 1;
    }   

    const char *ruta_csv = argv[1];

    //Conectar memoria compartida
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        exit(1);
    }

    // Ajustar tamaño del segmento (una sola vez)
    ftruncate(fd, sizeof(shm_data));

    // Mapear el segmento en el espacio de direcciones
    shm_data *data = mmap(NULL, sizeof(shm_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    // Abrir semáforos existentes o crearlos
    sem_t *sem_req = sem_open(SEM_REQ, O_CREAT, 0666, 0);
    sem_t *sem_res = sem_open(SEM_RES, O_CREAT, 0666, 0);

    if (sem_req == SEM_FAILED || sem_res == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    //Cargar dataset
    FILE *f = fopen(ruta_csv, "r");
    if (!f) {
        perror("Error al abrir el dataset");
        exit(1);
    }

    construir_indice(f);
    printf("Buscardor listo. Indice construido.\n");

    while(1) {
        // Esperar a que la interfaz envíe una consulta
        sem_wait(sem_req);

        //Verificar si debe salir
        if (strcmp(data->query, "<<EXIT>>") == 0)
            break;  

        char temp[RESP_MAX];
        char *res = NULL;

        //Tiempo de busqueda
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);

        res = buscar_por_clave(f, data->query, temp); 

        clock_gettime(CLOCK_MONOTONIC, &end);
        double tiempo = (end.tv_sec - start.tv_sec) +
                        (end.tv_nsec - start.tv_nsec) / 1e9;
        
        printf("Tiempo de busqueda: %.6f segundos\n",tiempo);

        //Guardar resultado en memoria compartida
        if (res)
            strncpy(data->result, res, BUF_SZ - 1);
        else
            strcpy(data->result, "NA");

        //Notificar que la respuesta esta lista
        sem_post(sem_res);
    }

    liberar_tabla();
    fclose(f);
    munmap(data, sizeof(shm_data));
    close(fd);

    //Eliminar recursos al salir
    shm_unlink(SHM_NAME);
    sem_unlink(SEM_REQ);
    sem_unlink(SEM_RES);

    printf("Buscador finalizado correctamente.\n");
    return 0;
}