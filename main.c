// main.c
#define _GNU_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "hash.h"

#define PIPE_READ 0
#define PIPE_WRITE 1
#define BUF_SZ 2048

// Protocolo simple:
// - Padre escribe consultas fijas de tamaño BUF_SZ al pipe_request.
// - Hijo responde escribiendo respuesta en pipe_response con tamaño BUF_SZ.
// - Mensaje especial "<EXIT>" indica terminar.

void proceso_padre_ui(int write_fd, int read_fd) {
    char opcion[8];
    char clave[CLAVE_MAX];
    char buf[BUF_SZ];

    while (1) {
        printf("\n--- MENU ---\n");
        printf("1) Buscar registro por clave (primer campo)\n");
        printf("2) Salir\n");
        printf("Opcion: ");
        if (!fgets(opcion, sizeof(opcion), stdin)) break;
        int op = atoi(opcion);
        if (op == 2) {
            // enviar orden de salida
            memset(buf, 0, sizeof(buf));
            strncpy(buf, "<<EXIT>>", sizeof(buf)-1);
            if (write(write_fd, buf, BUF_SZ) != BUF_SZ) perror("write to child");
            // leer ack opcional
            if (read(read_fd, buf, BUF_SZ) > 0) {
                // ignorar
            }
            break;
        } else if (op == 1) {
            printf("Ingrese la clave (primer campo, sin comas): ");
            if (!fgets(clave, sizeof(clave), stdin)) break;
            // limpiar newline
            size_t L = strlen(clave);
            if (L && (clave[L-1] == '\n' || clave[L-1] == '\r')) clave[L-1] = '\0';
            if (strlen(clave) == 0) {
                printf("Clave vacia. Cancelando.\n");
                continue;
            }

            memset(buf, 0, sizeof(buf));
            strncpy(buf, clave, sizeof(buf)-1);
            // enviar consulta
            if (write(write_fd, buf, BUF_SZ) != BUF_SZ) perror("write to child");

            // leer respuesta (bloqueante) desde hijo
            memset(buf, 0, sizeof(buf));
            ssize_t r = read(read_fd, buf, BUF_SZ);
            if (r <= 0) {
                perror("read from child");
                break;
            }
            if (strcmp(buf, "NA") == 0) {
                printf("Resultado: NA (no encontrado)\n");
            } else {
                printf("Registro encontrado: %s\n", buf);
            }
        } else {
            printf("Opcion invalida\n");
        }
    }
}

void proceso_hijo_busqueda(int read_fd, int write_fd, const char *ruta_csv) {
    // Abrir archivo y construir índice
    FILE *f = fopen(ruta_csv, "r");
    if (!f) {
        perror("Hijo: abrir dataset");
        // enviar error y salir
        char err[BUF_SZ];
        snprintf(err, sizeof(err), "ERROR: no se puede abrir %s", ruta_csv);
        write(write_fd, err, BUF_SZ);
        exit(1);
    }

    construir_indice(f);
    // enviar ack listo (opcional)
    char ready[BUF_SZ];
    memset(ready, 0, sizeof(ready));
    strncpy(ready, "READY", sizeof(ready)-1);
    write(write_fd, ready, BUF_SZ);

    char req[BUF_SZ];
    char resp[BUF_SZ];

    while (1) {
        memset(req, 0, sizeof(req));
        ssize_t r = read(read_fd, req, BUF_SZ);
        if (r <= 0) {
            // pipe cerrado?
            break;
        }
        // si mensaje de salida
        if (strcmp(req, "<<EXIT>>") == 0) {
            // enviar ack y terminar
            memset(resp, 0, sizeof(resp));
            strncpy(resp, "BYE", sizeof(resp)-1);
            write(write_fd, resp, BUF_SZ);
            break;
        }

        // procesar búsqueda
        memset(resp, 0, sizeof(resp));
        char temp[RESP_MAX];
        if (buscar_por_clave(f, req, temp)) {
            strncpy(resp, temp, BUF_SZ-1);
        } else {
            strncpy(resp, "NA", BUF_SZ-1);
        }
        // responder
        if (write(write_fd, resp, BUF_SZ) != BUF_SZ) {
            perror("hijo write");
            break;
        }
    }
    
    liberar_tabla();
    fclose(f);
    
    
    _exit(0);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s dataset.csv\n", argv[0]);
        return 1;
    }
    const char *ruta_csv = argv[1];

    // pipes: request_pipe (padre->hijo), response_pipe (hijo->padre)
    int pipe_req[2];
    int pipe_res[2];
    if (pipe(pipe_req) == -1) { perror("pipe"); return 1; }
    if (pipe(pipe_res) == -1) { perror("pipe"); return 1; }

    pid_t pid = fork();
    if (pid < 0) { perror("fork"); return 1; }

    if (pid == 0) {
        // hijo: cerramos extremos no usados
        close(pipe_req[PIPE_WRITE]); // hijo no escribe en request
        close(pipe_res[PIPE_READ]);  // hijo no lee response

        // hijo lee de pipe_req[0], escribe a pipe_res[1]
        proceso_hijo_busqueda(pipe_req[PIPE_READ], pipe_res[PIPE_WRITE], ruta_csv);

        // cerrar
        close(pipe_req[PIPE_READ]);
        close(pipe_res[PIPE_WRITE]);
        _exit(0);
    } else {
        // padre: cerramos extremos no usados
        close(pipe_req[PIPE_READ]); // padre no lee request
        close(pipe_res[PIPE_WRITE]); // padre no escribe response

        // leer ack 'READY' desde hijo (opcional)
        char buff[BUF_SZ];
        memset(buff, 0, sizeof(buff));
        if (read(pipe_res[PIPE_READ], buff, BUF_SZ) > 0) {
            if (strncmp(buff, "READY", 5) == 0) {
                printf("Indice construido en el proceso buscador. Listo.\n");
            } else {
                // si el hijo envió error, mostrar y salir
                if (strncmp(buff, "ERROR", 5) == 0) {
                    fprintf(stderr, "Error hijo: %s\n", buff);
                    // cerrar pipes y esperar hijo
                    close(pipe_req[PIPE_WRITE]);
                    close(pipe_res[PIPE_READ]);
                    wait(NULL);
                    return 1;
                }
            }
        }

        proceso_padre_ui(pipe_req[PIPE_WRITE], pipe_res[PIPE_READ]);

        // cerrar pipes
        close(pipe_req[PIPE_WRITE]);
        close(pipe_res[PIPE_READ]);

        // esperar hijo
        wait(NULL);
    }

    return 0;
}
