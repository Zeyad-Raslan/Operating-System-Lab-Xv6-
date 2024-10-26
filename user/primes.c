#include "kernel/types.h"
#include "user/user.h"

#define MAX_NUM 280
#define READ 0
#define WRITE 1
#define INT_SIZE 4

void process_primes(int input_pipe[]) __attribute__((noreturn));

void process_primes(int input_pipe[]) {
    int prime;
    int num;
    int output_pipe[2];

    // Close the write end of the input pipe since we only need to read from it
    close(input_pipe[WRITE]);

    // Read the first number from the input pipe, which will be the next prime
    if (read(input_pipe[READ], &prime, INT_SIZE) <= 0) {
        close(input_pipe[READ]);
        exit(0);  // Exit if no more numbers are available
    }

    // Print the prime number
    printf("prime %d\n", prime);

    // Set up a new pipe for the next stage in the pipeline
    if (pipe(output_pipe) < 0) {
        fprintf(2, "Failed to create pipe\n");
        close(input_pipe[READ]);
        exit(1);
    }

    if (fork() == 0) {
        // Child process: recursively process with the output_pipe as input
        close(output_pipe[WRITE]);  // Close the write end in the child process
        close(input_pipe[READ]);    // Close the input pipe in the child process
        process_primes(output_pipe);
    } else {
        // Parent process: filter and pass non-multiples to the child
        close(output_pipe[READ]);  // Close read end in the parent

        // Read numbers from the input pipe and pass non-multiples to output_pipe
        while (read(input_pipe[READ], &num, INT_SIZE) > 0) {
            if (num % prime != 0) {
                write(output_pipe[WRITE], &num, INT_SIZE);
            }
        }

        // Close all ends of the pipes in the parent process
        close(input_pipe[READ]);
        close(output_pipe[WRITE]);
        wait(0);  // Wait for the child to finish
        exit(0);
    }
}

int main(int argc, char *argv[]) {
    int p[2];

    // Create the initial pipe for communication
    if (pipe(p) < 0) {
        fprintf(2, "Failed to create pipe\n");
        exit(1);
    }

    if (fork() == 0) {
        // Child process: start the prime processing chain
        close(p[WRITE]);  // Close write end in the child process
        process_primes(p);
    } else {
        // Parent process: generate numbers 2 to MAX_NUM and pass to child
        close(p[READ]);  // Close the read end in the parent

        for (int i = 2; i <= MAX_NUM; i++) {
            write(p[WRITE], &i, INT_SIZE);  // Write each number into the pipe
        }

        // Close the write end after sending all numbers and wait for the child
        close(p[WRITE]);
        wait(0);
        exit(0);
    }

    return 0;  // Not strictly necessary, as main exits via exit(0)
}

