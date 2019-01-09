/*******************************************************************************
* �ļ����ƣ�md_hmc5883.c
*
* ժ    Ҫ��1.�������ģ��I2Cͨ��Э��
*           2.��ʼ��hmc5883���������,
*
* ��ǰ�汾��
* ��    �ߣ�Acorss������	
* ��    �ڣ�2017/12/18
* ���뻷����keil5
*
* ��ʷ��Ϣ��
*******************************************************************************/

#include "md_hmc5883.h"
#include "stdio.h"

#define CYCLE_50HZ_FROM_500HZ   10

static float _hmc5883_Gauss_LSB_XYZ = 1024.0F;  // Varies with gain
__IO uint32_t Mag_RunCount=0;

hmc5883MagGain   _magGain;
hmc5883MagData 	m_Hmc5883;
/*******************************************************************************/
/**
  * @brief  ģ��IIC��ʱ
  * @param
  * @note
  * @retval void
  * @author Acorss������
  */
void Sim_I2C1_Delay(uint32_t delay)
{
	while(--delay);	//dly=100: 8.75us; dly=100: 85.58 us (SYSCLK=72MHz)
}

/**
  * @brief  ģ��IIC��ʼʱ��
  * @param
  * @note
  * @retval void
  * @author Acorss������
  */
uint8_t Sim_I2C1_START(void)
{
	SDA1_OUT();
	Sim_I2C1_SDA_HIG;
	Sim_I2C1_SCL_HIG;
	Sim_I2C1_NOP;

// if(!Sim_I2C1_SDA_STATE) return Sim_I2C1_BUS_BUSY;

	Sim_I2C1_SDA_LOW;
	Sim_I2C1_NOP;

	Sim_I2C1_SCL_LOW;
	Sim_I2C1_NOP;

	//if(Sim_I2C1_SDA_STATE) return Sim_I2C1_BUS_ERROR;

	return Sim_I2C1_READY;
}

/**
  * @brief  ģ��IICֹͣʱ��
  * @param
  * @note
  * @retval void
  * @author Acorss������
  */
void Sim_I2C1_STOP(void)
{
	SDA1_OUT();
	Sim_I2C1_SCL_LOW;
	Sim_I2C1_SDA_LOW;
	Sim_I2C1_NOP;

//	Sim_I2C1_SCL_LOW;
//  Sim_I2C1_NOP;

	Sim_I2C1_SCL_HIG;
	Sim_I2C1_SDA_HIG;
	Sim_I2C1_NOP;
}

unsigned char Sim_I2C1_Wait_Ack(void)
{
	volatile unsigned char ucErrTime=0;
	SDA1_IN();
	Sim_I2C1_SDA_HIG;
	Sim_I2C1_NOP;;
	Sim_I2C1_SCL_HIG;
	Sim_I2C1_NOP;;
	while(Sim_I2C1_SDA_STATE)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			Sim_I2C1_STOP();
			return 1;
		}
	}
	Sim_I2C1_SCL_LOW;
	return Sim_I2C1_READY;
}

/**
  * @brief  ģ��IICӦ��ʱ��
  * @param
  * @note
  * @retval void
  * @author Acorss������
  */
void Sim_I2C1_SendACK(void)
{
	Sim_I2C1_SCL_LOW;
	SDA1_OUT();
	Sim_I2C1_SDA_LOW;
	Sim_I2C1_NOP;
	Sim_I2C1_SCL_HIG;
	Sim_I2C1_NOP;
	Sim_I2C1_SCL_LOW;
	Sim_I2C1_NOP;
}

/**
  * @brief  ģ��IIC��Ӧ��ʱ��
  * @param
  * @note
  * @retval void
  * @author Acorss������
  */
void Sim_I2C1_SendNACK(void)
{
	Sim_I2C1_SCL_LOW;
	SDA1_OUT();
	Sim_I2C1_SDA_HIG;
	Sim_I2C1_NOP;
	Sim_I2C1_SCL_HIG;
	Sim_I2C1_NOP;
	Sim_I2C1_SCL_LOW;
	Sim_I2C1_NOP;
}

/**
  * @brief  ģ��IIC���͵��ֽ�ʱ��
  * @param
  * @note
  * @retval void
  * @author Acorss������
  */
uint8_t Sim_I2C1_SendByte(uint8_t Sim_i2c_data)
{
	uint8_t i;
	SDA1_OUT();
	Sim_I2C1_SCL_LOW;
	for(i=0; i<8; i++)
	{
		if(Sim_i2c_data&0x80) Sim_I2C1_SDA_HIG;
		else Sim_I2C1_SDA_LOW;

		Sim_i2c_data<<=1;
		Sim_I2C1_NOP;

		Sim_I2C1_SCL_HIG;
		Sim_I2C1_NOP;
		Sim_I2C1_SCL_LOW;
		Sim_I2C1_NOP;
	}
	return Sim_I2C1_READY;
}

/**
  * @brief  ģ��IIC�����ֽڣ���Ӧ��
  * @param
  * @note
  * @retval void
  * @author Acorss������
  */
uint8_t Sim_I2C1_ReceiveByte(void)
{
	uint8_t i,Sim_i2c_data;
	SDA1_IN();
	//Sim_I2C1_SDA_HIG;
// Sim_I2C1_SCL_LOW;
	Sim_i2c_data=0;

	for(i=0; i<8; i++)
	{
		Sim_I2C1_SCL_LOW;
		Sim_I2C1_NOP;
		Sim_I2C1_SCL_HIG;
		// Sim_I2C1_NOP;
		Sim_i2c_data<<=1;

		if(Sim_I2C1_SDA_STATE)	Sim_i2c_data|=0x01;

		// Sim_I2C1_SCL_LOW;
		Sim_I2C1_NOP;
	}
	Sim_I2C1_SendNACK();
	return Sim_i2c_data;
}

/**
  * @brief  ģ��IIC�����ֽڣ���Ӧ��
  * @param
  * @note
  * @retval void
  * @author Acorss������
  */
uint8_t Sim_I2C1_ReceiveByte_WithACK(void)
{

	uint8_t i,Sim_i2c_data;
	SDA1_IN();
	//Sim_I2C1_SDA_HIG;
// Sim_I2C1_SCL_LOW;
	Sim_i2c_data=0;

	for(i=0; i<8; i++)
	{
		Sim_I2C1_SCL_LOW;
		Sim_I2C1_NOP;
		Sim_I2C1_SCL_HIG;
		// Sim_I2C1_NOP;
		Sim_i2c_data<<=1;

		if(Sim_I2C1_SDA_STATE)	Sim_i2c_data|=0x01;

		// Sim_I2C1_SCL_LOW;
		Sim_I2C1_NOP;
	}
	Sim_I2C1_SendACK();
	return Sim_i2c_data;
}


/**
  * @brief  ģ��IIC�Ķ��ֽڶ�
  * @param
  * @note
  * @retval void
  * @author Acorss������
  */
uint8_t Sim_DMP_I2C_Read8(uint8_t moni_dev_addr, uint8_t moni_reg_addr, uint8_t moni_i2c_len, uint8_t *moni_i2c_data_buf)
{

	Sim_I2C1_START();
	Sim_I2C1_SendByte(moni_dev_addr << 1 | I2C1_Direction_Transmitter);
	Sim_I2C1_Wait_Ack();
	Sim_I2C1_SendByte(moni_reg_addr);
	Sim_I2C1_Wait_Ack();
	//Sim_I2C1_STOP();
	
	Sim_I2C1_START();
	Sim_I2C1_SendByte(moni_dev_addr << 1 | I2C1_Direction_Receiver);
	Sim_I2C1_Wait_Ack();
	while (moni_i2c_len)
	{
		if (moni_i2c_len==1) *moni_i2c_data_buf =Sim_I2C1_ReceiveByte();
		else *moni_i2c_data_buf =Sim_I2C1_ReceiveByte_WithACK();
		moni_i2c_data_buf++;
		moni_i2c_len--;
	}
	Sim_I2C1_STOP();
	return 0x00;
}

/*******************************************************************************/
/**
  * @brief  ģ��IIC�Ķ��ֽ�д
  * @param
  * @note   ������check���ܵ�ʱ��ֻ���ǵ��ֽ�д����������ֽ�д��������check����
  * @retval void
  * @author Acorss������
  */
int8_t Sim_I2C1_Write8(uint8_t moni_dev_addr, uint8_t moni_reg_addr, uint8_t moni_i2c_len, uint8_t *moni_i2c_data_buf)
{
	uint8_t i;
	Sim_I2C1_START();
	Sim_I2C1_SendByte(moni_dev_addr << 1 | I2C1_Direction_Transmitter);
	Sim_I2C1_Wait_Ack();
	Sim_I2C1_SendByte(moni_reg_addr);
	Sim_I2C1_Wait_Ack();
	
	//Sim_I2C1_START();
	for (i=0; i<moni_i2c_len; i++)
	{
		Sim_I2C1_SendByte(moni_i2c_data_buf[i]);
		Sim_I2C1_Wait_Ack();
	}
	Sim_I2C1_STOP();	
		return 0;
}

/*******************************��������****************************************
* ��������: void HMC5883_Set_MagGain(hmc5883MagGain gain)
* �������: gain����Ҫ���õ�������ֵ
* ���ز���:  
* ��    ��: ���ô�����������
* ��    ��: by Acorss������
* ��    ��: 2017/12/18
*******************************************************************************/ 
void HMC5883_Set_MagGain(hmc5883MagGain gain)
{
	uint8_t writebyte;
	writebyte = gain;
  Sim_I2C1_Write8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_CRB_REG_M,1,&writebyte);
	
	_magGain = gain;

  switch(gain)
  {
    case HMC5883_MAGGAIN_1_2:
      _hmc5883_Gauss_LSB_XYZ = 1024.0;
      break;
    case HMC5883_MAGGAIN_1_9:
      _hmc5883_Gauss_LSB_XYZ = 768.0;
      break;
    case HMC5883_MAGGAIN_2_5:
     _hmc5883_Gauss_LSB_XYZ = 614.0;
      break;
    case HMC5883_MAGGAIN_4_0:
    _hmc5883_Gauss_LSB_XYZ = 415.0;
      break;
    case HMC5883_MAGGAIN_4_7:
      _hmc5883_Gauss_LSB_XYZ = 361.0;
      break;
    case HMC5883_MAGGAIN_5_6:
     _hmc5883_Gauss_LSB_XYZ = 307.0;
      break;
    case HMC5883_MAGGAIN_8_1:
      _hmc5883_Gauss_LSB_XYZ = 219.0;
      break;
  } 
}
/*******************************��������****************************************
* ��������: void HMC5883_Start_Convst(void)
* �������: 
* ���ز���:  
* ��    ��: ����hmc5883�ĵ��β���ģʽ 6ms����Զ�����
* ��    ��: by Acorss������
* ��    ��: 2017/12/18
*******************************************************************************/ 
void HMC5883_Start_Convst(void)
{
	uint8_t writebyte;
  // single measurement mode
	writebyte = 0x01;
  Sim_I2C1_Write8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_MR_REG_M,1,&writebyte);
}

/*******************************��������****************************************
* ��������: void HMC5883_Init(void)
* �������: 
* ���ز���:  
* ��    ��: ��ʼˢHMC5883������ģʽ������Ϊ����1.2g�����β���ģʽ
* ��    ��: by Acorss������
* ��    ��: 2017/12/18
*******************************************************************************/ 
void HMC5883_Init(void)
{
	//ȷ���������ϵ��ȶ�ʱ��
	HAL_Delay(10);	
	uint8_t writebyte;
	/*
	//����ģʽ ��75HZ���Ƶ��
	printf("HMC5883_REGISTER_MAG_CRA_REG_M :0x78 \r\n");
	writebyte = 0x78;
	Sim_I2C1_Write8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_CRA_REG_M,1,&writebyte);
	Sim_DMP_I2C_Read8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_CRA_REG_M,1,&writebyte);
	printf("read data:%d\r\n",writebyte);
	
	//����+/- 1.3
	printf("HMC5883_Set_MagGain :0x20\r\n");
	HMC5883_Set_MagGain(HMC5883_MAGGAIN_1_2);
	Sim_DMP_I2C_Read8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_CRB_REG_M,1,&writebyte);
	printf("read data:%d\r\n",writebyte);
	
	// Contimuous Measurement mode
	printf("Measurement mode :0x00\r\n");
	writebyte = 0x00;
  Sim_I2C1_Write8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_MR_REG_M,1,&writebyte);
	Sim_DMP_I2C_Read8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_MR_REG_M,1,&writebyte);
	printf("read data:%d\r\n",writebyte);
	*/
	/*����ģʽ��ƽ������8��*/
	writebyte = 0x70;
	Sim_I2C1_Write8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_CRA_REG_M,1,&writebyte);
	
	//����+/- 1.3
	HMC5883_Set_MagGain(HMC5883_MAGGAIN_1_2);
	
	// single measurement mode
	writebyte = 0x01;
  Sim_I2C1_Write8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_MR_REG_M,1,&writebyte);
	//����װ��ʱ�������160hz
	
	HAL_Delay(10);	
}

/*******************************��������****************************************
* ��������: void HMC5883_Read_Data(hmc5883MagData * _magData)
* �������: 
* ���ز���:  
* ��    ��: ��ȡ��������������Ĵų���Ϣ
* ��    ��: by Acorss������
* ��    ��: 2017/12/18
*******************************************************************************/ 
uint8_t readBuff[6];
void HMC5883_Read_Data(hmc5883MagData * _magData)
{	
  Sim_DMP_I2C_Read8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_OUT_X_H_M,6,readBuff);
	uint8_t xhi = readBuff[0];
  uint8_t xlo = readBuff[1];
  uint8_t zhi = readBuff[2];
  uint8_t zlo = readBuff[3];
  uint8_t yhi = readBuff[4];
  uint8_t ylo = readBuff[5];
	
	_magData->x = (int16_t)(xlo | ((int16_t)xhi << 8));
  _magData->y = (int16_t)(ylo | ((int16_t)yhi << 8));
  _magData->z = (int16_t)(zlo | ((int16_t)zhi << 8));
	//printf("%lf ,%lf��%lf \r\n",_magData.x ,_magData.y,_magData.z);
	//����ɸ�˹��λ
	_magData->x /= _hmc5883_Gauss_LSB_XYZ;
	_magData->y /= _hmc5883_Gauss_LSB_XYZ;
	_magData->z /= _hmc5883_Gauss_LSB_XYZ;
	//printf("%lf ,%lf��%lf \r\n",_magData->x ,_magData->y,_magData->z);
}

/*******************************��������****************************************
* ��������: void HMC5883_Read_Identif(void)
* �������: 
* ���ز���:  
* ��    ��: ��ȡIDֵ
* ��    ��: by Acorss������
* ��    ��: 2017/12/18
*******************************************************************************/ 
void HMC5883_Read_Identif(void)
{
  Sim_DMP_I2C_Read8(HMC5883_ADDRESS_MAG,HMC5883_REGISTER_MAG_IRA_REG_M,3,readBuff);
	printf("IdA:%d,IdB:%d,IdC:%d\r\n",readBuff[0],readBuff[1],readBuff[2]);
	HAL_Delay(100);
}

/*******************************��������****************************************
* ��������: void HMC5886_Test(void)
* �������: 
* ���ز���:  
* ��    ��: �򵥵Ĳ��Ժ�������֤I2Cͨ�ţ����ݶ�д�Ƿ������ȡ�
* ��    ��: by Acorss������
* ��    ��: 2017/12/18
*******************************************************************************/ 
void HMC5886_Test(void)
{
  //����һ��ת��
	HMC5883_Start_Convst();
	HAL_Delay(10);
	//100hz���ش�����
	HMC5883_Read_Data(&m_Hmc5883);
}

/*******************************��������****************************************
* ��������: void Loop_Read_Mag(void)
* �������: 
* ���ز���:  
* ��    ��: 50HZѭ���شŲɼ��������������µ����ݱ��浽m_Hmc5883�С����øú���ǰִ��һ��HMC5883_Start_Convst();
* ��    ��: by Acorss������
* ��    ��: 2018/4/22
*******************************************************************************/ 
void Loop_Read_Mag(void)
{
	//����ִ��һ�θñ���+1
	Mag_RunCount++;

	//Bar_RunCountÿ�仯10�Σ�if���ִ��һ�Σ�����if���50hzִ��
	if(Mag_RunCount % CYCLE_50HZ_FROM_500HZ ==0)
	{
      //50hz���ش�����
			HMC5883_Read_Data(&m_Hmc5883);
				//������һ��ת��
		  HMC5883_Start_Convst();	
	}
}

