#include "lib/clock.h"
#include "lib/data_structures.h"
#include "lib/process_management.h"
#include "lib/io.h"
#include "lib/scheduling_algorithms.h"
#include "lib/ipc.h"
#include "lib/safeExit.h"

void clearResources(int signum);
int msg_q_id;

int main(int argc, char * argv[]){
    signal(SIGINT, clearResources);
    signal(SIGSEGV, clearResources);
    
    msg_q_id = getProcessMessageQueue(KEYSALT);
    FIFOQueue* fq = FIFOQueue__create();
    FILE* f = openFile("t.txt", "r");

    while (!isEndOfFile(f)) {
        ProcessData* pd = readProcess(f);
        ProcessData__print(pd);
        FIFOQueue__push(fq, pd);
    }

    int algo, q;
    printf("Please enter the algorithm needed (0: HPF - 1: SRTN - 2: RR): ");
    scanf("%d", &algo);

    if (algo == RR) {
        printf("Please enter the Q for RR algorithm: ");
        scanf("%d", &q);
    }

    int clkPid = createChild("./clk.out", 0, 0);
    int schPid = createChild("./scheduler.out", algo, q);
    initClk();
    int newClock, oldClock = -1;
    while (!FIFOQueue__isEmpty(fq)){
        newClock = getClk();
        
        if (newClock > oldClock) {
            ProcessData* top_pd = FIFOQueue__peek(fq);
            while (!FIFOQueue__isEmpty(fq) && top_pd->t_arrival <= newClock) {
                FIFOQueue__pop(fq);
                sendProcessMessage(createProcessMessage(SCHEDULER_TYPE, *top_pd), msg_q_id);
                top_pd = FIFOQueue__peek(fq);
            }
        }
        oldClock = newClock;
    }

    kill(schPid, SIGUSR1);
    waitForChild(schPid);
    destroyClk(true);
}

void clearResources(int signum){
    deleteProcessMessageQueue(msg_q_id);
    destroyClk(true);
    safeExit(-1);
}
