#include "ringbuffer.h"
/**
  * @brief  rbInitialize��ʼ�����ã�����������Ϣ����ṹ��
  * @param  pRingBuff:ringbuffer�ṹ��
	*         buff�����ݻ�����
	*         length����������С
  * @note   
  * @retval void
  * @author Acorss������	
  */	
void rbInit(RingBuffer* pRingBuff, uint8_t* buff, uint16_t length)
{
	pRingBuff->pBuff = buff;
	pRingBuff->pEnd  = buff + length;
	pRingBuff->wp = buff;
	pRingBuff->rp = buff;
	pRingBuff->length = length;
	pRingBuff->flagOverflow = 0;
}

