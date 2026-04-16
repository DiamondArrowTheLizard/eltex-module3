#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>

#define SHM_NAME "/shm"
#define MAX_NUMBERS 10
#define MAX_NUMBER INT_MAX

struct shared_data 
{
    int count;
    int numbers[MAX_NUMBERS];
    int min;
    int max;
    int ready;
};

struct data_stat
{
    int set_max;
    int max;

    int set_min;
    int min;
};

void update_stat(struct data_stat* st, int min, int max, int set)
{
    if(st->min > min)
    {
        st->min = min;
        st->set_min = set;
    }
    if(st->max < max)
    {
        st->max = max;
        st->set_max = set;
    }
}

int stop = 0;

void sigint_handler(int sig) 
{
    sig = sig;
    ++stop;
}

int main(void) {
    signal(SIGINT, sigint_handler);
    struct data_stat stats = {
        .set_max = 0,
        .max = 0,

        .set_min = 0,
        .min = INT_MAX,
    };

    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) 
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }
    if (ftruncate(shm_fd, sizeof(struct shared_data)) == -1) 
    {
        perror("ftruncate");
        exit(EXIT_FAILURE);
    }
    struct shared_data* shared = mmap(NULL, sizeof(struct shared_data),
                                      PROT_READ | PROT_WRITE, MAP_SHARED,
                                      shm_fd, 0);
    if (shared == MAP_FAILED) 
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    srand(time(NULL));
    int processed = 0;

    while (stop == 0) 
    {
        int num_count = rand() % MAX_NUMBERS + 1;
        shared->count = num_count;
        for (int i = 0; i < num_count; ++i) 
        {
            shared->numbers[i] = rand() % MAX_NUMBER;
        }

        pid_t pid = fork();
        if (pid == -1) 
        {
            perror("fork");
            break;
        }
        if (pid == 0) 
        {
            signal(SIGINT, SIG_IGN);
            int min_val = shared->numbers[0];
            int max_val = shared->numbers[0];
            for (int i = 1; i < shared->count; ++i) 
            {
                if (shared->numbers[i] < min_val) min_val = shared->numbers[i];
                if (shared->numbers[i] > max_val) max_val = shared->numbers[i];
            }
            shared->min = min_val;
            shared->max = max_val;
            shared->ready = 1;
            exit(EXIT_SUCCESS);

        } else {

            int status;
            waitpid(pid, &status, 0);
            if (shared->ready) 
            {
                update_stat(&stats, shared->min, shared->max, processed + 1);
                fprintf(stdout, "Set %d: Min = %d; Max = %d\n", processed + 1, shared->min, shared->max);
                shared->ready = 0;
            }
            ++processed;
        }
    }

    fprintf(stdout, "\nSets parsed: %d\n", processed);

    fprintf(stdout, "Min number: %d (At set %d)\nMax number: %d (At set %d)\n",
            stats.min, stats.set_min, stats.max, stats.set_max);

    munmap(shared, sizeof(struct shared_data));
    close(shm_fd);
    shm_unlink(SHM_NAME);
    return 0;
}
