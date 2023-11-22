#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#include <signal.h>

#define SIZE 1280

int fd_shm; // Internal key to the shared memory
sem_t *empty_sem;
sem_t *full_sem;
sem_t *mutex_sem;
sem_t *producer_finished_sem; // Semaphore to signal producer finished
int value;
char *shm_ptr; // Pointer to shared memory

// Cleanup code
void clean_up() {
    // Cleanup code
    munmap(shm_ptr, SIZE);// Unmap shared memory
    close(fd_shm); // Close shared memory file descriptor
    sem_close(empty_sem); // Close empty semaphore
    sem_close(full_sem); // Close full semaphore
    sem_close(mutex_sem);// Close mutex semaphore
    sem_close(producer_finished_sem);// Close producer finished semaphore
}

//function to signal_handler
void signal_handler(int signum) {
    if (signum == SIGINT) {
        clean_up();
        exit(0);
    }
}

//function to check string size and input
void check_string_size(char *str) {
    if (strlen(str) > 128) {
        printf("ERR\n");
        clean_up();
        exit(1);
    }
}

int main() {

    int index_memory = 0; // The specific polynomial index in memory
    signal(SIGINT, signal_handler);
    // Open the semaphore for tracking empty slots
    empty_sem = sem_open("/empty_sem", O_CREAT, 0666, 10);
    if (empty_sem == SEM_FAILED) {
        perror("ERR\n");
        exit(1);
    }

    //Open the semaphore for tracking full place
    full_sem = sem_open("/full_sem", O_CREAT, 0666, 0);
    if (full_sem == SEM_FAILED) {
        perror("ERR\n");
        exit(1);
    }

    //Open the semaphore for the lock
    mutex_sem = sem_open("/mutex_sem", O_CREAT, 0666, 1);
    if (mutex_sem == SEM_FAILED) {
        perror("ERR\n");
        exit(1);
    }

    //get a pointer to the shared memory
    if ((fd_shm = shm_open("/shm", O_RDWR | O_CREAT, 0777)) == -1) {
        perror("ERR\n");
        exit(1);
    }

    //The shared memory allocation is 1280 in size which is actually 10 polynomials maximum
    if ((ftruncate(fd_shm, SIZE)) == -1) {
        perror("ERR\n");
        exit(1);
    }
    //combine the pointer we creat in shm_open to the memory we took
    if ((shm_ptr = mmap(NULL, SIZE, PROT_WRITE, MAP_SHARED, fd_shm, 0)) == MAP_FAILED) {
        perror("ERR\n");
        exit(1);
    }

    memset(shm_ptr, 0, SIZE);// Clear shared memory

    while (1) {

        char input_str[130];
        memset(input_str, 0, strlen(input_str));
        //printf("Enter the string:");
        fflush(stdout);
        fgets(input_str, 130, stdin);

        check_string_size(input_str);

        input_str[strcspn(input_str, "\n")] = 0; // Remove the trailing newline character

        if (index_memory == 10) {
            index_memory = 0;
        }
        if (sem_wait(empty_sem) != 0)perror("ERR\n"); //Decrease empty in one -- > empty - 1 // the number of free place
        if (sem_wait(mutex_sem) != 0)perror("ERR\n"); //Decrease mutex in one -- > mutex - 1
        //sem_getvalue(empty_sem, &value);

        //printf("The value of the semaphore after the wait is %d\n", value);
        strcpy(shm_ptr + (index_memory * 128), input_str);
        index_memory++;

        if (strcmp(input_str, "END") == 0) {
            sleep(1);
            sem_post(mutex_sem); //Increase mutex in one -- > mutex ++
            sem_post(full_sem); //Increase mutex in one -- > mutex ++
            clean_up();
            exit(0); // Terminate the program
        }

        sem_post(mutex_sem); //Increase mutex in one -- > mutex ++
        sem_post(full_sem); //Increase mutex in one -- > mutex ++
        sleep(1);
    }
}//Version 1.0 final-producer