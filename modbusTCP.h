#ifndef MODBUS_TCP_H_
#define MODBUS_TCP_H_
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>

int Send_Modbus_Request(char* server_add, int port, uint8_t* APDU, int APDUlen, uint8_t* APDU_R);

int write_modbus(int fd, uint8_t* PDU, int PDUlen);

int read_modbus(int fd, uint8_t* PDU_R, int PDU_Rlen);


#endif 