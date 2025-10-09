#include <stdio.h>
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>
#include "modbusTCP.h"
#include "modbusAP.h"
static uint16_t transaction_id = 0;

uint16_t get_transaction_id(){
    transaction_id++;
    if(transaction_id == 0x0000){
        transaction_id++;
    }
    return transaction_id;
}

int write_modbus(int fd, uint8_t* PDU, int PDUlen){
    return send(fd, PDU, PDUlen, 0);
}

int read_modbus(int fd, uint8_t* PDU_R, int PDU_Rlen){
    return recv(fd, PDU_R, PDU_Rlen, 0);
}

int Send_Modbus_Request(char* server_add, int port, uint8_t* APDU, int APDUlen, uint8_t* APDU_R){
   int fd = socket(PF_INET , SOCK_STREAM , IPPROTO_TCP);
   int in=0;
   struct sockaddr_in server;

   uint8_t PDU[7+APDUlen], PDU_R[260];

   uint16_t transactionId = get_transaction_id();

   uint16_t len = APDUlen + 1;
    //MBAP

   PDU[0] = (transactionId >> 8) & 0xFF;
   PDU[1] = transactionId & 0xFF;
   PDU[2] = 0x00;
   PDU[3] = 0x00;
   PDU[4] = (len >> 8) & 0xFF;
   PDU[5] = len & 0xFF;
   PDU[6] = 0x01;
    //copiar o APDU para para o array do PDU que já contem o cabeçalho MBAP
   memcpy(PDU + 7, APDU, APDUlen);

   if (fd<0){
        printf("Socket creation failed...\n");
		return -1;
   }
   else{
    printf("Socket successfully created...\n");
   }

   server.sin_addr.s_addr = inet_addr(server_add);
   server.sin_family = AF_INET;
   server.sin_port=htons(port);

   if(connect(fd, (struct sockaddr *)&server, sizeof(server))<0){
        printf("Connection creation failed...\n");
        return -1;
   }
   else{
        printf("Connection successfully created...\n");
   }

   int out = write_modbus(fd, PDU, 7 + APDUlen);

   if(out<0){
        printf("Send PDU failed...\n");
        return -1;
   }
   else{
        printf("Sent data (%d bytes): ", out);
        for(int i = 0; i < out; i++){
            printf("%02X", PDU[i]);
        }
        printf("\n");
   }

   bzero(PDU_R, 260);
   int j = 0;
   while(j<7){
        in = read_modbus(fd, PDU_R+j, 1);
        j++;
   }

   uint16_t len_R = (PDU_R[4]<<8) | PDU_R[5]&0xFF;

   while(j<7+len_R){
        in = read_modbus(fd, PDU_R+j, 1);
        j++;
   }

   if(in<0){
    printf("Recv PDU failed..\n");
	return -1;
   }
   else{
    printf("Received data (%d bytes): ", j-1);
    for(int i = 0; i < j-1; i++){
            printf("%02X", PDU_R[i]);
        }
        printf("\n");
   }
    uint16_t respTransactionId = (PDU_R[0] << 8) | PDU_R[1];
    if (respTransactionId != transactionId) {
        printf("ERROR: Transaction ID mismatch! Sent: 0x%04X, Received: 0x%04X\n",
           transactionId, respTransactionId);
    }

   memcpy(APDU_R, PDU_R + 7, len_R-1);
   
   close(fd);

return j-1;
}