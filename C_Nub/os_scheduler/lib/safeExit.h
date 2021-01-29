#pragma once
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

void safeExit(int status) {
    if (status == -1)
        killpg(getgid(), SIGINT);
    exit(status);
}
