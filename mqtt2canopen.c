#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

#include <co_canopen.h>

void catch_signal(int sig)
{
(void)sig; /* unused */
}

#define CO_TIMER_INTERVAL 5000

const CO_OD_ASSIGN_T od_assign[] = {
    { 0x1000, 1, 0, CO_ODTYPE_VAR, 0 },
    { 0x1001, 1, 0, CO_ODTYPE_VAR, 1 },
};

const CO_OBJECT_DESC_T od_description[] = {
    { /* 0x1000:0 */
        0,                                              /* subIndex */
        CO_DTYPE_U32_VAR,                               /* dType */
        0,                                              /* tableIdx */
        CO_ATTR_NUM | CO_ATTR_READ | CO_ATTR_DEFVAL,    /* attr */
        0,                                              /* defValIdx */
        0,                                              /* limitMinIdx */
        0                                               /* limitMaxIdx */
    },
    { /* 0x1001:0 */
        0,
        CO_DTYPE_U8_EMCY,
        0x1001,
        CO_ATTR_NUM | CO_ATTR_READ,
        0,
        0,
        0
    },
};

const UNSIGNED32 odConst_u32[] = {
    0x301,
};

UNSIGNED32 odVar_u32[] = {
    0,
};

CO_CONST CO_OD_DATA_VARIABLES_T od_data_variables = {
    NULL, /* odConst_u8 */
    NULL, /* odConst_u16 */
    odConst_u32, /* odConst_u32 */
    NULL, /* odConst_r32 */
    NULL, /* odVar_u8 */
    NULL, /* odVar_u16 */
    odVar_u32, /* odVar_u32 */
    NULL, /* odVar_r32 */
    NULL, /* odConst_i8 */
    NULL, /* odConst_i16 */
    NULL, /* odConst_i32 */   
    NULL, /* odVar_i8 */
    NULL, /* odVar_i16 */
    NULL, /* odVar_i32 */
    NULL, /* odPtr_u8 */
    NULL, /* odPtr_u16 */
    NULL, /* odPtr_u32 */
    NULL, /* odPtr_i8 */
    NULL, /* odPtr_i16 */
    NULL, /* odPtr_i32 */
    NULL, /* odPtr_r32 */
    NULL, /* odConstVisString */
    NULL, /* odConstVisStringLen */
    NULL, /* odVisString */
    NULL, /* odVisStringLen */
    NULL, /* odOctetString */
    NULL, /* odOctetStringLen */
    NULL, /* domainPtr */
    NULL, /* domainLen */
};

int main(int argc, char** argv)
{
    int opt;
    while ( (opt = getopt(argc, argv, "h:p:U:P:t:i:B:n:")) != -1 ) {
        switch(opt) {
        case 'h': /* broker_hostname */
        case 'p': /* broker_port */
        case 'U': /* broker_username */
        case 'P': /* broker_password */
        case 't': /* mqtt_topic_prefix */
            break;
        case 'i':
            /*sscanf(optarg, "%d", &nodeid);*/
            break;
        default:
            fprintf(stderr, "Usage: %s [-i interface] [-B baudrate] [-n nodeId]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    int ret;

    if ( codrvCanInit(250 /* FIXME */) != RET_OK ) {
        fprintf(stderr, "error initialising CAN driver\n");
        exit(EXIT_FAILURE);
    }
    printf("initialised CAN driver\n");

    coOdInitOdPtr(
        od_assign,      sizeof(od_assign)      / sizeof(CO_OD_ASSIGN_T), 
        od_description, sizeof(od_description) / sizeof(CO_OBJECT_DESC_T), 
        NULL /* pEventPtr */, 
        &od_data_variables);
    printf("initialised object dictionary\n");
    
    coTimerInit(CO_TIMER_INTERVAL);
    printf("initialised timer\n");

    coLedInit();
    printf("initialised LED\n");

    if ( (ret = coSdoServerInit(1)) != RET_OK ) {
        fprintf(stderr, "error initialising SDO server (%d)\n", ret);
        exit(EXIT_FAILURE);
    }
    printf("initialised SDO server\n");

    if ( (ret = coErrorCtrlInit(0, 0)) != RET_OK ) {
        fprintf(stderr, "error initialising error control (%d)\n", ret);
        exit(EXIT_FAILURE);
    }
    printf("initialised error control\n");

    if ( (ret = coEmcyProducerInit()) != RET_OK ) {
        fprintf(stderr, "error initialising EMCY producer (%d)\n", ret);
        exit(EXIT_FAILURE);
    }
    printf("initialised EMCY producer\n");

    if ( (ret = coNmtInit(0)) != RET_OK ) {
        fprintf(stderr, "error initialising NMT (%d)\n", ret);
        exit(EXIT_FAILURE);
    }
    printf("initialised NMT\n");

    if ( (ret = codrvTimerSetup(CO_TIMER_INTERVAL)) != RET_OK ) {
        fprintf(stderr, "error setting up CANopen timer (%d)\n", ret);
        return ret;
    }
    printf("setup CANopen timer\n");

    if ( (ret = codrvCanEnable()) != RET_OK ) {
        fprintf(stderr, "error enabling CAN driver (%d)\n", ret);
        return ret;
    }
    printf("enabled CAN driver\n");

    for (;;) {
        usleep(1000);
        coCommTask();
    }

    return 0;
}
