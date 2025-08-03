#ifndef SIGNALS_H
#define SIGNALS_H

#include <signal.h>

void setup_shell_signals();
void reset_child_signals();
void sigchld_handler(int sig);
const char* get_signal_description(int sig);

extern volatile sig_atomic_t exit_flag;

#endif