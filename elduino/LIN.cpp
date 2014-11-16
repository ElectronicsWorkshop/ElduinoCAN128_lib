
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "LIN.h"
#include <string.h>
unsigned short LINbaud=10400;
unsigned char LIN_calculateChecksum(unsigned char *buff,unsigned char len,unsigned char ProtectedID);
extern void LIN_received(void);//void LIN_received(unsigned char PID,unsigned char *buff,unsigned char len);

void USART1_Init (unsigned int baud)
{
	unsigned short ubrr_val=0;
	UCSR1A|=1<<U2X1;
	/* Set baud rate */
	ubrr_val=(F_CPU/8/baud-1)&0xFFF;
	UBRR1H = (unsigned char) (ubrr_val>>8);
	UBRR1L = (unsigned char) ubrr_val;
	/*UBRR1H = (unsigned char) (baud>>8);
	UBRR1L = (unsigned char) baud;*/
	/* Set frame format: 8data, no parity & 1 stop bits */
	UCSR1C = (0<<UMSEL1) | (0<<UPM1) | (0<<USBS1) | (1<<UCSZ10) | (1<<UCSZ11);
	/* Enable receiver and transmitter */
	UCSR1B = (1<<RXEN1) | (1<<TXEN1) ;//| (0<<UCSZ2);
	//stdout = &uart_str; // printf to UART
	UCSR1A |=(1<<UDRE1);
	
	
}

//---------------------------------------------------------------------------
int USART1_Transmit (unsigned char data)
{
	/* Put data into buffer, sends the data */
	UDR1 = data;
	/* Wait for empty transmit buffer */
	//while ( ! ( UCSR1A & (1<<UDRE1)));
	while ( ! ( UCSR1A & (1<<TXC1)));
	UCSR1A|=(1<<TXC1);
	return 0;
}

enum lin_states {idle,start,sync,protectedID,get_payload};
lin_states lin_state=idle;

unsigned char lin_msg_len=0;
unsigned char lin_buffer[9];
unsigned char lin_PID=0;
unsigned char lin_expected_len=8;
unsigned char lin_filter=lin_filter_none;

unsigned char lin_data_available=0;

unsigned char LIN_read(unsigned char *ID,unsigned char *buff,unsigned char *len){
	if (lin_data_available==0)
		return 0;
	if ((lin_msg_len>9)||(lin_msg_len<=0)){
		lin_data_available=0;
		return 0;
	}
	memcpy(buff,lin_buffer,lin_msg_len-1);
	*len=lin_msg_len-1;
	*ID=lin_PID;
	lin_data_available=0;
	return 1;
}
void LIN_setExpectedRXlen(unsigned char len){
lin_expected_len=len;
}

void LIN_ACK(void){
	lin_data_available=0;
}

//void LIN_received(unsigned char PID,unsigned char *buff,unsigned char len) __attribute__((weak));
//void LIN_received(unsigned char PID,unsigned char *buff,unsigned char len){}

void LIN_received(void) __attribute__((weak));
void LIN_received(void){}

void usart1Rcvd(unsigned char c,unsigned char fl){
	if (lin_data_available!=0)
		return;
  //frame error detected - BREAK field has been received
  if ((fl&(1<<FE1))!=0){
  
    if ((lin_state==get_payload)&& \
		((lin_PID==lin_filter)||(lin_filter==lin_filter_none)))
	{
	 lin_data_available=1;
     //LIN_received(lin_PID,lin_buffer,lin_msg_len-1); 
	 LIN_received();
    }
    if (c==0){
      lin_state=start;
     // lin_msg_len=0;
    }
    else{
      lin_state=idle;
     // lin_msg_len=0;
    }
    return;
  }
  switch(lin_state){
   case start:
     //check if sync field is received correctly
     if (c!=0x55){
       //there was an error. Return to idle state
      lin_state=idle;
      return;
     }
	 lin_msg_len=0;
    #ifdef _LIN_DEBUG 
     Serial.print("s:");
     Serial.println((int)c);
    #endif
     lin_state=sync;
   break; 
   case sync:
   #ifdef _LIN_DEBUG 
     Serial.print("P:");
     Serial.println((int)c);
     #endif
     lin_PID=c&0x3F;
     lin_state=protectedID;
   break; 
    case protectedID:
    case get_payload:
      //add data to LIN receive buffer
      lin_buffer[lin_msg_len++]=c;
      lin_state=get_payload;
      #ifdef _LIN_DEBUG 
      Serial.print("d:");
      Serial.println((int)(unsigned char)c);
      #endif
      if ((lin_msg_len==lin_expected_len+1)&& \
			(lin_expected_len<9))
	  {
		if ((lin_PID==lin_filter)||(lin_filter==lin_filter_none)){
			lin_data_available=1;
			//LIN_received(lin_PID,lin_buffer,lin_msg_len-1); 
			LIN_received();
		}
       lin_state=idle;
      }
    break;
   default:
   #ifdef _LIN_DEBUG 
   Serial.print("?:");
   Serial.println((int)(unsigned char)c);
   #endif
   break;
  }
  
}

void LIN_init(uint16_t baud){
	//set TX pin as output
	DDRD=1<<PD3;
	//enable pull-up on the RX pin
	PORTD|=1<<PD2;
	//set the transmit enable pin to output
	DDRG|=1<<PG3;
	LINbaud=baud;
	USART1_Init(LINbaud);
	LIN_RXenable();
}

void LIN_RXenable(void){
	//enable usart RX
	UCSR1B|= (1<<RXEN1);
	//enable usart RX interrupt
	UCSR1B|=(1<<RXCIE1);
}

void LIN_RXdisable(void){
	//enable usart RX
	UCSR1B &= ~(1<<RXEN1);
	//enable usart RX interrupt
	UCSR1B&=~(1<<RXCIE1);
}

void LIN_generateBreakField(void){
	USART1_Init((unsigned long)LINbaud*9/13);
	USART1_Transmit(0x00);
	USART1_Init(LINbaud);
}

void LIN_send(unsigned char ID,char *buff,unsigned char buffLen){
	unsigned char ID_p0=0,ID_p1=0,chk=0,prID=0;
	ID_p0=(((ID>>0)&1)^((ID>>1)&1)^((ID>>2)&1)^((ID>>4)&1))<<6;
	ID_p1=(~(((ID>>1)&1)^((ID>>3)&1)^((ID>>4)&1)^((ID>>5)&1)))<<7;
	prID=(ID&0x3F)|ID_p0|ID_p1;
	//disable receiver
	UCSR1B &=~(1<<RXEN1);
	//send BREAK field
	LIN_generateBreakField();
	//send SYNC field
	USART1_Transmit(0x55);
	//send protected identifier field
	USART1_Transmit(prID);//(ID&0x3F)|ID_p0|ID_p1);
	//send data
	for (unsigned char i=0;i<buffLen;i++)
		USART1_Transmit(buff[i]);
	chk=LIN_calculateChecksum((unsigned char*)buff,buffLen,prID);
	USART1_Transmit(chk);
	//enable receiver
	UCSR1B |= (1<<RXEN1);
}

unsigned char LIN_calculateChecksum(unsigned char *buff,unsigned char len,unsigned char ProtectedID){
	unsigned short chk=0;
	chk=ProtectedID;
	for (char i=0;i<len;i++){
		chk+=buff[i];
		if ((chk&(1<<8))>0)
			chk++;
		chk&=0xff;
	}
	chk=~(chk);
	chk&=0xFF;
	return chk;
}

void LIN_TXenable(void){
	PORTG|=1<<PG3;
}
void LIN_TXdisable(void){
	PORTG&=~(1<<PG3);
}
void LIN_set_RX_ID_filter(unsigned char ID){
	lin_filter=ID;
}