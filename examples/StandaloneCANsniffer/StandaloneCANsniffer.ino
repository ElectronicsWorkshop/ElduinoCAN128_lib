/*
  Standalone CAN sniffer
  
  An example of CAN bus sniffer for Elduino CAN128 board. The board 
  is receiving data from the CAN bus and stores it in RAM. Stored data
  is sent out through serial communication and displayed on the LCD.
  Since LCD is too small to display all packets, there is a menu to
  browse between them.
  
  This example requires LCD-button shield if you want to see the data
  without computer.

  created 2014
  by Vaidas Zakarka
*/

#include <LiquidCrystal.h>
#include "CAN.h"

// select the pins used on the LCD panel
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
int lcd_key     = 0;
int adc_key_in  = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// read the buttons
int read_LCD_buttons()
{
 adc_key_in = analogRead(0);      // read the value from the sensor 
 if (adc_key_in > 1000) return btnNONE; // We make this the 1st option for speed reasons since it will be the most likely result
 // For V1.1 us this threshold
 if (adc_key_in < 50)   return btnRIGHT;  
 if (adc_key_in < 250)  return btnUP; 
 if (adc_key_in < 450)  return btnDOWN; 
 if (adc_key_in < 650)  return btnLEFT; 
 if (adc_key_in < 800)  return btnSELECT;  
 return btnNONE;  // when all others fail, return this...
}
int led = 13;

struct {
    unsigned long ID;
    unsigned char lastData[8];
    unsigned char lastMSGlen;
    unsigned short count;
    unsigned short timeDelay;
    unsigned short lastTime;
} typedef CANmsgData;
static CANmsgData dta[20],dta1[20];
static unsigned char dtaLen=0,dtaLen1=0;
unsigned char meniu_mode=0;
unsigned short bd=125;
void setup()
{
  
 pinMode(led, OUTPUT); 
 lcd.begin(16, 2);              // start the library
 pinMode(10,OUTPUT);
 analogWrite(10, 100);
 lcd.setCursor(0,0);
 lcd.print("                "); // print a simple message
 lcd.setCursor(0,1);
 lcd.print("                "); // print a simple message
 bd= menu_CAN_settings();
 lcd.setCursor(0,0);
 lcd.print("                "); // print a simple message
 lcd.setCursor(0,1);
 lcd.print("                "); // print a simple message
 memset(dta,0,sizeof(dta));
 memset(dta1,0,sizeof(dta1));
 CAN.begin();
 CAN.baudConfig(125);
 CAN.set11(0,0x00AB,0x000000);
 //lcd.setCursor(0,1);
 lcd.print(bd);
 //while(1);
 CAN.baudConfig(bd);
 CAN.attachInterrupt(CAN_irq,0);
 
 Serial.begin(115200); 
 DDRC|=(1<<PC6)|(1<<PC7);
}


  
void CAN_irq(void){
  char buff[8]={1,2,3,4,5,6,7,8};
  char msglen=0;
  unsigned long id=0;
  char i=0;
  PORTC|=1<<PC6;
  PORTB^=1<<PB1;
  CAN.read(0,buff,&msglen,&id);
  if (msglen<0)
    return;
  for (i=0;i<dtaLen;i++){
    if (dta[i].ID==id){
      memcpy(dta[i].lastData,buff,msglen);
      dta[i].lastMSGlen=msglen;
      
      if (dta[i].count!=0)
        dta[i].timeDelay=((unsigned short)millis()&0xFFFF)-(unsigned short)dta[i].lastTime;
      else
        dta[i].timeDelay=0;
      dta[i].lastTime=millis()&0xFFFF;
      dta[i].count++;
      break;
    }
  }
  if (i==dtaLen){
    if (dtaLen<20){
      dta[i].ID=id;
      memcpy(dta[i].lastData,buff,msglen);
      dta[i].lastMSGlen=msglen;
     if (dta[i].count!=0)
        dta[i].timeDelay=((unsigned short)millis()&0xFFFF)-(unsigned short)dta[i].lastTime;
      else
        dta[i].timeDelay=0;
      dta[i].lastTime=millis()&0xFFFF;
      dtaLen++;
    }
  }
    PORTC&=~(1<<PC6);
}
void printHex(int num, int precision) {
      char tmp[16];
      char format[128];
      sprintf(format, "%%.%dX", precision);
      sprintf(tmp, format, num);
      lcd.print(tmp);
}
void printIds(void){
  char str[40];
  unsigned char str_len=0;
 /* Serial.write(27);       // ESC command
  Serial.print("[2J");    // clear screen command*/
  Serial.write(27);
  Serial.print("[H");     // cursor to home command
  Serial.print("Baud rate:");
  Serial.println(bd);
  
  Serial.println("    ID  |          DATA         |COUNT|TIME,ms|");// tLST |");
  Serial.println("xxxxxxxx|D0 D1 D2 D3 D4 D5 D6 D7|xxxxx|xxxxxxx|");// tLST |");
  for (int i=0;i<dtaLen1;i++){
      str_len=snprintf(str,40,"%08X|",dta1[i].ID);
      Serial.print(str);
      for (int n=0;n<dta1[i].lastMSGlen;n++){
        str_len=snprintf(str,40,"%02X",dta1[i].lastData[n]);
        Serial.print(str);
        if (n!=(dta1[i].lastMSGlen-1))
         Serial.print(" ");
      }
      for (int k=0;(k<8-dta1[i].lastMSGlen);k++)
        Serial.print("   ");
             //   if (n==(dta1[i].lastMSGlen-1))
         Serial.print("|");
      str_len=snprintf(str,40,"%05d| %06d|%06d|\n\r",dta1[i].count,dta1[i].timeDelay,dta1[i].lastTime);
      str_len=snprintf(str,40,"%05d| %06d|\n\r",dta1[i].count,dta1[i].timeDelay);
        Serial.print(str);
     // break;
   }
   Serial.println("");
}

void menu_messages(void){
  static char menuItem=0;
  static char displayItem=0;
  static char messageNo=0;
  lcd.setCursor(0,0);
  lcd.print(dta1[messageNo].ID,HEX);
  
  lcd.setCursor(10,0);
  if (displayItem==0)
    lcd.print((dta1[messageNo].count%0xFF));
  else
    lcd.print((dta1[messageNo].timeDelay));
  
  
  lcd.setCursor(14,0);
  lcd.print(messageNo,HEX);
  
  lcd.setCursor(0,1);
  for (int i=0;i<dta1[messageNo].lastMSGlen;i++){
    printHex(dta1[messageNo].lastData[i],2);
  }
   
  lcd_key = read_LCD_buttons();  // read the buttons
  if (lcd_key==btnLEFT){
    if (menuItem>0)
      menuItem--;
      lcd.setCursor(9,0); 
     lcd.write(' ');
     
     lcd.setCursor(13,0);
     lcd.write(' ');      
  } 
  if (lcd_key==btnRIGHT){
    if (menuItem<1)
      menuItem++;
      lcd.setCursor(9,0); 
     lcd.write(' ');
     
     lcd.setCursor(13,0);
     lcd.write(' ');
  } 
  if (menuItem==0)
    lcd.setCursor(9,0);
  else
    lcd.setCursor(13,0);
  lcd.write('*');
  
  if (menuItem==0){
    if (lcd_key==btnUP){
      if (displayItem<1)
        displayItem++;
      lcd.clear();
    } 
    
    if (lcd_key==btnDOWN){
      if (displayItem>0){
        displayItem--;
        lcd.clear();
      }
    } 
  }
  
  if (menuItem==1){
    if (lcd_key==btnUP){
      if (messageNo<(dtaLen1-1)){
        messageNo++;
        lcd.clear();
      }
    } 
    
    if (lcd_key==btnDOWN){
      if (messageNo>0)
        messageNo--;
      lcd.clear();
    } 
  } 
}

unsigned short menu_CAN_settings(void){
  unsigned char can_baud_index=0;
  unsigned short can_baudrates[]={100,125,250,500,1000};

 // lcd_key=btnNONE;
 lcd_key = read_LCD_buttons();  // read the buttons 
  while(lcd_key!=btnRIGHT){
    lcd_key = read_LCD_buttons();  // read the buttons 
    if (lcd_key==btnUP){
      if (can_baud_index<((sizeof(can_baudrates)/sizeof(unsigned short))-1)){
        can_baud_index++;
        lcd.clear();
      }
    } 
    
    if (lcd_key==btnDOWN){
      if (can_baud_index>0){
        can_baud_index--;
        lcd.clear();
      }
    } 
    lcd.setCursor(0,0);
    lcd.print("BAUD rate:");
    lcd.setCursor(10,0);
    lcd.print((int)can_baudrates[can_baud_index]);
    
    lcd.setCursor(0,1);
    lcd.print((int)can_baud_index);
    
    
    while(lcd_key != btnNONE){
      lcd_key = read_LCD_buttons();  // read the buttons
      if (lcd_key == btnRIGHT)
        return can_baudrates[can_baud_index];
    }
  }
   while(lcd_key != btnNONE)
     lcd_key = read_LCD_buttons();
     delay(1000);
     
  while(lcd_key != btnNONE)
     lcd_key = read_LCD_buttons();
     lcd_key=btnNONE;
  return can_baudrates[can_baud_index];
  
//  lcd.print(dta1[messageNo].ID,HEX);
  
}

char ReceivedByte; 
unsigned char rsbuff[40],UART_index=0;
void loop()
{
  char buff[8]={1,2,3,4,5,6,7,8};
  char msglen=0;
  unsigned long id=0;
  //global disable interrupts
  cli();
  PORTC|=1<<PC7;
  //copy received packets data
  memcpy(dta1,dta,sizeof(dta1));
  //copy received packets data length
  dtaLen1=dtaLen;
  PORTC&=~(1<<PC7);
  //global enable interrupts
  sei();
  
  //print the received packets through serial communication
  printIds();

  //show/control the menu
  menu_messages();
  
    for (int i=0;i<UART_index;i++){
      Serial.print((char)rsbuff[i]);
    }
    
    while (Serial.available()>0){
	ReceivedByte = Serial.read(); 
        Serial.print((char)ReceivedByte);
	uart_append(ReceivedByte,0);
    }
  // wait for a second 
  delay(100);               
}

void uart_append(char byte,char buff){
	static unsigned char n=0;
	if ((n<40)&&(byte>=32)&&(byte<=127))
		rsbuff[n++]=byte;  
        if ((n==0)&&(byte==13))
          n=UART_index;
	if ((byte==13)&&(n>0))
	{
		UART_index=n;
		receive_uart();
                Serial.write(27);       // ESC command
                Serial.print("[2J");    // clear screen command*/
		n=0;
	}
}



void receive_uart(void){
 /* Serial.println("");
  for (int i=0;i<UART_index;i++)
    Serial.print((char)rsbuff[i]);
  Serial.println("");*/
  unsigned char st[150];
  unsigned short id_val;
 // unsigned short B,B1,B2;
  unsigned char n,baitai[9],a1,a2,err=0;
  if ((memcmp(rsbuff,"T",sizeof("T")-1)==0))
  {
    err=0;
    if(sscanf((const char*)rsbuff,"T,%03x,%1d",&id_val,&n,&a2)!=2)
      err=1;
    if (n==0)
      err=1;
    for (int i=0;i<n;i++)
    {
      if (sscanf((const char*)&rsbuff[7+i*3],",%02x",&baitai[i])!=1)
      {
        err=1;
	break;
      }
    }

    if (err==0)
    {
	/*printf("TOK,%d,%d",id_val,n);
	for (int i=0;i<n;i++)
          printf(",%02x",baitai[i]);
	printf("\n");*/
        Serial.println("sent");
	//can_tx(&baitai,n,id_val);
        CAN.send11((char*)baitai,8,id_val);
    }
    else
      Serial.println("ERR:1");
  }
}
