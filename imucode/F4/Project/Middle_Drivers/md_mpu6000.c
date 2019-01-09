/*******************************************************************************
* �ļ����ƣ�md_mpu6000.c
*
* ժ    Ҫ����ʼ��mpu6000���������
*
* ��ǰ�汾��
* ��    �ߣ� 
* ��    �ڣ�2017/12/18
* ���뻷����keil5
*
* ��ʷ��Ϣ��
*******************************************************************************/

#include "md_mpu6000.h"
#include "string.h"
#include "md_struct.h"

extern SPI_HandleTypeDef hspi1;
//ȫ�ֱ��������ڱ���MPU6000�Ĵ���������
MPU6000_Data 		m_Mpu6000;

/*******************************��������****************************************
* ��������: static uint8_t MPU6000_Spi_Com(uint8_t addr, uint8_t data)
* �������: addr:��Ҫ��д�ļĴ�����ַ��data����Ҫд��Ĳ���
* ���ز���: uint8_t �����ؼĴ�����ȡ����ֵ
* ��    ��: ��ȡĿ��Ĵ���/д��Ŀ��Ĵ���һ���ֽ�
* ��    ��: Across������ 
* ��    ��: 2017/12/18
*******************************************************************************/ 
static uint8_t MPU6000_Spi_Com(uint8_t addr, uint8_t data)
{
	uint8_t txdata[2];
	uint8_t rxdata[2];
	
	txdata[0] = addr;
	txdata[1] = data;
	
	//write data and receive reg value
	MPU_CS_L;
	HAL_Delay(1);
	HAL_SPI_TransmitReceive(&hspi1, txdata, rxdata, 2, 500);
	MPU_CS_H;	
	
	return rxdata[1];
}

/*******************************��������****************************************
* ��������: void MPU6000_Init(void)
* �������: void	
* ���ز���: void 
* ��    ��: MPU6000��ʼ��
* ��    ��: Across������ 
* ��    ��: 2017/12/18
*******************************************************************************/ 
void MPU6000_Init(void)
{
	MPU6000_Spi_Com(PWR_MGMT_1, 0x80);
	HAL_Delay(10);
	MPU6000_Spi_Com(0x68, 0x07);
	HAL_Delay(10);
	
	MPU6000_Spi_Com(PWR_MGMT_1, 0x01);  //����ȡ��ʱ��Ϊ����X��
	HAL_Delay(10);
	MPU6000_Spi_Com(PWR_MGMT_2, 0x00);  //ʹ�ܼ��ٶȼơ�������
	HAL_Delay(10);
	/* (1000 / (1+1) = 500Hz) */
	MPU6000_Spi_Com(SMPLRT_DIV, 0x00);  //0x01 
	HAL_Delay(10);
	/*   lowpass : 41 */
	MPU6000_Spi_Com(CONFIG, 0x03);  //0x03
	HAL_Delay(10);
	MPU6000_Spi_Com(ACCEL_CONFIG2, 0x03);
	HAL_Delay(10);
	/* 18: -2000~+2000 */
	MPU6000_Spi_Com(GYRO_CONFIG, 0x18);
	HAL_Delay(10);
	/* 00: -4g~+4g 16384/g */
	MPU6000_Spi_Com(ACCEL_CONFIG, 0x08);
	HAL_Delay(10);
	HAL_Delay(100);
	MPU_CS_H;
}

/*******************************��������****************************************
* ��������: void MPU6000_Get_Data(mpu6000_s *mpu6000)
* �������: mpu6000_s:mpu6000����Ľṹ��
* ���ز���: void	
* ��    ��: ��ȡmpu6000�����
* ��    ��: Across������  
* ��    ��: 2017/12/18
*******************************************************************************/ 
void MPU6000_Get_Data(MPU6000_Data *mpu6000)
{

	uint8_t txdata[15] = {0};
	uint8_t rxdata[15];	
	int16_t tmp;
	
	txdata[0] = ACCEL_XOUT_H | 0x80;
	MPU_CS_L;
	HAL_SPI_TransmitReceive(&hspi1, txdata, rxdata, 15, 50);
	MPU_CS_H;
	//acc
	tmp = (rxdata[1] << 8) + rxdata[2];
	mpu6000->acc_raw.x = tmp;
	
	tmp = (rxdata[3] << 8) + rxdata[4];
	mpu6000->acc_raw.y = tmp;

	tmp = (rxdata[5] << 8) + rxdata[6];
	mpu6000->acc_raw.z = tmp;

	//temperature
	mpu6000->temp = (rxdata[7] << 8) + rxdata[8];
	//gyro
	tmp = (rxdata[9] << 8) + rxdata[10];
	mpu6000->gyro_raw.x = tmp;
	
	tmp = (rxdata[11] << 8) + rxdata[12];
	mpu6000->gyro_raw.y = tmp;
	
	
	tmp = (rxdata[13] << 8) + rxdata[14];
	mpu6000->gyro_raw.z = tmp;
}

/*******************************��������****************************************
* ��������: void Get_Accel(Vector3_Int16 *acc)
* �������: Vector3_Int16�����ᶨ��Ľṹ�壬���ڴ洢X,Y,Z�ı�����
* ���ز���:  
* ��    ��: ��ȡ���ٶȼƵ�����
* ��    ��: Across������  
* ��    ��: 2017/12/18
*******************************************************************************/  
void Get_Accel(Vector3_Int16 *acc)
{
	uint8_t txdata[7] = {0};
	uint8_t rxdata[7];
	uint16_t tmp;
	
	txdata[0] = ACCEL_XOUT_H | 0x80;
	MPU_CS_L;
	HAL_SPI_TransmitReceive(&hspi1, txdata, rxdata, 7, 50);
	MPU_CS_H;
	tmp = (rxdata[1] << 8) + rxdata[2];
	acc->x = tmp;
	tmp = (rxdata[3] << 8) + rxdata[4];
	acc->y = tmp;
	tmp = (rxdata[5] << 8) + rxdata[6];
	acc->z = tmp;
}

/*******************************��������****************************************
* ��������: void Get_Gyro(Vector3_Int16 *gyro)
* �������: Vector3_Int16�����ᶨ��Ľṹ�壬���ڴ洢X,Y,Z�ı�����
* ���ز���:  
* ��    ��: ��ȡ�����ǵ�����
* ��    ��: Across������ 
* ��    ��: 2017/12/18
*******************************************************************************/ 
void Get_Gyro(Vector3_Int16 *gyro)
{
	uint8_t txdata[7] = {0};
	uint8_t rxdata[7];
	int16_t tmp;
	
	txdata[0] = GYRO_XOUT_H | 0x80;
	MPU_CS_L;
	HAL_SPI_TransmitReceive(&hspi1, txdata, rxdata, 7, 50);
	MPU_CS_H;
	tmp = (rxdata[1] << 8) + rxdata[2];
	gyro->x = tmp;
	tmp = (rxdata[3] << 8) + rxdata[4];
	gyro->y = tmp;
	tmp = (rxdata[5] << 8) + rxdata[6];
	gyro->z = tmp;
}

/*******************************��������****************************************
* ��������:  void MPU6000_Test(void)
* �������:  void
* ���ز���:  void
* ��    ��:  ����SPI�ӿڼ�mpu6000��ʼ���ɹ����
* ��    ��:  Across������
* ��    ��:  2018.4.2
*******************************************************************************/ 
void MPU6000_Test(void)
{
   MPU6000_Data data;
	 MPU6000_Get_Data(&data);
	 printf("%d,%d,%d,%d,%d,%d\r\n",data.acc_raw.x,data.acc_raw.y,data.acc_raw.z,data.gyro_raw.x,data.gyro_raw.y,data.gyro_raw.z);
	 HAL_Delay(20);
}

/*******************************��������****************************************
* ��������: void Loop_Read_MPU6000(void)
* �������: void	
* ���ز���: void 
* ��    ��: ѭ����ȡmpu6000�Ĵ��������ݣ��������ݱ�����ȫ�ֱ���m_Mpu6000�С�
* ��    ��: Across������
* ��    ��: 2018.4.3 
*******************************************************************************/ 
void Loop_Read_MPU6000(void)
{
   MPU6000_Get_Data(&m_Mpu6000);
}
