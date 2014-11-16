#include "LIN.h"

// buffer for received LIN data
char buff[8];
// LIN package byte count
unsigned char len=0;
// LIN package ID
unsigned char id=0;

void setup(){
  //Initialize Serial port, 15200 bps baud rate
  Serial.begin(115200);
  //Initialize LIN with 9600 bps baud rate
  LIN_init(9600);
  //Optional: set ID filter for received packages
  //LIN_set_RX_ID_filter(0x22);
}

// temporary buffer for constructing strings to print
char str[50];
// result for LIN read data function
unsigned char res=0;

void loop(){

  //check if LIN data is available
  res=LIN_read(&id,(unsigned char*)buff,&len);
  if (res!=0)
  {
    //construct string with packet ID and lenght
    snprintf(str,50,"ID:%d LEN:%d DATA:",id,len);
    //send constructed string to serial port
    Serial.print(str);
    //Send data bytes to serial port
    for (int i=0;i<len;i++){
      Serial.print(buff[i],16);
      Serial.print(" ");
    }
    //Send new line to serial port
    Serial.println("");
  }
  
  //wait 100ms
  delay(100);
}
