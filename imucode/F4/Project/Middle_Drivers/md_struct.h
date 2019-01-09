/*******************************************************************************
* �ļ����ƣ�md_struct.h
*
* ժ    Ҫ���Զ���ṹ��
*
* ��ǰ�汾��
* ��    �ߣ�
* ��    �ڣ�2017/12/18
* ���뻷����keil5
*
* ��ʷ��Ϣ��
*******************************************************************************/

#ifndef __MD_STRUCT_H
#define __MD_STRUCT_H

#include "stm32f4xx_hal.h"

/** 
  * @brief  LED Status enumeration 
  */
typedef enum
{
  LED_OFF = 0U,
  LED_ON
}LED_Status;

typedef struct{
	int16_t x;
	int16_t y;
	int16_t z;
} Vector3_Int16;

typedef struct{
	float x;
	float y;
	float z;
}Vector3_Float;

typedef struct{
	Vector3_Int16 acc_raw;
	Vector3_Int16 gyro_raw;
	Vector3_Float accf;
	Vector3_Float gyrof;
	int16_t temp;   //temperature
}MPU6000_Data;

/* MS56XX��ѹ��ʱ����Ľṹ��*/
typedef struct {
  int32_t temp;
	int32_t pressure;
} MS56XX_Data;

/*hmc5883�شżƶ���Ľṹ��*/
typedef struct {
        float x;
        float y;
        float z;
        float orientation;
} hmc5883MagData;


#define CONSTRAINT(in, min, max)  (in > max ? max : (in < min ? min : in))
#define ABS(a) ((a) > 0 ? (a) : -(a))
#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

#endif
