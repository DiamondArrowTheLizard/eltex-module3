#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define PROJECT_ID 'S'
#define SEM_COUNT 1
#define BUFFER_SIZE 1024
#define SLEEP_TIME 2

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    char *filename = argv[1];
    key_t key = ftok(filename, PROJECT_ID);
    if (key == -1) {
        perror("ftok");
        return EXIT_FAILURE;
    }

    int semid = semget(key, SEM_COUNT, 0666);
    if (semid == -1) {
        perror("semget (run producer first)");
        return EXIT_FAILURE;
    }

    struct sembuf lock = {0, -1, 0};
    struct sembuf unlock = {0, 1, 0};

    char buffer[BUFFER_SIZE];

    while (1) {
        semop(semid, &lock, 1);

        FILE *f = fopen(filename, "r+");
        if (f) {
            if (fgets(buffer, BUFFER_SIZE, f)) {
                
                int min, max, val, first = 1;
                char *token = strtok(buffer, " ");
                while (token != NULL) {
                    val = atoi(token);
                    if (first) {
                        min = max = val;
                        first = 0;
                    } else {
                        if (val < min) min = val;
                        if (val > max) max = val;
                    }
                    token = strtok(NULL, " ");
                }
                
                if (!first) {
                    printf("[%d] File: %s | Min: %d | Max: %d\n", getpid(), filename, min, max);
                }

                freopen(filename, "w", f); 
            }
            fclose(f);
        }

        semop(semid, &unlock, 1);
        sleep(SLEEP_TIME);
    }

    return EXIT_SUCCESS;
}
