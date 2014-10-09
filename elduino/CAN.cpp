#include <CAN.h>

CANClass CAN;//create can object

typedef void (*voidFuncPtr)(void);
static volatile voidFuncPtr intFunc[10];

ISR (CANIT_vect)
{
	if(intFunc[CANIT])
		intFunc[CANIT]();
}

void CANClass::attachInterrupt(void (*userFunc)(void), int interrupt) {
    intFunc[interrupt] = userFunc;
	//enable rx and global interrupts
	CANGIE=((1<<ENRX)|(1<<ENIT));
	//enable Mob0 interrupt
	CANIE2=(1<<IEMOB0);
	// disable MOB8-14 interrupts
	CANIE1=0x00;
	// Highest Interrupt Priority: MOb0
	CANHPMOB=0x00;
}

void CANClass::detachInterrupt(int interrupt) {

}


void CANClass::begin()//constructor for initializing can module.
{
	//enable rx and global interrupts
	//CANGIE=((1<<ENRX)|(1<<ENIT));
	//enable Mob0 interrupt
	CANIE2=0x00;
	// disable MOB8-14 interrupts
	CANIE1=0x00;
	// Highest Interrupt Priority: MOb0
	CANHPMOB=0x00;
	//reset can baud rate
	CANBT1=0;
	CANBT2=0;
	CANBT3=0;

	CANTCON=0x00;

	for (int num_channel = 0; num_channel < 15; num_channel++)
	{
		CANPAGE  = num_channel << 4;
		CANCDMOB = 0;
		CANSTMOB  = 0;
		CANIDT1  = 0;
		CANIDT2  = 0;
		CANIDT3  = 0;
		CANIDT4  = 0;
		CANIDM1  = 0xff;
		CANIDM2  = 0xff;
		CANIDM3  = 0xff;
		CANIDM4  = 0xff;
		for (int num_data = 0; num_data < 8; num_data++) CANMSG = 0;
	}

	// CAN: On
	CANGCON=0x02;
	   while(ENFG==0) ;   // waite until CAN bus being enabled 
}

void CANClass::baudConfig(int bitRate)//sets bitrate for CAN node
{
	switch (bitRate)
	{
		/*case 10:
		
		break;
		case 20:
		
		break;
		case 50:
		
		break;*/
		case 100:
			CANBT1=0x12;
			CANBT2=0x0c;
			CANBT3=0x37;
		break;
		case 125:
			CANBT1=0x0E;
			CANBT2=0x0c;
			CANBT3=0x37;
		break;
		case 250:
			CANBT1=0x06;
			CANBT2=0x0c;
			CANBT3=0x37;
		break;
		case 500:
			CANBT1=0x02;
			CANBT2=0x0c;
			CANBT3=0x37;
		break;
		case 1000:
			CANBT1=0x00;
			CANBT2=0x0c;
			CANBT3=0x36;
		break;		
		default: //500kbps
			CANBT1=0x02;
			CANBT2=0x0c;
			CANBT3=0x37;
		break;
	}
}
//----------------------- not esential -------------------------
//Method added to enable testing in loopback mode.(pcruce_at_igpp.ucla.edu)
/*void CANClass::setMode(CANMode mode) { //put CAN controller in one of five modes
switch(mode) {
  	case CONFIGURATION:

			break;
  	case NORMAL:

			break;
  	case SLEEP:

	  	break;
    case LISTEN:

	  	break;
  	case LOOPBACK:

	  	break;
   }

}*/

void CANClass::set(char page,unsigned int CANID){
	CANPAGE = (page<<4) ;    // select MOb1
	CANIDM1 = 0xFF;     
	CANIDM2 = 0xFF;
	CANIDM3 = 0xFF;
	CANIDM4 = 0xFF;
	CANIDT1 = (CANID>>3);
	CANIDT2 = (CANID<<5)&0xFF;
	CANIDT3 = 0x00;
	CANIDT4 = 0x00;
	CANEN2 |= (2 << 0);
	CANSTMOB=0x00;
	CANCDMOB = CH_RxENA ;   // enable reception
}

void CANClass::set11(char page,unsigned short CANID, unsigned long CANMASK){
	CANPAGE = (page<<4) ;    // select MOb1
	CANIDM1 = CANMASK;     
	CANIDM2 = CANMASK>>8;
	CANIDM3 = CANMASK>>16;
	CANIDM4 = CANMASK>>24;
	CANIDT1 = (CANID>>3);
	CANIDT2 = (CANID<<5)&0xFF;
	CANIDT3 = 0x00;
	CANIDT4 = 0x00;
	CANEN2 |= (2 << 0);
	CANSTMOB=0x00;
	CANCDMOB = CH_RxENA ;   // enable reception
}

void CANClass::set29(char page,unsigned long CANID, unsigned long CANMASK){
	CANPAGE = (page<<4) ;    // select MOb1
	CANIDM1 = CANMASK;     
	CANIDM2 = CANMASK>>8;
	CANIDM3 = CANMASK>>16;
	CANIDM4 = CANMASK>>24;
	CANIDT4=(CANID<<3);
	CANIDT3=(CANID>>5);
    CANIDT2 =(CANID>>13);
    CANIDT1 =(CANID>>21);
	CANEN2 |= (2 << 0);
	CANSTMOB=0x00;
	CANCDMOB = CH_RxENA ;   // enable reception
}

void CANClass::send29(char *buff,char count,unsigned long CANID)//transmits the 2.0B packet with 29 bit identifier
{
	unsigned short i=0;
    CANPAGE = 13<<4 ;  // select MOb0
	CANIDT4=(CANID<<3);
	CANIDT3=(CANID>>5);
    CANIDT2 =(CANID>>13);
    CANIDT1 =(CANID>>21);
	for (i=0;i<count;i++)
		CANMSG = buff[i];
    CANCDMOB = 0x40|count|(1<<IDE);   // enable transmission, data length=count
	while(((CANSTMOB & 0x40 ) != 0x40)&&(i++<500));   // if the TXOK flag has been set
    CANSTMOB &= 0xBF ;           // clear TXOK flag
    CANCDMOB = 0x00;  
}

void CANClass::send11(char *buff,char count,unsigned long CANID)//transmits the 2.0A packet with 11 bit identifier
{
	unsigned short i=0;
	// select MOb0 13
    CANPAGE = 13<<4 ;  
	CANIDT1=(CANID>>3);
	CANIDT2=(CANID<<5);
    CANIDT3 = 0x00 ;
    CANIDT4 = 0x00 ;
	for (i=0;i<count;i++)
		CANMSG = buff[i];
    CANCDMOB = 0x40|count;   // enable transmission, data length=count
	while(((CANSTMOB & 0x40 ) != 0x40)&&(i++<500));   // if the TXOK flag has been set
    CANSTMOB &= 0xBF ;           // clear TXOK flag
    CANCDMOB = 0x00;  
}

void CANClass::read(char page,char *buff,char *count)
{
	char msglen=-1;
	unsigned char a1;
	CANPAGE = ((page) << 4);
	if((CANSTMOB & MSK_CANSTMOB_RxOk) == MSK_CANSTMOB_RxOk)
	{
		msglen=CANCDMOB&15;
		for (int i=0;i<msglen;i++)
		{
			a1=CANMSG;
			buff[i]=a1;
		}
		CANSTMOB&=~MSK_CANSTMOB_RxOk;
	}
	CANCDMOB = CH_RxENA;  
	*count=msglen;
}			

void CANClass::read(char page,char *buff,char *count,unsigned long *ID)
{
	unsigned char msglen=0,a1;
	CANPAGE = ((page) << 4);
	
	if((CANSTMOB & MSK_CANSTMOB_RxOk) == MSK_CANSTMOB_RxOk)
	{
		if ((CANCDMOB&(1<<IDE))==0){
			*ID=(((unsigned long)CANIDT1<<3) | ((unsigned long)CANIDT2>>5))&0x7FF;
		}
		else{
			*ID=(((unsigned long)CANIDT4>>3) | ((unsigned long)CANIDT3<<5) | ((unsigned long)CANIDT2<<13) |((unsigned long)CANIDT1<<21))&0x1FFFFFFF;
		}
		msglen=CANCDMOB&15;
		for (int i=0;i<msglen;i++)
		{
			a1=CANMSG;
			buff[i]=a1;
		}
		CANSTMOB&=~MSK_CANSTMOB_RxOk;
	}
	CANCDMOB = CH_RxENA;  
	*count=msglen;
}			

char CANClass::readID_0()//reads ID in recieve buffer 0
{

}


	//extending CAN data read to full frames(pcruce_at_igpp.ucla.edu)
	//It is the responsibility of the user to allocate memory for output.
	//If you don't know what length the bus frames will be, data_out should be 8-bytes
void CANClass::readDATA_ff_0(unsigned char* length_out,unsigned char *data_out,unsigned short *id_out){
	
}

	//Adding method to read status register
	//can be used to determine whether a frame was received.
	//(readStatus() & 0x80) == 0x80 means frame in buffer 0
	//(readStatus() & 0x40) == 0x40 means frame in buffer 1
unsigned char CANClass::readStatus() 
{


}

void CANClass::load_0(unsigned char identifier, unsigned char data)//loads ID and DATA into transmit buffer 0
{

}

void CANClass::load_ff_0(unsigned char length,unsigned short identifier,unsigned char *data)
{

}

