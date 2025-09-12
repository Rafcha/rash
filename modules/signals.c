#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

volatile sig_atomic_t exit_flag = 0;

// signals
static void sigint_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);
    rl_replace_line("", 0);
    rl_on_new_line();
    rl_redisplay();
}


static void sigtstp_handler(int sig) {
    (void)sig;
    write(STDOUT_FILENO, "\n", 1);

    rl_replace_line("", 0);
    rl_on_new_line();
    rl_redisplay();
}

static void sigterm_handler(int sig) {
    exit_flag = 1;
    write(STDOUT_FILENO, "\nSIGTERM received, exiting...\n", 30);
}

const char* get_signal_description(int sig) {
    return strsignal(sig);
}

// SIGCHLD
void sigchld_handler(int sig) {
    int saved_errno = errno;
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {
        if (WIFSTOPPED(status)) {
            int stop_sig = WSTOPSIG(status);
            printf("\n[%d] stopped by signal %d (%s)\n", 
            pid, stop_sig, get_signal_description(stop_sig));
        } else if (WIFSIGNALED(status)) {
            int term_sig = WTERMSIG(status);
            printf("\n[%d] killed by signal %d (%s)\n", 
            pid, term_sig, get_signal_description(term_sig));
        }
    }
    errno = saved_errno;
}


// signals for shell
void setup_shell_signals() {
    struct sigaction sa;

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    // SIGINT
    sa.sa_handler = sigint_handler;
    sa.sa_flags = SA_RESTART;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    // SIGTSTP
    sa.sa_handler = sigtstp_handler;
    sigaction(SIGTSTP, &sa, NULL);

    // SIGTERM and SIGHUP
    sa.sa_handler = sigterm_handler;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);

    // SIGCHLD
    sa.sa_handler = sigchld_handler;
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    sigaction(SIGCHLD, &sa, NULL);
}

void reset_child_signals() {
    signal(SIGINT, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
    signal(SIGHUP, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
}
