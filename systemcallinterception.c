#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/syscall.h>

const char *syscall_names[] = {
    [0] = "read",
    [1] = "write",
    [2] = "open",
    [3] = "close",
    [4] = "fstat",
    [5] = "lseek",
    [6] = "mmap",
    [7] = "mprotect",
    [8] = "munmap",
    [9] = "brk",
    [10] = "access",
    [11] = "pipe",
    [12] = "select",
    [13] = "dup",
    [14] = "fcntl",
    [15] = "execve",
    [16] = "exit",
    [17] = "wait4",
};

int main() {
    pid_t pid;
    struct user_regs_struct regs;
    int status;

    // Launch the target process
    pid = fork();

    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process (target code)
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execl("./target", "target", NULL);
        perror("execl");
        exit(EXIT_FAILURE);
    }

    // Parent process (tracer)
    waitpid(pid, &status, 0);

    // Trace system calls
    while (1) {
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        waitpid(pid, &status, 0);

        if (WIFEXITED(status)) {
            printf("Target process exited.\n");
            break;
        }

        // Get system call number
        ptrace(PTRACE_GETREGS, pid, NULL, &regs);
        long syscall_num = regs.orig_rax;

        printf("System call %ld\n", syscall_num);

        // Optionally, you can log or analyze the system call further here

        // Continue execution
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
        waitpid(pid, &status, 0);
    }

    return 0;
}
