#ifndef MODBUS_AP_H_
#define MODBUS_AP_H_
#define RHR 0x03
#define WMR 0x10

int Read_h_regs (char* server_add, int port, uint32_t st_r, uint16_t n_r, uint16_t* val);

int Write_multiple_regs (char* server_add, int port, uint32_t st_r, uint16_t n_r, uint16_t* val);

#endif 