
#include <avr/io.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include "LIN.h"
unsigned short LINbaud=10400;
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
void LIN_init(uint16_t baud){
	//set TX pin as output
	DDRD=1<<PD3;
	//enable pull-up on the RX pin
	PORTD|=1<<PD2;
	//set the transmit enable pin to output
	DDRG|=1<<PG3;
	LINbaud=baud;
	USART1_Init(LINbaud);
}

void LIN_generateBreakField(void){
	USART1_Init((unsigned long)LINbaud*9/13);
	USART1_Transmit(0x00);
	USART1_Init(LINbaud);
}
void LIN_send(unsigned char ID,char *buff,unsigned char buffLen){
	unsigned char ID_p0=0,ID_p1=0;
	//TODO: create the ID parity calculation
	ID_p0=(((ID>>0)&1)|((ID>>1)&1)|((ID>>2)&1)|((ID>>4)&1))<<6;
	ID_p1=(~(((ID>>1)&1)|((ID>>3)&1)|((ID>>4)&1)|((ID>>5)&1)))<<7;
	//send BREAK field
	LIN_generateBreakField();
	//send SYNC field
	USART1_Transmit(0x55);
	//send protected identifier field
	USART1_Transmit((ID&0x3F)|ID_p0|ID_p1);
	//send data
	for (unsigned char i=0;i<buffLen;i++)
		USART1_Transmit(buff[i]);
}

void LIN_TXenable(void){
	PORTG|=1<<PG3;
}