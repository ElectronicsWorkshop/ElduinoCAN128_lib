#ifndef lin_h
#define lin_h
#include <avr/io.h>
	#define lin_filter_none	255
	#define lin_expectedLen_NotSet	255
	void LIN_init(uint16_t baud);
	unsigned char LIN_read(unsigned char *ID,unsigned char *buff,unsigned char *len);
	void LIN_ACK(void);
	void LIN_send(unsigned char ID,char *buff,unsigned char buffLen);
	void LIN_set_RX_ID_filter(unsigned char ID);
	void LIN_setExpectedRXlen(unsigned char len);
	void LIN_TXenable(void);
	void LIN_TXdisable(void);
	void LIN_RXenable(void);
	void LIN_RXdisable(void);
#endif