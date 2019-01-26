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

	// ��ʼ��DVI����
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	 //ʹ��PB�˿�ʱ��

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 		 //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		 //IO���ٶ�Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOB,GPIO_Pin_5);
	delay_ms(1);
	GPIO_SetBits(GPIOB,GPIO_Pin_5);
	
	// ��ʼ��IIC
	IIC_Init();
}

/**
 * BH1750 ����ģʽ
 * @param   
 * @return  0[�ɹ�] 1[ʧ��]
 * @brief 
 **/
u8 BH1750_SetMode(u8 cmd)
{
	IIC_Start();	//IIC��ʼ
	IIC_Send_Byte(BH1750FVI_ADDRESS);	//���͵�ַ
	if(IIC_Wait_Ack())
		return 1;
	IIC_Send_Byte(cmd);	//���Ͳ�������
	if(IIC_Wait_Ack())
		return 1;
	IIC_Stop();	//IIC����
	
	return 0;
}

/**
 * BH1750 ��ȡ����
 * @param   
 * @return 
 * @brief 
 **/
u16 BH1750_ReadData(void)
{
	u16 RxData;
	u8 temp;
	
	IIC_Start();	//IIC��ʼ
	IIC_Send_Byte(BH1750FVI_ADDRESS+1);	//���Ͷ���ַ
	if(IIC_Wait_Ack())
		return 0;
	RxData = IIC_Read_Byte(1);
	printf("H=%x ",RxData);
	RxData = (RxData << 8);
	temp = IIC_Read_Byte(0);
	printf("L=%x\r\n",temp);
	RxData = RxData|temp;
	IIC_Stop();	//IIC����
	
	return RxData;	
}

/**
 * ��ȡ���ݲ�ת��Ϊʵ�ʹ��ֵ
 * @param   
 * @return 
 * @brief ������1lx�ֱ���ģʽ
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
 * ���θ߷ֱ���ģʽ����
 * @param   
 * @return 
 * @brief 
 **/
u16 BH1750_ReadData_Once(void)
{
	u16 RxData;
	
	IIC_Start();	//IIC��ʼ
	IIC_Send_Byte(BH1750FVI_ADDRESS);	//���͵�ַ
	if(IIC_Wait_Ack())
		return 0;
	IIC_Send_Byte(BH1750FVI_H_MODE_ONCE);	//���Ͳ�������
	if(IIC_Wait_Ack())
		return 0;
	IIC_Stop();	//IIC����
	
	delay_ms(180);	//��ʱ180ms�ȴ�ת�����
	
	IIC_Start();	//IIC��ʼ
	IIC_Send_Byte(BH1750FVI_ADDRESS+1);	//���Ͷ���ַ
	if(IIC_Wait_Ack())
		return 0;
	RxData = IIC_Read_Byte(1);
	RxData = (RxData << 8);
	RxData = IIC_Read_Byte(0);
	IIC_Stop();	//IIC����
	
	return RxData;
}