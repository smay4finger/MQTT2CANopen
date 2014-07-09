#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>

#include "canfestival.h"
#include "dictionary.h"

int nodeid = 1;
s_BOARD board = { "vcan0", "500k" };

void catch_signal(int sig)
{
    signal(SIGTERM, catch_signal);
    signal(SIGINT, catch_signal);
    printf("Got Signal %d\n",sig);
}

void InitNodes(CO_Data* d, UNS32 id)
{
d=d;
    printf("InitNodes(id=%d)\n", id);
    setNodeId(&mqtt_Data, nodeid);
    setState(&mqtt_Data, Initialisation);
}

void Exit(CO_Data* d, UNS32 id)
{
d=d;
    printf("Exit(id=%d)\n", id);
    setState(&mqtt_Data, Stopped);
}

int main(int argc, char** argv)
{
    char* can_library = "libcanfestival_can_socket.so";
    int opt;
    while ( (opt = getopt(argc, argv, "b:B:i:L:")) != -1 ) {
        switch(opt) {
        case 'b':
            board.busname = optarg;
            break;
        case 'B':
            board.baudrate = optarg;
            break;
        case 'i':
            sscanf(optarg, "%x", &nodeid);
            break;
        case 'L':
            can_library = optarg;
            break;
        default:
            fprintf(stderr, "Usage: %s [-b busname] [-B baudrate]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    signal(SIGTERM, catch_signal);
    signal(SIGINT, catch_signal);

    TimerInit();

#ifdef CANFESTIVAL_DYNAMIC_LOAD
    if ( !LoadCanDriver(can_library) ) {
        fprintf(stderr, "couldn't load CAN driver\n");
        exit(EXIT_FAILURE);
    }
#endif

    if ( !canOpen(&board, &mqtt_Data) ) {
        fprintf(stderr, "couldn't initialise CANopen\n");
        goto fail;
    }
    
    StartTimerLoop(&InitNodes);

    pause();

    StopTimerLoop(&Exit);

fail:
    TimerCleanup();

    return 0;
}
