#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <signal.h>

// Structure to hold system call data
struct SysCallData {
    pid_t pid;
    int syscall_number;
    // Add other fields as needed (e.g., arguments, return value, timestamp)
};

// Function to trace system calls in a child process
void *trace_child(void *arg) {
    pid_t pid = *((pid_t *)arg);
    int trace_duration = 10; // Trace for 10 seconds (adjust as needed)
    int elapsed_time = 0;

    // Simulated tracing loop with termination condition
    while (elapsed_time < trace_duration) {
        // Placeholder: Simulate tracing activity
        printf("Tracing system calls for process %d\n", pid);
        sleep(1); // Sleep for 1 second

        elapsed_time++; // Increment elapsed time
    }

    printf("Trace for process %d complete.\n", pid);

    pthread_exit(NULL);
}

// Signal handler for SIGINT (Ctrl+C)
void sigint_handler(int signum) {
    // Placeholder: Handle SIGINT signal
    printf("Received SIGINT. Exiting.\n");
    exit(EXIT_SUCCESS);
}

int main() {
    // Register signal handler for SIGINT
    signal(SIGINT, sigint_handler);

    // Create multiple child processes
    int num_processes = 2;
    pid_t child_pids[num_processes];
    for (int i = 0; i < num_processes; i++) {
        child_pids[i] = fork();
        if (child_pids[i] == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (child_pids[i] == 0) { // Child process
            // Placeholder: Execute child process logic
            // For example, you can execute an external program using exec functions
            execl("/bin/ls", "ls", "-l", NULL);
            perror("execl");
            exit(EXIT_FAILURE);
        }
    }

    // Create a thread for each child process to trace system calls
    pthread_t threads[num_processes];
    for (int i = 0; i < num_processes; i++) {
        if (pthread_create(&threads[i], NULL, trace_child, (void *)&child_pids[i]) != 0) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for the threads to finish
    for (int i = 0; i < num_processes; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            perror("pthread_join");
            exit(EXIT_FAILURE);
        }
    }

    // Wait for the child processes to exit
    for (int i = 0; i < num_processes; i++) {
        int status;
        waitpid(child_pids[i], &status, 0);
        if (WIFEXITED(status)) {
            printf("Child process %d exited with status %d\n", child_pids[i], WEXITSTATUS(status));
        } else {
            printf("Child process %d exited abnormally\n", child_pids[i]);
        }
    }

    return 0;
}

