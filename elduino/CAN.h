
#ifndef can_h
#define can_h
#include <avr/io.h>
#include <avr/interrupt.h>
#define MSK_CANGCON_ENA  	0x02
#define MSK_CANGCON_GRES 	0x01
#define RxEMAX            8
#define CH_DISABLE         0x00
#define CH_RxENA           0x80
#define CH_TxENA           0x40
#define MSK_CANGIE_ENRX    0x20
#define MSK_CANGIE_ENTX    0x10
#define MSK_CANSTMOB_RxOk  0x20
#define DLC_MAX 8

//enum CANMode {CONFIGURATION=0,NORMAL=1,SLEEP=2,LISTEN=3,LOOPBACK=4};

class CANClass
{
private:
	
public:
	#define CANIT	0
	static void begin();//sets up MCP2515
	static void baudConfig(int bitRate);//sets up baud

	//Method added to enable testing in loopback mode.(pcruce_at_igpp.ucla.edu)
	//static void setMode(CANMode mode) ;//put CAN controller in one of five modes
	void set(char page,unsigned int CANID);
	void set11(char page,unsigned short CANID, unsigned long CANMASK);
	void set29(char page,unsigned long CANID, unsigned long CANMASK);
	void send29(char *buff,char count,unsigned long CANID);//transmits the 2.0B packet with 29 bit identifier
	void send11(char *buff,char count,unsigned long CANID);//transmits the 2.0A packet with 11 bit identifier
	void read(char page,char *buff,char *count);
	void read(char page,char *buff,char *count,unsigned long *ID);
	void detachInterrupt(int interrupt);
	void attachInterrupt(void (*userFunc)(void), int interrupt);
	
	
	static char readID_0();//read ID/DATA of recieve buffer X


	static char readDATA_0();


	//extending CAN data read to full frames(pcruce_at_igpp.ucla.edu)
	//data_out should be array of 8-bytes or frame length.
	static void readDATA_ff_0(unsigned char* length_out,unsigned char *data_out,unsigned short *id_out);


	//Adding can to read status register(pcruce_at_igpp.ucla.edu)
	//can be used to determine whether a frame was received.
	//(readStatus() & 0x80) == 0x80 means frame in buffer 0
	//(readStatus() & 0x40) == 0x40 means frame in buffer 1
	static unsigned char readStatus();

	static void load_0(unsigned char identifier, unsigned char data);//load transmit buffer X


	//extending CAN write to full frame(pcruce_at_igpp.ucla.edu)
	//Identifier should be a value between 0 and 2^11-1, longer identifiers will be truncated(ie does not support extended frames)
	static void load_ff_0(unsigned char length,unsigned short identifier,unsigned char *data);


};
extern CANClass CAN;
#endif
