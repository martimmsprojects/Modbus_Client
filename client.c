#include <stdio.h>
#include <stdint.h>
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "modbusTCP.h"
#include "modbusAP.h"

int main() {
    char server_addr[64];
    int port, val1, n_r1;
    long st_r1;
    char c;
    uint16_t n_r;
    uint32_t st_r;

    printf("Write the server's address: ");
    scanf("%63s", server_addr);

    printf("Write the port you want to use: ");
    scanf("%d", &port);
    printf("Write (w) or Read (r): ");
    scanf(" %c", &c);  

    printf("Register start address : ");
    scanf("%ld", &st_r1);

    printf("How many registers: ");
    scanf("%d", &n_r1);

    n_r = (uint16_t)(n_r1);
    st_r = (uint32_t)(st_r1);

    uint16_t val[125];

    if (c == 'w') {
        for (int i = 0; i < n_r; i++) {
            printf("Register %ld value : ", (long)(st_r + i));
            scanf("%d", &val1);
            val[i] = val1;
        }

        int result = Write_multiple_regs(server_addr, port, st_r, n_r, val);

        if(result > 0){
            printf("Write %d registers:\n", result);
        }
        else{
            printf("Erro Writting");
            return result;
        }
    } 
    else if (c == 'r') {
        int result = Read_h_regs(server_addr, port, st_r, n_r, val);

        if (result > 0) {
            printf("Read %d registers:\n", result);
            for (int i = 0; i < result; i++) {
                printf("Reg[%ld] = 0x%04X (%u)\n", (long)(st_r + i), val[i], val[i]);
            }
        }

        else{
            printf("Erro Reading");
            return result;
        }
    } 

    else {
        printf("Invalid option. Use 'w' or 'r'.\n");
    }

    return 0;
}
