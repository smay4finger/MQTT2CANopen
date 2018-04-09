#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <mosquitto.h>

#include "canfestival.h"
#include "dictionary.h"
#include "dcf.h"

char* broker_hostname = "localhost";
int broker_port = 1883;
char* broker_username = NULL;
char* broker_password = NULL;

char* mqtt_topic_prefix = "canopen";

struct mosquitto* mosq = NULL;

int nodeid = 127;
s_BOARD board = { "can0", "20k" };

/****************************************************************************/
/*   MQTT callbacks                                                         */
/****************************************************************************/

void mqtt_connect_callback(struct mosquitto *mosq, void *userdata, int result)
{
userdata = userdata; /* unused */
    printf("----- mqtt_connect_callback(mosq=%p, userdata=%p, result=%d)\n",
        mosq,
        userdata,
        result);
    if ( !result ) {
        char topic[2048];
        snprintf(topic, sizeof(topic), "%s/%s/node%d/#", 
            mqtt_topic_prefix,
            board.busname,
            nodeid);
        mosquitto_subscribe(mosq, NULL, topic, 0);
    }
}

void mqtt_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
{
mosq = mosq; /* unused */
userdata = userdata; /* unused */
    printf("----- mqtt_message_callback(mosq=%p, userdata=%p, message=[topic=%s, payload=%s])\n",
        mosq,
        userdata,
        message->topic,
        (char*)message->payload);
}

void mqtt_publish_nodestate(char* state)
{
    char topic[2048];
    char payload[2048];
    snprintf(topic, sizeof(topic), "%s/%s/node%d/state",
        mqtt_topic_prefix,
        board.busname,
        nodeid);
    snprintf(payload, sizeof(payload), "%s", state);
    mosquitto_publish(mosq, NULL, topic, 
        strlen(payload), payload, 
        0 /* qos */,
        true /* retain */);
}

/****************************************************************************/
/*   CANopen callbacks                                                      */
/****************************************************************************/

UNS32 mapUpdate(CO_Data* d, const indextable* indextable, UNS8 bSubindex)
{
    char topic[2048] = "";
    char message[2048] = "";

    subindex s = indextable->pSubindex[bSubindex];

    printf("----- mapUpdate(d=%p, indextable=%p, bSubindex=%d)\n", d, indextable, bSubindex);
    printf("-----    index=%04X\n", indextable->index);
    printf("-----    bSubindex=%d\n", bSubindex);
    printf("-----    datatype=%d\n", indextable->pSubindex[bSubindex].bDataType);
    printf("-----    size=%d\n", indextable->pSubindex[bSubindex].size);


    printf("-----    size=%d\n", s.size);

    snprintf(topic, sizeof(topic), "%s/%s/node%d/%04x/%d",
        mqtt_topic_prefix,
        board.busname,
        nodeid,
        indextable->index,
        bSubindex);

    switch(s.bDataType) {
    case int8:
        snprintf(message, sizeof(message), "%d", *((INTEGER8*)s.pObject)); break;
    case int16:
        snprintf(message, sizeof(message), "%d", *((INTEGER16*)s.pObject)); break;
    case int24:
        snprintf(message, sizeof(message), "%d", *((INTEGER24*)s.pObject)); break;
    case int32:
        snprintf(message, sizeof(message), "%d", *((INTEGER32*)s.pObject)); break;
    case int40:
        snprintf(message, sizeof(message), "%ld", *((INTEGER40*)s.pObject)); break;
    case int48:
        snprintf(message, sizeof(message), "%ld", *((INTEGER48*)s.pObject)); break;
    case int56:
        snprintf(message, sizeof(message), "%ld", *((INTEGER56*)s.pObject)); break;
    case int64:
        snprintf(message, sizeof(message), "%ld", *((INTEGER64*)s.pObject)); break;
    case uint8:
        snprintf(message, sizeof(message), "%d", *((UNS8*)s.pObject)); break;
    case uint16:
        snprintf(message, sizeof(message), "%d", *((UNS16*)s.pObject)); break;
    case uint24:
        snprintf(message, sizeof(message), "%d", *((UNS24*)s.pObject)); break;
    case uint32:
        snprintf(message, sizeof(message), "%d", *((UNS32*)s.pObject)); break;
    case uint40:
        snprintf(message, sizeof(message), "%ld", *((UNS40*)s.pObject)); break;
    case uint48:
        snprintf(message, sizeof(message), "%ld", *((UNS48*)s.pObject)); break;
    case uint56:
        snprintf(message, sizeof(message), "%ld", *((UNS56*)s.pObject)); break;
    case uint64:
        snprintf(message, sizeof(message), "%ld", *((UNS64*)s.pObject)); break;

    case real32: /* float */
        snprintf(message, sizeof(message), "%f", *((REAL32*)s.pObject)); break;
    case real64: /* double */
        snprintf(message, sizeof(message), "%f", *((REAL64*)s.pObject)); break;

    case visible_string:
    case octet_string:
    case unicode_string:
    case time_of_day:
    case time_difference:
    case domain:
    case boolean:
    default:
        printf("------ unimplemented datatype\n");
        return 0;
    }

    printf("-----    MQTT topic=%s payload=%s\n", topic, message);

    mosquitto_publish(mosq, NULL, topic, 
        strlen(message), message,
        0 /* qos */,
        false /* retain */);

    return 0;
}

void initialisation(CO_Data* d)
{
    printf("----- initialisation(d=%p)\n", d);
    mqtt_publish_nodestate("initialisation");
}

void preOperational(CO_Data* d)
{
    printf("----- preOperational(d=%p)\n", d);
    mqtt_publish_nodestate("preOperational");
}

void operational(CO_Data* d)
{
    printf("----- operational(d=%p)\n", d);
    mqtt_publish_nodestate("operational");
}

void stopped(CO_Data* d)
{
    printf("----- stopped(d=%p)\n", d);
    mqtt_publish_nodestate("stopped");
}

void heartbeatError(CO_Data* d, UNS8 heartbeatID)
{
    printf("----- heatbeatError(d=%p, heatbeatID=%d)\n", d, heartbeatID);
}

void post_SlaveBootup(CO_Data* d, UNS8 nodeId)
{
    printf("----- post_SlaveBootup(d=%p, nodeId=%d)\n", d, nodeId);
}

void post_SlaveStateChange(CO_Data* d, UNS8 nodeId, e_nodeState newNodeState)
{
    printf("----- post_SlaveStateChange(d=%p, nodeId=%d, newNodeState=%d)\n", d, nodeId, newNodeState);
}

void post_TPDO(CO_Data* d)
{
    printf("----- post_TPDO(d=%p)\n", d);
}

void post_sync(CO_Data* d)
{
    printf("----- post_sync(d=%p)\n", d);
}

void post_emcy(CO_Data* d, UNS8 nodeID, UNS16 errCode, UNS8 errReg, const UNS8 errSpec[5])
{
    printf("----- post_emcy(d=%p, nodeID=%d, errCode=0x%04x, errReg=0x%02x) "\
                "errSpec=[0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x]\n",
        d, nodeID, errCode, errReg, 
        errSpec[0], errSpec[1], errSpec[2], errSpec[3], errSpec[4]);
}

UNS32 storeODSubIndex(CO_Data* d, UNS16 wIndex, UNS8 bSubindex)
{
    printf("----- storeODSubIndex(d=%p, wIndex=0x%04x, bSubindex=%d)\n",
        d, wIndex, bSubindex);
    return 0;
}

/****************************************************************************/

void catch_signal(int sig)
{
sig = sig;
    signal(SIGTERM, catch_signal);
    signal(SIGINT, catch_signal);
}

void InitNodes(CO_Data* d, UNS32 id)
{
    printf("InitNodes(d=%p, id=%d)\n", d, id);

    /* register callbacks for manufacturer specific variables */
#if 0
    int wIndex;
    for ( wIndex = 0x2000; wIndex < 0x6000; wIndex++ ) {
        UNS32 errorCode;
        ODCallback_t* callbacks;
        indextable* odentry = mqtt2canopen_scanIndexOD(&mqtt2canopen_Data, wIndex, &errorCode);
        if (errorCode == OD_SUCCESSFUL && callbacks) {
            int subindex;
            for ( subindex = 0; subindex < odentry->bSubCount; subindex++ ) {
                callbacks[subindex] = mapUpdate;
            }
        }
    }
#endif

    setNodeId(&mqtt2canopen_Data, nodeid);
    setState(&mqtt2canopen_Data, Initialisation);
}

void Exit(CO_Data* d, UNS32 id)
{
    printf("Exit(d=%p, id=%d)\n", d, id);

    setState(&mqtt2canopen_Data, Stopped);
}

/****************************************************************************/

int main(int argc, char** argv)
{
#ifdef CANFESTIVAL_DYNAMIC_LOAD
    char* can_library = "libcanfestival_can_socket.so";
#endif
    int opt;
    while ( (opt = getopt(argc, argv, "h:p:U:P:t:b:B:i:L:")) != -1 ) {
        switch(opt) {
        case 'h': /* broker_hostname */
        case 'p': /* broker_port */
        case 'U': /* broker_username */
        case 'P': /* broker_password */
        case 't': /* mqtt_topic_prefix */
            break;
        case 'b':
            board.busname = optarg;
            break;
        case 'B':
            board.baudrate = optarg;
            break;
        case 'i':
            sscanf(optarg, "%d", &nodeid);
            break;
        case 'L':
#ifdef CANFESTIVAL_DYNAMIC_LOAD
            can_library = optarg;
#else
            fprintf(stderr, "warning: will not load CAN driver, dynamic load was disabled at compile time.\n");
#endif
            break;
        default:
            fprintf(stderr, "Usage: %s [-b busname] [-B baudrate] [-i nodeid]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    /* mosquitto init */
    mosquitto_lib_init();
    if ( (mosq = mosquitto_new(NULL, true, NULL)) == NULL ) {
        perror("Mosquitto initialization failed");
        exit(EXIT_FAILURE);
    }
    mosquitto_connect_callback_set(mosq, mqtt_connect_callback);
    mosquitto_message_callback_set(mosq, mqtt_message_callback);

    {
        char topic[2048];
        char payload[2048];
        snprintf(topic, sizeof(topic), "%s/%s/node%d/state",
            mqtt_topic_prefix,
            board.busname,
            nodeid);
        snprintf(payload, sizeof(payload), "unknown");
        mosquitto_will_set(mosq, topic, strlen(payload), payload, 0, true);
    }

    if ( mosquitto_connect_async(mosq, broker_hostname, broker_port, 20) ) {
        fprintf(stderr, "mosquitto_connect_async failed.\n");
        exit(EXIT_FAILURE);
    }
    if ( mosquitto_loop_start(mosq) ) {
        fprintf(stderr, "mosquitto_loop_start failed.\n");
        exit(EXIT_FAILURE);
    }

    /* CanFestival init */
    signal(SIGTERM, catch_signal);
    signal(SIGINT, catch_signal);

    TimerInit();
#ifdef CANFESTIVAL_DYNAMIC_LOAD
    if ( !LoadCanDriver(can_library) ) {
        fprintf(stderr, "couldn't load CAN driver\n");
        exit(EXIT_FAILURE);
    }
#endif
    if ( !canOpen(&board, &mqtt2canopen_Data) ) {
        fprintf(stderr, "couldn't initialise CANopen\n");
        goto fail;
    }

    mqtt2canopen_Data.initialisation        = initialisation;
    mqtt2canopen_Data.preOperational        = preOperational;
    mqtt2canopen_Data.operational           = operational;
    mqtt2canopen_Data.stopped               = stopped;
    mqtt2canopen_Data.heartbeatError        = heartbeatError;
    mqtt2canopen_Data.post_SlaveBootup      = post_SlaveBootup;
    mqtt2canopen_Data.post_SlaveStateChange = post_SlaveStateChange;
    mqtt2canopen_Data.post_TPDO             = post_TPDO;
    mqtt2canopen_Data.post_sync             = post_sync;
    mqtt2canopen_Data.post_emcy             = post_emcy;
    mqtt2canopen_Data.storeODSubIndex       = storeODSubIndex;

    StartTimerLoop(&InitNodes);

    pause();

    /* CanFestival destroy */
    StopTimerLoop(&Exit);
fail:
    TimerCleanup();

    /* mosquitto destroy */
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();

    return 0;
}
