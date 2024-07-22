#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/syscall.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/user.h>
#include <sys/reg.h>

// Function to log system call information
void log_syscall(pid_t pid, pid_t tid, struct user_regs_struct *regs, long ret) {
    // Open the log file in append mode
    FILE *logfile = fopen("syscall_log.txt", "a");
    if (!logfile) {
        perror("fopen");
        exit(1);
    }

    // Get current timestamp
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[20];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);

    // Log process/thread ID, timestamp, system call number, arguments, return value, and error code
    fprintf(logfile, "timestamp: %s, pid: %d, tid: %d, syscall: %llu, args: [%llu, %llu, %llu, %llu, %llu, %llu], ret: %lu, errno: %d\n",
            timestamp, pid, tid, regs->orig_rax, regs->rdi, regs->rsi, regs->rdx, regs->r10, regs->r8, regs->r9, ret, errno);

    // Close the log file
    fclose(logfile);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program> [args...]\n", argv[0]);
        return 1;
    }

    // Start the target program
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        // Child process: execute the target program
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
        execvp(argv[1], argv + 1);
        perror("execvp");
        return 1;
    }

    // Parent process: trace the child
    int status;
    while (1) {
        wait(&status);
        if (WIFEXITED(status)) break;

        // Get the system call number
        struct user_regs_struct regs;
        ptrace(PTRACE_GETREGS, pid, 0, &regs);

        // Log the system call and its return value
        long ret = ptrace(PTRACE_PEEKUSER, pid, sizeof(long) * ORIG_RAX, NULL);
        log_syscall(pid, pid, &regs, ret);

        // Continue the child process
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
    }

    return 0;
}


