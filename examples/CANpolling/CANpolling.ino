/*
  CAN polling example
  
  A simple example of setting up, receiving and sending data through CAN bus. 
  
  Elduino CAN128 board has an autonomous CAN module that can read, filter and 
  store CAN messages without intervention of the microcontroller itself.
  The CAN bus module works as follows: when new CAN message arrives, it is stored
  in a mailbox. There are total of 14 mail boxes. All mailboxes can have separate 
  CAN ID filters so that only packets with certain ID can reach it. When data arrives,
  it is stored to the mailbox and kept there until it is read(new data does not override 
  the old one). Having more than one mailbox helps making sure that no critical data is lost.
  In case two desired packages come one after another in a very short time and normally you
  would not be able to process them both because of microcontroller limitations, you can route 
  the packages to different mailboxes and read them consecutively.

  created 2014
  by Vaidas Zakarka
*/
#include "CAN.h"
char CAN_buff[8];
char CAN_msgLen=0;
unsigned long CAN_id=0;

void setup(){
 //Initialize CAN bus library
 CAN.begin();
 //Set CAN baud to 500 bps
 CAN.baudConfig(500);
 //Set 11-bit CAN ID filter for mailbox 0
 CAN.set11(0,0x00AB,0xFFFFFF);
}

void loop(){
  //Try to read the received data from mailbox 0
  CAN.read(0,CAN_buff,&CAN_msgLen,&CAN_id);
  //Was there any data?
  if (CAN_msgLen>=0){
    //Send the same data with incremented CAN ID
    CAN.send11(CAN_buff,CAN_msgLen,CAN_id+1);
  }
}
