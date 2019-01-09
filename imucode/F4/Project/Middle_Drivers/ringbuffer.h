#ifndef __RINGBUFFER_H
#define __RINGBUFFER_H

#ifdef __cplusplus
 extern "C" {
#endif 

#include "stm32f4xx_hal.h"
	 
#include <stdio.h>

typedef struct {
	uint8_t* pBuff;
	uint8_t* pEnd;  // pBuff + legnth
	uint8_t* wp;    // Write Point
	uint8_t* rp;    // Read Point
	uint16_t length;
	uint8_t  flagOverflow; // set when buffer overflowed
} RingBuffer;


void rbInit(RingBuffer* pRingBuff, uint8_t* buff, uint16_t length);
void rbClear(RingBuffer* pRingBuff);
void rbPush(RingBuffer* pRingBuff, uint8_t value);
uint8_t rbPop(RingBuffer* pRingBuff);
uint16_t rbGetCount(const RingBuffer* pRingBuff);
int8_t rbIsEmpty(const RingBuffer* pRingBuff);
int8_t rbIsFull(const RingBuffer* pRingBuff);

/**
  * @brief  ���ringbuffer�ṹ��������Ϣ
  * @param  pRingBuff���������ringbuffer
  * @note   
  * @retval void
  * @author Acorss������	
  */	
inline void rbClear(RingBuffer* pRingBuff)
{
 	pRingBuff->wp = pRingBuff->pBuff;
	pRingBuff->rp = pRingBuff->pBuff;
	pRingBuff->flagOverflow = 0;
}

/**
  * @brief  ѹ�뵥�ֽڵ�������
  * @param  pRingBuff���������ringbuffer
  *         value��ѹ�������
  * @note   
  * @retval void
  * @author Acorss������	
  */	
inline void rbPush(RingBuffer* pRingBuff, uint8_t value)
{
	uint8_t* wp_next = pRingBuff->wp + 1;
	if( wp_next == pRingBuff->pEnd ) {
		wp_next -= pRingBuff->length; // Rewind pointer when exceeds bound
	}
	if( wp_next != pRingBuff->rp ) {
		*pRingBuff->wp = value;
		pRingBuff->wp = wp_next;
	} else {
		pRingBuff->flagOverflow = 1;
	}
}

/**
  * @brief  ѹ�����ֽڵ�������
  * @param  pRingBuff���������ringbuffer   
  * @note   
  * @retval ѹ��������
  * @author Acorss������	
  */	
inline uint8_t rbPop(RingBuffer* pRingBuff)
{
	if( pRingBuff->rp == pRingBuff->wp ) 
		return 0; // empty
  
	uint8_t ret = *(pRingBuff->rp++);
	if( pRingBuff->rp == pRingBuff->pEnd ) {
		pRingBuff->rp -= pRingBuff->length; // Rewind pointer when exceeds bound
	}
	return ret;
}

/**
  * @brief  ��ȡ��������δ������ֽ���
  * @param  pRingBuff���������ringbuffer   
  * @note   
  * @retval ��������ֽ���
  * @author Acorss������	
  */
inline uint16_t rbGetCount(const RingBuffer* pRingBuff)
{
	return (pRingBuff->wp - pRingBuff->rp + pRingBuff->length) % pRingBuff->length;
}

/**
  * @brief  �жϻ������Ƿ�Ϊ��
  * @param  pRingBuff���������ringbuffer   
  * @note   
  * @retval ��Ϊ1������Ϊ0
  * @author Acorss������	
  */
inline int8_t rbIsEmpty(const RingBuffer* pRingBuff)
{
	return pRingBuff->wp == pRingBuff->rp; 
}

/**
  * @brief  �жϻ������Ƿ��
  * @param  pRingBuff���������ringbuffer   
  * @note   
  * @retval ��Ϊ1������Ϊ0
  * @author Acorss������	
  */
inline int8_t rbIsFull(const RingBuffer* pRingBuff)
{
 	return (pRingBuff->rp - pRingBuff->wp + pRingBuff->length - 1) % pRingBuff->length == 0;
}


#ifdef __cplusplus
}
#endif

#endif 

