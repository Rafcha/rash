#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define HOST_NAME_MAX 128
#define MAX_LINE 1024
#define MAX_ARGS 64

int main() {
    char command[MAX_LINE];

	// username ==================
    char *username;
    username = getlogin();
    if (!username) {
        username = "unknown";
    }
    // ===========================

    // hostname ==================
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
    // ===========================

	// direcotry =================
	char buffer[256];
	getcwd(buffer, sizeof(buffer));
	//============================
    puts("Welcome to \x1b[38;5;213mR\x1b[0m\x1b[38;5;197mA\x1b[0m\u001b[31mS\u001b[0m\x1b[38;5;213mH\x1b[0m\n");
    while (1) {
        // start ========
        char display_path[256];
        const char *home = getenv("HOME");

        if (home && strncmp(buffer, home, strlen(home)) == 0) {
            snprintf(display_path, sizeof(display_path), "~%s", buffer + strlen(home));
        } else {
            snprintf(display_path, sizeof(display_path), "%s", buffer);
        }
        printf("\x1b[38;5;213m[ %s ]\x1b[0m@\x1b[38;5;197m[ %s ]\x1b[0m \u001b[31m%s\u001b[0m > ", username, hostname, display_path);
        // ================

        // commands ============
        if (fgets(command, MAX_LINE, stdin) == NULL) {
            break;
        }
        command[strcspn(command, "\n")] = 0;

        char *args[MAX_ARGS];
        int i = 0;
        args[i] = strtok(command, " ");
        while (args[i] != NULL && i < MAX_ARGS - 1) {
            i++;
            args[i] = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (args[0] == NULL) continue;
        if (strcmp(args[0], "exit") == 0) break;
        if (strcmp(args[0], "cd") == 0) {
            const char *path = args[1] ? args[1] : getenv("HOME");
            if (chdir(path) != 0) {
                perror("cd");
            }
            getcwd(buffer, sizeof(buffer));
            continue;
        }
        // ===============

        // fork,exec,wait =========
        pid_t pid = fork();

        if (pid == 0) {
            execvp(args[0], args);
            perror("error");
            exit(1);
        } else if (pid > 0) {
            wait(NULL);
        } else {
            perror("fork failed");
        }
        // ========================
    }

    return 0;
}
