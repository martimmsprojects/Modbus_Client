#include <stdio.h>
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "modbusTCP.h"
#include "modbusAP.h"
char* modbus_exception_msg(uint8_t ex){
    switch(ex){
        case 0x01: return "Illegal Fuction";
        case 0x02: return "Illegal Data Address";
        case 0x03: return "Illegal Data Value";
        case 0x04: return "Slave Device Fail";
        case 0x05: return "Acknowlege";
        case 0x06: return "Slave Device Busy";
        case 0x08: return "Memory Parity Error";
        case 0x0A: return "Gateway Path Unavailable";
        case 0x0B: return "Gateway Target Failed to Respond";
        default:   return "Unknown Exception";
    }
}
int Read_h_regs(char* server_add, int port, uint32_t st_r, uint16_t n_r, uint16_t* val){
    if(!server_add){
        printf("INVALID Server address...\n");
        return -1;
    }
    if(port < 0 || port > 65535){
        printf("INVALID port...\n");
    }

    if(st_r < 0 || st_r > 65536){
        printf("INVALID Register address number...\n");
        return -1;
    }

    if((uint32_t)(n_r) + st_r > 65536){
        printf("INVALID Register address numbers...\n");
        return -1;
    }

    if(n_r > 125 || n_r <= 0){
        printf("INVALID number of reading registers: Value should be in between 1-125");
        return -1;
    }
    
    st_r = st_r - 1;
    uint8_t APDU[5], APDU_R[253]; //começar no zero
    int i = 0;
    APDU[i++] = RHR;
    APDU[i++] = (st_r>>8) & 0xFF;
    APDU[i++] = st_r & 0xFF;
    APDU[i++] = (n_r>>8) & 0xFF;
    APDU[i++] = n_r & 0xFF;

    if(Send_Modbus_Request(server_add, port, APDU, 5, APDU_R) < 0){
        return -1;
    }

    printf("DATA RECIVED ...\n");

    if(APDU_R[0] & 0x80){
        uint8_t ex = APDU_R[1];
        printf("Modbus exception (FC=0x%02X): %s\n", APDU[0] & 0x7F, modbus_exception_msg(ex));
        return -1;
    }         
    for(i=0; i<n_r; i++){
        val[i] = (APDU_R[i*2+2]<<8) | APDU_R[i*2+3];
    }

    return n_r;
}
int Write_multiple_regs (char* server_add, int port, uint32_t st_r, uint16_t n_r, uint16_t* val){
    if(!server_add){
        printf("INVALID Server address...\n");
        return -1;
    }
    if(port < 0 || port > 65535){
        printf("INVALID port...\n");
    }
    if(st_r < 0 || st_r > 65536){
        printf("INVALID Register address number...\n");
        return -1;
    }

    if(n_r + st_r > 65536){
        printf("INVALID Register address numbers...\n");
        return -1;
    }

    if(n_r > 123 || n_r <= 0){
        printf("INVALID number of writting registers: Value should be in between 1-123");
        return -1;
    }

    st_r = st_r - 1;
    uint8_t APDU[6+n_r*2], APDU_R[253];
    int i = 0;
    APDU[i++] = WMR;
    APDU[i++] = (st_r>>8) & 0xFF;  
    APDU[i++] = st_r & 0xFF; 
    APDU[i++] = (n_r>>8) & 0xFF;
    APDU[i++] = n_r & 0xFF;
    APDU[i++] = (n_r*2) & 0xFF;

    for(int j=0; j<n_r; j++){
        APDU[i++] = (val[j]>>8) & 0xFF;
        APDU[i++] = val[j] & 0xFF;
    }

    if(Send_Modbus_Request(server_add, port, APDU, 6+n_r*2, APDU_R) < 0){
        return -1;
    }

    printf("SUCCESSED...\n");

    if(APDU_R[0] & 0x80){
        uint8_t ex = APDU_R[1];
        printf("Modbus exception (FC=0x%02X): %s\n", APDU[0] & 0x7F, modbus_exception_msg(ex));
        return -1;
    }

    return n_r;
}