#include "Signals.h"
#include "player.h"

void Signals_Setup()
{
    signal(SIGSEGV, SegmentationFault);
    signal(SIGHOLD, FuncStop);
}

void SegmentationFault(int sig)
{
    printf("Caught Seg Fault, exiting...\n");
    exit(-1);
}

void FuncStop(int sig)
{
    if(pthread_equal(pthread_self(), play_thread_id))
    {
        play_thread_wait = true;
    }
}
