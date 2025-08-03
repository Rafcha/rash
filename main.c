#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include "modules/signals.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <limits.h>

#define HOST_NAME_MAX 128
#define MAX_LINE 1024
#define MAX_ARGS 64

int main() {
    char history_path[PATH_MAX];
    snprintf(history_path, sizeof(history_path), "%s/.bash_history", getenv("HOME"));
    read_history(history_path);

    setup_shell_signals();

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

    while (!exit_flag) {
        getcwd(buffer, sizeof(buffer));
        // start ========
        char display_path[256];
        const char *home = getenv("HOME");

        if (home && strncmp(buffer, home, strlen(home)) == 0) {
            snprintf(display_path, sizeof(display_path), "~%s", buffer + strlen(home));
        } else {
            snprintf(display_path, sizeof(display_path), "%s", buffer);
        }
        char prompt_string[512];
        snprintf(prompt_string, sizeof(prompt_string),
                "\x1b[38;5;213m[ %s ]\x1b[0m@\x1b[38;5;197m[ %s ]\x1b[0m \u001b[31m%s\u001b[0m > ",
                username, hostname, display_path);
        // ================

        // commands ============
        char *command = readline(prompt_string);
        if (!command) break;
        if (*command) add_history(command);

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
            reset_child_signals();
            execvp(args[0], args);
            perror("execvp error");
            exit(127);
        } else if (pid > 0) {
            int status;
            
            while (1) {
                if (waitpid(pid, &status, WUNTRACED) == -1) {
                    if (errno == EINTR) continue;
                    perror("waitpid error");
                    break;
                }
                
                if (WIFEXITED(status)) {
                    int exit_status = WEXITSTATUS(status);
                    if (exit_status != 0) {
                        printf("\n[%d] exited with code %d\n", pid, exit_status);
                    }
                    break;
                } else if (WIFSIGNALED(status)) {
                    int term_sig = WTERMSIG(status);
                    printf("\n[%d] terminated by signal %d (%s)\n", 
                        pid, term_sig, get_signal_description(term_sig));
                    break;
                } else if (WIFSTOPPED(status)) {
                    int stop_sig = WSTOPSIG(status);
                    printf("\n[%d] stopped by signal %d (%s)\n", 
                        pid, stop_sig, get_signal_description(stop_sig));
                    break;
                }
            }
        } else {
            perror("fork failed");
        }
        // ========================
        free(command);
    }
    write_history(history_path);
    puts("Goodbye! ;>");
    return 0;
}
