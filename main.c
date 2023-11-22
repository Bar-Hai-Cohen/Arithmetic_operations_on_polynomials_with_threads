#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#define SIZE 1280 // Size of shared memory segment

int fd_shm; // Internal key to the shared memory
sem_t *empty_sem;
sem_t *full_sem;
sem_t *mutex_sem;
char *shm_ptr; // Pointer to shared memory

// Function to perform clean-up tasks and free memory
void clean_up() {
    // Cleanup code
    munmap(shm_ptr, SIZE);
    close(fd_shm);
    shm_unlink("/shm");
    sem_close(empty_sem);
    sem_close(full_sem);
    sem_close(mutex_sem);
    sem_unlink("/empty_sem");
    sem_unlink("/full_sem");
    sem_unlink("/mutex_sem");
}

// Signal handler function for SIGINT signal (Ctrl+C)
void signal_handler(int signum) {
    if (signum == SIGINT) {
        clean_up();
        exit(0);
    }
}

// Structure to represent a polynomial
typedef struct {
    int *polynomial; // Array to store the polynomial coefficients
    int degree;      // Degree of the polynomial
} polynomial;

// Structure to hold data for each thread
typedef struct {
    polynomial *poly1;   // Pointer to polynomial 1
    polynomial *poly2;   // Pointer to polynomial 2
    int coeffIndex;      // Index of the coefficient to calculate
    int operation;       // Flag indicating the type of operation
    int *result;         // Array to store the calculated result
    pthread_mutex_t *mutex; // Mutex for thread synchronization
} ThreadData;

// Function to print the resulting polynomial
void print_result(int *result, int max_degree) {
    // Print the result
    // Print the result
    int num_of_zero=0;
    for (int i = 0; i <= max_degree; ++i) {
        if (result[i] == 0) {
            num_of_zero++;
        }
        if(num_of_zero==max_degree){
            printf("%d",0);
            printf("\n");
            return;
        }
    }
    int flag_result_before=0;
    int flag_not_first_print=0;
    for (int i = 0; i <= max_degree; i++) {
        if (result[i] == 0) {
            flag_result_before=1;
            continue;
        }
        if (i == 0) {
            printf("%dx^%d", result[i], max_degree - i);
            flag_not_first_print=1;
            continue;
        } else {//Checks what the sign of the coefficient is before the coefficient

            if (result[i] < 0) {
                if(flag_result_before==1&&flag_not_first_print==0){
                    flag_result_before=0;
                    printf("%dx^%d", result[i], max_degree - i);
                    flag_not_first_print=1;
                    continue;
                }
                else {
                    printf(" - ");
                    result[i] = result[i] * -1;
                }
            } else {
                if(flag_result_before==1&&flag_not_first_print==0){
                    flag_result_before=0;
                    printf("%dx^%d", result[i], max_degree - i);
                    flag_not_first_print=1;
                    continue;
                }
                else {
                    printf(" + ");
                }
            }
        }
        if (i == max_degree) {
            printf("%d", result[i]);
            flag_not_first_print=1;
            break;
        }
        printf("%dx^%d", result[i], max_degree - i);
        flag_not_first_print=1;
    }
    printf("\n");
}

// Function to multiply two polynomials
void multiply_polynomials(int *poly1, int degree1, int *poly2, int degree2) {
    int max_degree = (degree1 > degree2) ? degree1 : degree2;
    int degree_result = degree1 + degree2;
    int result[degree_result];
    int gap = degree2-degree1;
    if(gap<0){
        gap = gap*(-1);
    }
    // Initialize the result polynomial to all zeroes
    for (int i = 0; i <= degree_result; i++) {
        result[i] = 0;
    }

    // Multiply the polynomials using nested loops
    for (int i = max_degree; i >= 0; i--) {
        for (int j = max_degree; j >= 0; j--) {
            int res = poly1[i] * poly2[j];
            result[i + j - gap] += res;
        }
    }
    // Print the result
    print_result(result, degree_result);
}

// Function to perform polynomial subtraction
void subtract_polynomials(int *coeff1, int *coeff2, int degree1, int degree2, int max) {
    int max_degree = (degree1 > degree2) ? degree1 : degree2;
    int result[max + 1];
    int i;


    // Subtract the coefficients of the two polynomials
    for (i = 0; i <= max_degree; i++) {
        result[i] = coeff1[i] - coeff2[i];
    }


    // Print the result
    print_result(result, max_degree);
}

// Function to perform polynomial addition
void add_polynomials(int *poly1, int *poly2, int degree1, int degree2, int max) {
    int max_degree = (degree1 > degree2) ? degree1 : degree2;
    int result[max + 1];

    for (int i = 0; i <= max_degree; i++) {
        result[i] = poly1[i] + poly2[i];
    }

    // Print the result
    print_result(result, max_degree);
}

// Function to check the operation to be performed
void check_operation(int flag_operation, int *coeff1, int *coeff2, int *degree1, int *degree2, int max) {
    int degree_1 = *degree1;
    int degree_2 = *degree2;
    if (flag_operation == 1) {
        //printf("Addition operation\n");
        add_polynomials(coeff1, coeff2, degree_1, degree_2, max);
    } else if (flag_operation == 2) {
        //printf("Subtraction operation\n");
        subtract_polynomials(coeff1, coeff2, degree_1, degree_2, max);
    } else if (flag_operation == 3) {
        //printf("Multiplication operation\n");
        multiply_polynomials(coeff1, degree_1, coeff2, degree_2);
    } else {
        printf("ERR\n");
    }

}

// Function to process a single polynomial and extract coefficients and degree
void process_polynomial(char *poly_str, int *coeff, int *degree, int diff, int max) {
    int i;
    char *coeff_str, *poly_op, *token, *save_ptr;
    int poly_len = strlen(poly_str);

    // Find the polynomial degree
    token = strtok_r(poly_str, ":", &save_ptr);
    *degree = atoi(token);

    // Find the polynomial coefficients
    coeff_str = strtok_r(NULL, ")", &save_ptr);
    token = strtok(coeff_str, ",");
    if ((*degree) < max) {
        i = diff;
    } else {
        i = 0;
    }
    while (token != NULL) {
        coeff[i++] = atoi(token);
        token = strtok(NULL, ",");
    }

}

// Function to process multiple polynomials from the input string
void process_polys(char *input_str, int *coeff1, int *coeff2, int *degree1, int *degree2, int diff, int max) {
    char *token, *save_ptr;
    int i = 0;

    // Split the string by '(' and process each cut
    token = strtok_r(input_str, "(", &save_ptr);
    while (token != NULL) {
        if (i == 0) {
            process_polynomial(token, coeff1, degree1, diff, max);
        } else if (i == 1) {
            process_polynomial(token, coeff2, degree2, diff, max);
        }
        token = strtok_r(NULL, "(", &save_ptr);
        i++;
    }
}

// Function to get the degree of a polynomial
int get_degree(char *poly_str) {
    int degree = 0;
    char *token, *save_ptr;

    // Find the polynomial degree
    token = strtok_r(poly_str, ":", &save_ptr);
    degree = atoi(token);

    return degree;
}

// Function to get the highest degree among two polynomials
int get_highest_degree(char *input_str) {
    char *token, *save_ptr;
    int highest_degree = 0;

    // Split the input string by '(' and process each polynomial string
    token = strtok_r(input_str, "(", &save_ptr);
    while (token != NULL) {
        // Find the degree of the current polynomial string
        int degree = get_degree(token);
        if (degree > highest_degree) {
            highest_degree = degree;
        }
        token = strtok_r(NULL, "(", &save_ptr);
    }

    return highest_degree;
}

// Function to get the difference between two degree among two polynomials
int get_difference_degree(char *input_str) {
    char *token, *save_ptr;
    int difference[2] = {0, 0};
    int k = 0;

    char temp[strlen(input_str)];
    strcpy(temp, input_str);

    // Split the input string by '(' and process each polynomial string
    token = strtok_r(temp, "(", &save_ptr);
    while (token != NULL) {
        // Find the degree of the current polynomial string
        int degree = get_degree(token);
        difference[k] = degree;
        k++;
        token = strtok_r(NULL, "(", &save_ptr);
    }

    return abs(difference[1] - difference[0]);
}

// all elements in both arrays to zero.
void set_zero(int *arr, int *arr1, int n) {

    for (int i = 0; i < n; i++) {
        arr[i] = 0;
    }

    for (int i = 0; i < n; i++) {
        arr1[i] = 0;
    }
}

//takes a string str as input and returns an integer based on the contents of the string. It checks if the string contains specific substrings ("ADD", "SUB", "MUL")
int get_operation(char *str) {
    if (strstr(str, "ADD")) {
        return 1;
    } else if (strstr(str, "SUB")) {
        return 2;
    } else if (strstr(str, "MUL")) {
        return 3;
    } else {
        return 0;
    }
}

// Function to calculate the resulting polynomial in a separate thread
void *calculateCoefficient(void *data) {
    ThreadData *threadData = (ThreadData *) data;
    int coeffIndex = threadData->coeffIndex;
    int operation = threadData->operation;
    int pol1 = threadData->poly1->polynomial[coeffIndex];
    int pol2 = threadData->poly2->polynomial[coeffIndex];

    if (operation == 1) { // Addition
        pthread_mutex_lock(threadData->mutex);
        threadData->result[coeffIndex] = pol1 + pol2;
        pthread_mutex_unlock(threadData->mutex);
    } else if (operation == 2) { // Subtraction
        pthread_mutex_lock(threadData->mutex);
        threadData->result[coeffIndex] =threadData->poly1->polynomial[coeffIndex] - threadData->poly2->polynomial[coeffIndex];
        pthread_mutex_unlock(threadData->mutex);
    }
    pthread_exit(NULL);
    return NULL;
}

int main() {
    signal(SIGINT, signal_handler);// Set up signal handler for interrupt signal (SIGINT)
    int index_memory_counter = 0;// Counter for the shared memory index
    polynomial poly1;
    polynomial poly2;

    // Open the shared memory object
    if ((fd_shm = shm_open("/shm", O_RDWR, 0)) == -1) {
        perror("ERR\n");
        clean_up();
        exit(1);
    }

    // Map the shared memory object into the process's address space
    if ((shm_ptr = mmap(NULL, SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, fd_shm, 0)) == MAP_FAILED) {
        perror("ERR\n");
        clean_up();
        exit(1);
    }

    // Open the empty semaphore
    if ((empty_sem = sem_open("/empty_sem", 0)) == SEM_FAILED) {
        perror("ERR\n");
        clean_up();
        exit(1);
    }

    // Open the full semaphore
    if ((full_sem = sem_open("/full_sem", 0)) == SEM_FAILED) {
        perror("ERR\n");
        clean_up();
        exit(1);
    }

    // Open the mutex semaphore
    if ((mutex_sem = sem_open("/mutex_sem", 0)) == SEM_FAILED) {
        perror("ERR\n");
        clean_up();
        exit(1);
    }

    while (1) {

        sem_wait(full_sem); //Decrease full in one -- > full - 1
        sem_wait(mutex_sem); //Decrease mutex in one -- > mutex - 1

        // Check if index_memory_counter is less than 10 and the shared memory is not empty
        if (index_memory_counter < 10) {
            char temp[128];
            strncpy(temp, shm_ptr+(index_memory_counter*128), 128);
            if(strcmp(temp,"END")==0){
                clean_up();
                exit(0);
            }

            int diff = get_difference_degree(temp);
            int flag_operation = get_operation(temp);
            int max = get_highest_degree(temp);
            //printf("max value: %d",max);

            // Allocate memory for polynomial arrays
            poly1.polynomial = (int *) malloc((max + 1) * sizeof(int));
            poly2.polynomial = (int *) malloc((max + 1) * sizeof(int));
            poly1.degree = 0;
            poly2.degree = 0;
            //printf("malloc size is : %d",max+1);

            // Set all coefficients to zero
            set_zero(poly1.polynomial, poly2.polynomial, max + 1);

            char send[128];
            strncpy(send, shm_ptr+(index_memory_counter*128), 128);

            process_polys(send, poly1.polynomial, poly2.polynomial, &poly1.degree, &poly2.degree, diff, max);

            if (flag_operation != 3) {
                pthread_t threads[max + 1];
                ThreadData threadData[max + 1];
                pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

                int *result = (int *) malloc((max + 1) * sizeof(int));
                memset(result, 0, (max + 1) * sizeof(int));
                // Create threads to calculate polynomial coefficients
                for (int i = 0; i < max + 1; i++) {
                    threadData[i].poly1 = malloc(sizeof(polynomial));
                    threadData[i].poly2 = malloc(sizeof(polynomial));
                    threadData[i].poly1->polynomial = malloc((max + 1) * sizeof(int));
                    threadData[i].poly2->polynomial = malloc((max + 1) * sizeof(int));
                    for (int j = 0; j < max + 1; ++j) {
                        threadData[i].poly1->polynomial[j] = poly1.polynomial[j];
                        threadData[i].poly2->polynomial[j] = poly2.polynomial[j];
                    }
                    threadData[i].coeffIndex = i;
                    threadData[i].operation = flag_operation;
                    threadData[i].result = result;
                    threadData[i].mutex = &mutex;

                    pthread_create(&threads[i], NULL, calculateCoefficient, (void *) &threadData[i]);
                }

                // Wait for threads to finish
                for (int i = 0; i < max+1; i++) {
                    pthread_join(threads[i], NULL);
                }
                print_result(result, max);

                // Clean up thread data and result array
                for (int i = 0; i < max + 1; ++i) {
                    free(threadData[i].poly1->polynomial);
                    free(threadData[i].poly2->polynomial);
                    free(threadData[i].poly1);
                    free(threadData[i].poly2);
                }
                free(result);
            } else {
                check_operation(flag_operation, poly1.polynomial, poly2.polynomial, &poly1.degree, &poly2.degree, max);
            }
            free(poly1.polynomial);
            free(poly2.polynomial);
            index_memory_counter++;
            if (index_memory_counter == 10) {
                index_memory_counter = 0;
                // Adjust the shm_ptr by subtracting 1280
                // This may indicate managing memory segments in a cyclic manner
            }
        }
        // Increase the mutex semaphore count by 1
        sem_post(mutex_sem); //Increase mutex in one -- > mutex ++

        // Increase the empty semaphore count by 1
        sem_post(empty_sem);

        // Sleep for 1 second before processing the next iteration
        sleep(1);
    }
}//VERSION 1.0 final thread consumer