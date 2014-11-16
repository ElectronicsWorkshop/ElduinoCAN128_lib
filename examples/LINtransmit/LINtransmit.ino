#include "LIN.h"

void setup(){
  // Initialize LIN with 9600 bps baud rate
  LIN_init(9600);
  // Enable transmission on LIN bus
  LIN_TXenable();
}

// Buffer for sending data to LIN bus
char buff[8]={0x4a,0x55,0x93,0xe5,0x00,0x00,0x00,0x00};

void loop(){
  // Send LIN package with 8 bytes of data, ID=0x22
  // Data is sent from buffer buff
  LIN_send(0x22,buff,8);
  // Wait 1 s
  delay(1000);
}
