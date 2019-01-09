/*******************************************************************************
* 文件名称：md_led_driver.c
*
* 摘    要：STM32F1的LED中间驱动层实现（底层硬件驱动和软件应用层之间的层）
*
* 当前版本：
* 作    者：
* 日    期：2017/12/18
* 编译环境：keil5
*
* 历史信息：
*******************************************************************************/

#include "md_led_driver.h"

/*******************************函数声明****************************************
* 函数名称: void MD_LED_AMBER_Control(uint8_t status)
* 输入参数: status：控制灯的亮灭状态
* 返回参数:  
* 功    能:
* 作    者: 
* 日    期: 2017/12/18
*******************************************************************************/ 
void MD_LED_AMBER_Control(uint8_t status)
{
	if(status==1)
	{
		//点亮
	  HAL_GPIO_WritePin(FMC_LED_AMBER_GPIO_Port,FMC_LED_AMBER_Pin,GPIO_PIN_RESET);
	}
	else
	{
		//熄灭
	   HAL_GPIO_WritePin(FMC_LED_AMBER_GPIO_Port,FMC_LED_AMBER_Pin,GPIO_PIN_SET);
	}
}

