#include <stdio.h>
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>	
#include <unistd.h>
#include "modbusTCP.h"
#include "modbusAP.h"
#include <errno.h>
#include <sys/time.h>

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
    struct timeval timeout;
    timeout.tv_sec = 5;    
    timeout.tv_usec = 0; 
   int fd = socket(PF_INET , SOCK_STREAM , IPPROTO_TCP);
   if (setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        //printf("Error: Failed to set receive timeout\n");
        close(fd);
        return -1;
    }
   int in = 0;
   int i = 0;
   struct sockaddr_in server;

   uint8_t PDU[7+APDUlen], PDU_R[260];

   uint16_t transactionId = get_transaction_id();

   uint16_t len = APDUlen + 1;


   PDU[i++] = (transactionId >> 8) & 0xFF;
   PDU[i++] = transactionId & 0xFF;
   PDU[i++] = 0x00;
   PDU[i++] = 0x00;
   PDU[i++] = (len >> 8) & 0xFF;
   PDU[i++] = len & 0xFF;
   PDU[i++] = 0x01;

   memcpy(PDU + 7, APDU, APDUlen);

   if (fd<0){
        //printf("Socket creation failed...\n");
        close(fd);
		return -1;
   }
   else{
    //printf("Socket successfully created...\n");
   }

   server.sin_addr.s_addr = inet_addr(server_add);
   server.sin_family = AF_INET;
   server.sin_port=htons(port);


   if(connect(fd, (struct sockaddr *)&server, sizeof(server))<0){
        //printf("Connection creation failed...\n");
        close(fd);
        return -1;
   }
   else{
        //printf("Connection successfully created...\n");
   }

   int out = write_modbus(fd, PDU, 7 + APDUlen);

   if(out<0){
        //printf("Send PDU failed...\n");
        close(fd);
        return -1;
   }
   //else{
       //printf("Sent data (%d bytes): ", out);
        //for(int i = 0; i < out; i++){
            //printf("%02X \t", PDU[i]);
        //}
        //printf("\n");
   //}

   bzero(PDU_R, 260);

   in = read_modbus(fd, PDU_R, 7);
   if (errno == EWOULDBLOCK || errno == EAGAIN){
        //printf("ERRO: Timeout");
        close(fd);
        return -1;
   } 
   if(in<0){
        //printf("Recv PDU failed..\n");
        close(fd);
	    return -1;
    }

   uint16_t len_R = (PDU_R[4]<<8) | PDU_R[5]&0xFF;  
                                           
   in = read_modbus(fd, PDU_R + 7, (int)(len_R) -1);
   if(in<0){
        //printf("Recv PDU failed..\n");
        close(fd);
	    return -1;
    }


    //printf("Received data (%d bytes): ", 6+len_R);
    
    for(int i = 0; i < 6+len_R; i++){
            printf("%02X \t", PDU_R[i]);
        }
        printf("\n");

    uint16_t respTransactionId = (PDU_R[0] << 8) | PDU_R[1];
    if (respTransactionId != transactionId) {
        //printf("ERROR: Transaction ID mismatch! Sent: 0x%04X, Received: 0x%04X\n",
           //transactionId, respTransactionId);
           close(fd);
           return -1;
    }

   memcpy(APDU_R, PDU_R + 7, len_R-1);
   
   close(fd);

return len_R-1;
}