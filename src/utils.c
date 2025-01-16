#include "shell.h"

void addToHistory(const char *cmd) {
    if (history_count < MAX_HISTORY) {
        history[history_count] = strdup(cmd);
        history_count++;
    } else {
        perror("Max history count exceeded, please restart shell");
    }
    curr_hist_index = history_count;
}
