#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

// Function to subtract two polynomials
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

// Function to add two polynomials
void add_polynomials(int *poly1, int *poly2, int degree1, int degree2, int max) {
    int max_degree = (degree1 > degree2) ? degree1 : degree2;
    int result[max + 1];

    for (int i = 0; i <= max_degree; i++) {
        result[i] = poly1[i] + poly2[i];
    }

    // Print the result
    print_result(result, max_degree);
}

// Function to check the operation to perform and call the appropriate function
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

// Function to process the input string containing two polynomial expressions
void process_polynomial(char *poly_str, int *coeff, int *degree, int diff, int max) {
    int i;
    char *coeff_str, *token, *save_ptr;
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

// Function to process the input string containing two polynomial expressions
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

// Function to extract the degree of a polynomial from a polynomial string
int get_degree(char *poly_str) {
    int degree = 0;
    char *token, *save_ptr;

    // Find the polynomial degree
    token = strtok_r(poly_str, ":", &save_ptr);
    degree = atoi(token);

    return degree;
}

// Function to find the highest degree among the polynomials in the input string
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

// Function to calculate the absolute difference in degrees between the two polynomials
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

// Function to set all elements of two arrays to zero
void set_zero(int *arr, int *arr1, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = 0;
    }

    for (int i = 0; i < n; i++) {
        arr1[i] = 0;
    }
}

//function to get the operation
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

// Function to check the size of the input string
void check_string_size(char *str) {
    if (strlen(str) > 128) {
        printf("ERR\n");
        exit(1);
    }
}

int main() {

    while (1) {
        char input_str[130];
        int degree1 = 0, degree2 = 0;

        // Read the input string
        //printf("Enter the string:");
        fgets(input_str, 130, stdin);
        // Check the size of the input string
        check_string_size(input_str);

        input_str[strcspn(input_str, "\n")] = 0; // Remove the trailing newline character

        if (strcmp(input_str, "END") == 0) {
            exit(0); // Terminate the program
        }

        char temp[strlen(input_str)];
        strcpy(temp, input_str);

        // Get the highest degree among the polynomials
        int diff = get_difference_degree(temp);
        //printf("the diff degree is %d\n ", diff);

        int flag_operation = get_operation(temp);
        //printf("the flag  is %d\n ", flag_operation);

        int max = get_highest_degree(temp);
        //printf("the max degree is %d\n ", max);

        int coeff1[max + 1];
        int coeff2[max + 1];
        set_zero(coeff1, coeff2, max+ 1);

        // Check the operation and call the appropriate function
        process_polys(input_str, coeff1, coeff2, &degree1, &degree2, diff, max);
        check_operation(flag_operation, coeff1, coeff2, &degree1, &degree2, max);
    }
}//SOF CODE 1.1 END