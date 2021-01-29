#include "lib/clock.h"
#include "lib/ipc.h"
#include "lib/data_structures.h"

// Update 3 small change

int msg_q_id;
bool finished = false;    
void handler(int signum) {
    finished = true;
}

int main(int argc, char * argv[]){
    initClk();

    signal(SIGUSR1, handler);
    msg_q_id = getProcessMessageQueue(KEYSALT);
    printf("Recieved queue with id %d\n", msg_q_id);
    while (1){
        ProcessData recievedProcess = recieveProcessMessage(msg_q_id, SCHEDULER_TYPE);
        if (recievedProcess.pid != -1)
            ProcessData__print(&recievedProcess);
        if (finished) printf("Process Generator sent all !\n");
    }
    destroyClk(false); 
}
