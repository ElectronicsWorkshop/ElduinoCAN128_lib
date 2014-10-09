#ifndef lin_h
#define lin_h
	void LIN_init(uint16_t baud);
	void LIN_send(unsigned char ID,char *buff,unsigned char buffLen);
	void LIN_TXenable(void);
#endif