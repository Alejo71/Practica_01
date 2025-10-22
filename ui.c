#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include "shm_defs.h"

int main() {
    //Crear segmento de memoria compartida (si no existe)
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1){
        perror("shm_open");
        exit(1);
    }

    //Ajustar tamaño del segmento
    ftruncate(fd, sizeof(shm_data));

    //Mapear
    shm_data *data = mmap(NULL, sizeof(shm_data), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    //Crear o abrir semaforos
    sem_t *sem_req = sem_open(SEM_REQ, O_CREAT, 0666, 0);
    sem_t *sem_res = sem_open(SEM_RES, O_CREAT, 0666, 0);

    if (sem_req == SEM_FAILED || sem_res == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    while (1) {
        printf("\n--- MENU ---\n");
        printf("1) Buscar registro por título (segunda columna)\n");
        printf("2) Salir\n> ");

        char opcion[8];
        fgets(opcion, sizeof(opcion),stdin);
        int op = atoi(opcion);

        if (op == 2) {
            strcpy(data->query, "<<EXIT>>");
            sem_post(sem_req);
            break;
        }

        if (op == 1) {
            printf("Ingrese título (exacto o igual a como aparece en CSV): ");
            fgets(data->query, sizeof(data->query),stdin);
            data->query[strcspn(data->query, "\n")] = '\0';

            //Notificar al buscador que hay una consulta
            sem_post(sem_req);

            //Esperar al buscador
            sem_wait(sem_res);

            printf("Resultado: %s\n", data->result);
        }    
    }

munmap(data, sizeof(shm_data));
close(fd);
sem_close(sem_req);
sem_close(sem_res);

return 0;

}