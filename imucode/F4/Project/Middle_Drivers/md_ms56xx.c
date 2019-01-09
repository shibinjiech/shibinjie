/*******************************************************************************
* �ļ����ƣ�md_ms56xx.c
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

#include "md_ms56xx.h"
#include "math.h"
#include "stdio.h"

#define CYCLE_100HZ_FROM_500HZ   5

extern SPI_HandleTypeDef hspi1;

const double T1 = 15.0 + 273.15;	/* temperature at base height in Kelvin */
const double A  = -6.5 / 1000;		/* temperature gradient in degrees per metre */
const double g  = 9.80665;				/* gravity constant in m/s/s */
const double RR  = 287.05;				/* ideal gas constant in J/kg/K */
const double p1 = 1013.15;
uint16_t C[8];  							// calibration coefficients

__IO ITStatus Bar_Conv_Flag = RESET;
__IO uint32_t Bar_RunCount=0;
__IO uint32_t D1; // ADC value of the pressure conversion
__IO uint32_t D2; // ADC value of the temperature conversion
//��ѹ�ƶ�ȡ����ѹֵ
MS56XX_Data 	  m_Ms56xx;

/*******************************��������****************************************
* ��������: void Baro_Reset(void)
* �������:
* ���ز���:
* ��    ��: ��λms56xx������
* ��    ��: by Across������
* ��    ��: 2017/12/18
*******************************************************************************/
void Baro_Reset(void)
{
	uint8_t txdata = CMD_RESET;
	BARO_CS_L;
	HAL_SPI_Transmit(&hspi1, &txdata, 1, 50);
	HAL_Delay(10);
	BARO_CS_H;
}

/*******************************��������****************************************
* ��������: unsigned int Baro_Cmd_Prom(char coef_num)
* �������: coef_num��ϵ��number
* ���ز���:
* ��    ��: ��ȡ�����������ϵ��
* ��    ��: by Across������
* ��    ��: 2017/12/18
*******************************************************************************/
unsigned int Baro_Cmd_Prom(char coef_num)
{
	unsigned int rc = 0;
	uint8_t txdata[3] = {0}, rxdata[3] = {0};

	BARO_CS_L;
	txdata[0] = CMD_PROM_RD + coef_num * 2;
	HAL_SPI_TransmitReceive(&hspi1, txdata, rxdata, 3, 50);
	BARO_CS_H;

	rc = 256 * rxdata[1] + rxdata[2];

	return rc;
}

/*******************************��������****************************************
* ��������: void Baro_Read_Coe(void)
* �������:
* ���ز���:
* ��    ��: ��ȡ����
* ��    ��: by Across������
* ��    ��: 2017/12/18
*******************************************************************************/
void Baro_Read_Coe(void)
{
	// read calibration coefficients
	for (uint8_t i = 0; i < 8; i++)
	{
		C[i] = Baro_Cmd_Prom(i);
	}
}

/*******************************��������****************************************
* ��������: uint8_t Baro_Crc4_Check(uint32_t n_prom[])
* �������: n_prom[]����ȡ��ϵ����������
* ���ز���: uint8_t�����ؼ����CRCУ��ֵ����ȷУ�鷵��ֵӦ����0X0B��
* ��    ��: CRC ���㣬������֤��ȡ����8λ����ϵ���Ƿ���ȷ
* ��    ��: by Across������
* ��    ��: 2017/12/18
*******************************************************************************/
uint8_t Baro_Crc4_Check(uint16_t n_prom[])
{
	uint32_t cnt;
	uint32_t n_rem, crc_read;
	uint8_t n_bit;

	crc_read = n_prom[7];
	n_prom[7] = (0xff00 & (n_prom[7]));

	for (cnt = 0; cnt < 16; cnt++)
	{
		if (cnt%2==1) n_rem ^= (unsigned short) ((n_prom[cnt>>1]) & 0x00FF);
		else n_rem ^= (unsigned short) (n_prom[cnt>>1]>>8);
		for (n_bit = 8; n_bit > 0; n_bit--)
		{
			if (n_rem & (0x8000))
				n_rem = (n_rem << 1) ^ 0x3000;
			else
				n_rem = (n_rem << 1);
		}
	}

	n_rem = (0x000F & (n_rem >> 12)); // // final 4-bit reminder is CRC code
	n_prom[7] = crc_read;

	return (n_rem ^ 0x00);
}



/*******************************��������****************************************
* ��������: void Baro_En_Conv(char cmd)
* �������:
* ���ز���:
* ��    ��: enable pressure adc cov
* ��    ��: by Across������
* ��    ��: 2017/12/18
*******************************************************************************/
void Baro_En_Conv(char cmd)
{
	uint8_t txdata;

	BARO_CS_L;
	txdata = CMD_ADC_CONV + cmd;
	HAL_SPI_Transmit(&hspi1, &txdata, 1, 50);
	BARO_CS_H;
}

/*******************************��������****************************************
* ��������: void Baro_Enable_Temp_Conv(void)
* �������:
* ���ز���:
* ��    ��: ʹ���¶�ת������
* ��    ��: by xiaodaqi
* ��    ��: 2017/12/18
*******************************************************************************/
void Baro_Enable_Temp_Conv(void)
{
	Baro_En_Conv(CMD_ADC_D2+CMD_ADC_4096);
}

/*******************************��������****************************************
* ��������: void Baro_Enable_Press_Conv(void)
* �������:
* ���ز���:
* ��    ��: ʹ��ѹ��ת������
* ��    ��: by xiaodaqi
* ��    ��: 2017/12/18
*******************************************************************************/
void Baro_Enable_Press_Conv(void)
{
	Baro_En_Conv(CMD_ADC_D1+CMD_ADC_4096);
}

/*******************************��������****************************************
* ��������: unsigned long Baro_Read_Data(void)
* �������:
* ���ز���:  ���ض�ȡ����ֵ
* ��    ��: read adc result, after calling Baro_En_Conv()
* ��    ��: by Across������
* ��    ��: 2017/12/18
*******************************************************************************/
unsigned long Baro_Read_Data(void)
{
	uint8_t txdata[4];
	uint8_t rxdata[4];
	unsigned long temp;

	BARO_CS_L;
	txdata[0] = CMD_ADC_READ;
	HAL_SPI_TransmitReceive(&hspi1, txdata, rxdata, 4, 50);
	BARO_CS_H;

	temp = 65535 * rxdata[1] + 256 * rxdata[2] + rxdata[3];

	return temp;
}

/*******************************��������****************************************
* ��������: float Baro_Cal_Alt(unsigned long D1, unsigned long D2)
* �������: D1:��ѹԭʼ��ֵ �� D2�¶�ԭʼ��ֵ
* ���ز���:
* ��    ��: �������ѹֵ����λmba��
* ��    ��: by Across������
* ��    ��: 2017/12/18
*******************************************************************************/
float Baro_Cal_Alt(unsigned long D1, unsigned long D2)
{
	int32_t P; // compensated pressure value
	int32_t TEMP; // compensated temperature value
	int32_t dT; // difference between actual and measured temperature
	int64_t OFF,OFF2; // offset at actual temperature
	int64_t SENS,SENS2; // sensitivity at actual temperature

	dT = D2 - (uint32_t)C[5] * 256;

#ifdef MS5611
	OFF = (int64_t)C[2] * 65536 + (int64_t)C[4] * dT / 128;
	SENS = (int64_t)C[1] * 32768 + (int64_t)C[3] * dT / 256;
#endif

#ifdef MS5607
	OFF = (int64_t)C[2] * 131072 + (int64_t)C[4] * dT / 64;
	SENS = (int64_t)C[1] * 65536 + (int64_t)C[3] * dT / 128;
#endif

	TEMP = 2000 + ((int64_t) dT * C[6]) / 8388608;
	//printf("TEMP is ��%ld  ",TEMP);

	OFF2 = 0;
	SENS2 = 0;

	if (TEMP < 2000)
	{
		OFF2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 2;
		SENS2 = 5 * ((TEMP - 2000) * (TEMP - 2000)) / 4;
	}

	if (TEMP < -1500)
	{
		OFF2 = OFF2 + 7 * ((TEMP + 1500) * (TEMP + 1500));
		SENS2 = SENS2 + 11 * ((TEMP + 1500) * (TEMP + 1500)) / 2;
	}

	OFF = OFF - OFF2;
	SENS = SENS - SENS2;

	P = (D1 * SENS / 2097152 - OFF) / 32768;
	//printf(" P is ��%ld \r\n ", P);
	return P;
}

/*******************************��������****************************************
* ��������: void Baro_Init(void)
* �������:
* ���ز���:
* ��    ��: ��ʼ��ms56xx
* ��    ��: by Across������
* ��    ��: 2017/12/18
*******************************************************************************/
void Baro_Init(void)
{
	Baro_Reset();
	Baro_Read_Coe();
	//����һ���¶�ת��
	Baro_Enable_Temp_Conv();
}

/*******************************��������****************************************
* ��������: void Baro_Read_Pressure_Test(void)
* �������:
* ���ز���:
* ��    ��: ���Դ��������ݶ�ȡ�Ƿ���ȷ��
* ��    ��: Across������
* ��    ��: 2018.4.15
*******************************************************************************/
void Baro_Read_Pressure_Test(void)
{
	/* 1.����һ����ѹת�� */
	Baro_Enable_Press_Conv();
	/* 2.��ʱ10ms�����ȡת������¶���ֵ*/
	HAL_Delay(10);
	D1 = Baro_Read_Data();
	/* 3.����һ��ѹ��ת��*/
	Baro_Enable_Temp_Conv();
	/*4.��ʱ10ms����ȡת�����ѹ����ֵ*/
	HAL_Delay(10);
	D2 = Baro_Read_Data();
	/* 5.���ù�ʽ�������յ���ѹ��ֵ��*/
	m_Ms56xx.pressure = Baro_Cal_Alt(D1,D2);
	printf("pressure is %lf mbar \r\n",m_Ms56xx.pressure/100.0);
}

/*******************************��������****************************************
* ��������: void Loop_Read_Bar(void)
* �������:
* ���ز���:
* ��    ��: 500hz��Ƶ�ʵ��øú��������������ѹ�����ݻ���50hzƵ�ʸ��¡�
            ע�⣺�ٵ�һ�����øú���ǰ����������һ���¶�ת����
* ��    ��:  Across������
* ��    ��:
*******************************************************************************/
void Loop_Read_Bar(void)
{
	//����ִ��һ�θñ���+1
	Bar_RunCount++;

	//Bar_RunCountÿ�仯5�Σ�if���ִ��һ�Σ�����if���100hzִ��
	if(Bar_RunCount % CYCLE_100HZ_FROM_500HZ ==0)
	{
		if(Bar_Conv_Flag==RESET)
		{
			Bar_Conv_Flag = SET;
			//��ȡ�¶�rawֵ
			D2 = Baro_Read_Data();
			//������һ����ѹת��
			Baro_Enable_Press_Conv();
		}
		else
		{
			Bar_Conv_Flag=RESET;
			//��ȡ��ѹrawֵ
			D1 = Baro_Read_Data();
			m_Ms56xx.pressure = Baro_Cal_Alt(D1,D2);			
			//������һ���¶�ת��
			Baro_Enable_Temp_Conv();
			//printf("pressure is %lf mbar \r\n",m_Ms56xx.pressure/100.0);
       //printf("%ld\r\n",m_Ms56xx.pressure);			
		}

	}
}

