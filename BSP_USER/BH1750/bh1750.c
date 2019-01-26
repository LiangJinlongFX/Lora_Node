/**
  * @file    bh1750.c
  * @author  Liang
  * @version V1.0.0
  * @date    2017-4-26
  * @brief	
  **/
#include "stm32f10x.h"
#include "myiic.h"
#include "bh1750.h"
#include "delay.h"

void BH1750_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;

	// 初始化DVI引脚
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //使能PB端口时钟

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO口速度为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_5);
	delay_ms(1);
	GPIO_SetBits(GPIOB,GPIO_Pin_5);
	
	// 初始化IIC
	IIC_Init();
}

/**
 * BH1750 设置模式
 * @param   
 * @return  0[成功] 1[失败]
 * @brief 
 **/
u8 BH1750_SetMode(u8 cmd)
{
	IIC_Start();	//IIC开始
	IIC_Send_Byte(BH1750FVI_ADDRESS);	//发送地址
	if(IIC_Wait_Ack())
		return 1;
	IIC_Send_Byte(cmd);	//发送测量命令
	if(IIC_Wait_Ack())
		return 1;
	IIC_Stop();	//IIC结束
	
	return 0;
}

/**
 * BH1750 读取数据
 * @param   
 * @return 
 * @brief 
 **/
u16 BH1750_ReadData(void)
{
	u16 RxData;
	u8 temp;
	
	IIC_Start();	//IIC开始
	IIC_Send_Byte(BH1750FVI_ADDRESS+1);	//发送读地址
	if(IIC_Wait_Ack())
		return 0;
	RxData = IIC_Read_Byte(1);
	printf("H=%x ",RxData);
	RxData = (RxData << 8);
	temp = IIC_Read_Byte(0);
	printf("L=%x\r\n",temp);
	RxData = RxData|temp;
	IIC_Stop();	//IIC结束
	
	return RxData;	
}

/**
 * 读取数据并转换为实际光感值
 * @param   
 * @return 
 * @brief 仅用于1lx分辨率模式
 **/
float BH1750_ReadRealData(void)
{
	float Data;
	u16 RxData;
	
	RxData=BH1750_ReadData();
	Data = (float)RxData/1.2;
	
	return Data;
}

/**
 * 单次高分辨率模式测量
 * @param   
 * @return 
 * @brief 
 **/
u16 BH1750_ReadData_Once(void)
{
	u16 RxData;
	
	IIC_Start();	//IIC开始
	IIC_Send_Byte(BH1750FVI_ADDRESS);	//发送地址
	if(IIC_Wait_Ack())
		return 0;
	IIC_Send_Byte(BH1750FVI_H_MODE_ONCE);	//发送测量命令
	if(IIC_Wait_Ack())
		return 0;
	IIC_Stop();	//IIC结束
	
	delay_ms(180);	//延时180ms等待转换完毕
	
	IIC_Start();	//IIC开始
	IIC_Send_Byte(BH1750FVI_ADDRESS+1);	//发送读地址
	if(IIC_Wait_Ack())
		return 0;
	RxData = IIC_Read_Byte(1);
	RxData = (RxData << 8);
	RxData = IIC_Read_Byte(0);
	IIC_Stop();	//IIC结束
	
	return RxData;
}