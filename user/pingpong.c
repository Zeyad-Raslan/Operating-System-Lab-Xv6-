#include "kernel/types.h"
#include "user/user.h"

int main() {
    int p_to_c[2]; // Pipe from parent to child
    int c_to_p[2]; // Pipe from child to parent
    char buf;
    int pid;

    // Create two pipes for communication in both directions
    pipe(p_to_c);
    pipe(c_to_p);

    if ((pid = fork()) == 0) {
        // Child process

        // Read a byte from the parent
        read(p_to_c[0], &buf, 1);

        // Print "<pid>: received ping"
        printf("%d: received ping\n", getpid());

        // Send the byte back to the parent
        write(c_to_p[1], &buf, 1);

        // Close all pipe ends in the child
        close(p_to_c[0]);
        close(p_to_c[1]);
        close(c_to_p[0]);
        close(c_to_p[1]);

        // Exit child process
        exit(0);
    } else {
        // Parent process

        // Send a byte to the child to start communication
        write(p_to_c[1], "x", 1);

        // Read the byte sent back by the child
        read(c_to_p[0], &buf, 1);

        // Print "<pid>: received pong"
        printf("%d: received pong\n", getpid());

        // Close all pipe ends in the parent
        close(p_to_c[0]);
        close(p_to_c[1]);
        close(c_to_p[0]);
        close(c_to_p[1]);

        // Wait for the child to exit
        wait(0);
    }

    // Exit parent process
    exit(0);
}

