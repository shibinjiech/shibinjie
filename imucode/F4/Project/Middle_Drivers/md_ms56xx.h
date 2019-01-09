/*******************************************************************************
* �ļ����ƣ�md_ms56xx.h
*
* ժ    Ҫ����ʼ��mpu5611���������
*
* ��ǰ�汾��
* ��    �ߣ�Across������	
* ��    �ڣ�2017/12/18
* ���뻷����keil5
*
* ��ʷ��Ϣ��
*******************************************************************************/
#ifndef __MD_ms56xx_H
#define __MD_ms56xx_H

#include "stm32f4xx_hal.h"
#include "main.h"
#include "md_struct.h"

/*�����Լ��Ĵ���������ѡ����Ӧ�ĺ�*/
#define MS5607
//#define MS5611

#if !defined (MS5607) && !defined (MS5611)
#error "Please define a macro to decide which sensor that be used!"
#endif

#if defined (MS5607) && defined (MS5611)
#error "These two macros (MS5607,MS5611) cannot be defined at the same time!"
#endif

//mpu6500��SPIʹ�����ſ���
#define BARO_CS_H     HAL_GPIO_WritePin(BARO_CS_GPIO_Port, BARO_CS_Pin, GPIO_PIN_SET)
#define BARO_CS_L     HAL_GPIO_WritePin(BARO_CS_GPIO_Port, BARO_CS_Pin, GPIO_PIN_RESET)


#define CMD_RESET    0x1E
#define CMD_ADC_READ 0x00
#define CMD_ADC_CONV 0x40
#define CMD_ADC_D1   0x00
#define CMD_ADC_D2   0x10

#define CMD_ADC_256  0x00
#define CMD_ADC_512  0x02
#define CMD_ADC_1024 0x04
#define CMD_ADC_2048 0x06
#define CMD_ADC_4096 0x08
#define CMD_PROM_RD  0xA0

void Baro_En_Conv(char cmd);
unsigned long Baro_Read_Data(void);
void Baro_Init(void);
float Baro_Get_Alt(void);
float Baro_Get_Alt2(void);
float Baro_Get_Alt3(void);
void Loop_Read_Bar(void);
void Baro_Read_Pressure_Test(void);

#endif 
