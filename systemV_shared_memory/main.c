#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <time.h>
#include <limits.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define MAX_NUMBERS 100
#define MAX_VAL 1000
#define SEM_PARENT 0
#define SEM_CHILD 1

struct shared_data {
    int numbers[MAX_NUMBERS];
    int count;
    int min;
    int max;
};

static volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
    (void)sig;
    keep_running = 0;
}

int main(void) {
    int shmid, semid;
    struct shared_data *data;
    pid_t pid;
    int sets_processed = 0;

    shmid = shmget(SHM_KEY, sizeof(struct shared_data), IPC_CREAT | 0666);
    data = (struct shared_data *)shmat(shmid, NULL, 0);
    semid = semget(SEM_KEY, 2, IPC_CREAT | 0666);

    semctl(semid, SEM_PARENT, SETVAL, 0);
    semctl(semid, SEM_CHILD, SETVAL, 0);

    struct sembuf parent_wait = {SEM_PARENT, -1, 0};
    struct sembuf parent_signal = {SEM_CHILD, 1, 0};
    struct sembuf child_wait = {SEM_CHILD, -1, 0};
    struct sembuf child_signal = {SEM_PARENT, 1, 0};

    signal(SIGINT, handle_sigint);

    pid = fork();

    if (pid == 0) {
        while (1) {
            semop(semid, &child_wait, 1);

            int current_min = INT_MAX;
            int current_max = INT_MIN;

            for (int i = 0; i < data->count; i++) {
                if (data->numbers[i] < current_min) current_min = data->numbers[i];
                if (data->numbers[i] > current_max) current_max = data->numbers[i];
            }

            data->min = current_min;
            data->max = current_max;

            semop(semid, &child_signal, 1);
        }
    } else {
        srand(time(NULL));

        while (keep_running) {
            data->count = (rand() % MAX_NUMBERS) + 1;
            for (int i = 0; i < data->count; i++) {
                data->numbers[i] = rand() % MAX_VAL;
            }

            semop(semid, &parent_signal, 1);
            
            if (semop(semid, &parent_wait, 1) == -1) break;

            printf("Set %d: Count=%d, Min=%d, Max=%d\n", 
                    ++sets_processed, data->count, data->min, data->max);
            
            sleep(1);
        }

        kill(pid, SIGKILL);
        wait(NULL);

        printf("\nTotal sets processed: %d\n", sets_processed);

        shmdt(data);
        shmctl(shmid, IPC_RMID, NULL);
        semctl(semid, 0, IPC_RMID);
    }

    return 0;
}
