#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>
#define MAX_SYSCALLS 1024

// Inside the main function
int call_count[MAX_SYSCALLS] = {0}; // Array to store call counts

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <target_program>\n", argv[0]);
        return 1;
    }

    pid_t pid;
    if ((pid = fork()) == 0) {
        // Child process
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], &argv[1]);
    } else if (pid > 0) {
        // Parent process
        int status;
        struct user_regs_struct regs;
        long orig_rax;

        waitpid(pid, &status, 0);
        ptrace(PTRACE_SETOPTIONS, pid, 0, PTRACE_O_EXITKILL);

        while (WIFSTOPPED(status)) {
            ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
            waitpid(pid, &status, 0);

            if (WIFSTOPPED(status)) {
                ptrace(PTRACE_GETREGS, pid, NULL, &regs);
                orig_rax = regs.orig_rax;
                call_count[orig_rax]++; // Increment call count for the syscall

                ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
                waitpid(pid, &status, 0);
            }
        }

        printf("System call profiling:\n");
        for (int i = 0; i <= MAX_SYSCALLS; i++) {
            if (call_count[i] > 0) {
                printf("Syscall %d (count=%d)\n", i, call_count[i]);
            }
        }

    } else {
        fprintf(stderr, "Failed to fork\n");
        return 1;
    }

    return 0;
}

