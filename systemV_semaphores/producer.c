#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

#define PROJECT_ID 'S'
#define SEM_COUNT 1
#define MAX_NUMBERS 10
#define MAX_VAL 100
#define SLEEP_TIME 1

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

    int semid = semget(key, SEM_COUNT, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        return EXIT_FAILURE;
    }

    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);

    struct sembuf lock = {0, -1, 0};
    struct sembuf unlock = {0, 1, 0};

    srand(time(NULL) ^ getpid());

    while (1) {
        semop(semid, &lock, 1);

        FILE *f = fopen(filename, "a");
        if (f) {
            int count = rand() % MAX_NUMBERS + 1;
            for (int i = 0; i < count; i++) {
                fprintf(f, "%d ", rand() % MAX_VAL);
            }
            fprintf(f, "\n");
            fclose(f);
        }

        semop(semid, &unlock, 1);
        sleep(SLEEP_TIME);
    }

    return EXIT_SUCCESS;
}
