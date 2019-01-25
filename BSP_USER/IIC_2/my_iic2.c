#include "my_iic2.h"
#include "delay.h"
#include "stdio.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEKս��STM32������
//IIC���� ����	   
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//�޸�����:2012/9/9
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
 
//��ʼ��IIC
void IIC2_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//ʹ��GPIOBʱ��
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //�������
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15);
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);		//��ַλΪ0
}

//����IIC��ʼ�ź�
void IIC2_Start(void)
{
	SDA2_OUT();     //sda�����
	IIC2_SDA=1;	  	  
	IIC2_SCL=1;
	delay_us(4);
 	IIC2_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC2_SCL=0;//ǯסI2C���ߣ�׼�����ͻ�������� 
}	

//����IICֹͣ�ź�
void IIC2_Stop(void)
{
	SDA2_OUT();//sda�����
	IIC2_SCL=0;
	IIC2_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC2_SCL=1; 
	IIC2_SDA=1;//����I2C���߽����ź�
	delay_us(4);							   	
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC2_Wait_Ack(void)
{
	u16 ucErrTime=0;
	SDA2_IN();      //SDA����Ϊ����  	
	IIC2_SDA=1;delay_us(1);	   
	IIC2_SCL=1;delay_us(1);
	while(READ2_SDA)
	{
		ucErrTime++;
		if(ucErrTime>500)
		{
			IIC2_Stop();
			return 1;
		}
	}
	IIC2_SCL=0;//ʱ�����0 	   
	return 0;  
} 
//����ACKӦ��
void IIC2_Ack(void)
{
	IIC2_SCL=0;
	SDA2_OUT();
	IIC2_SDA=0;
	delay_us(2);
	IIC2_SCL=1;
	delay_us(2);
	IIC2_SCL=0;
}
//������ACKӦ��		    
void IIC2_NAck(void)
{
	IIC2_SCL=0;
	SDA2_OUT();
	IIC2_SDA=1;
	delay_us(2);
	IIC2_SCL=1;
	delay_us(2);
	IIC2_SCL=0;
}		

//IIC����һ���ֽ�
//���شӻ�����Ӧ��
//1����Ӧ��
//0����Ӧ��			  
void IIC2_Send_Byte(u8 txd)
{                        
    u8 t;   
		SDA2_OUT(); 	    
    IIC2_SCL=0;//����ʱ�ӿ�ʼ���ݴ���
    for(t=0;t<8;t++)
    {              
        //IIC_SDA=(txd&0x80)>>7;
		if((txd&0x80)>>7)
			IIC2_SDA=1;
		else
			IIC2_SDA=0;
		txd<<=1; 	  
		delay_us(5);   //��TEA5767��������ʱ���Ǳ����
		IIC2_SCL=1;
		delay_us(5); 
		IIC2_SCL=0;	
		delay_us(5);
    }	 
} 


//��1���ֽڣ�ack=1ʱ������ACK��ack=0������nACK   
u8 IIC2_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA2_IN();//SDA����Ϊ����
    for(i=0;i<8;i++ )
	{
        IIC2_SCL=0; 
        delay_us(3);
				IIC2_SCL=1;
        receive<<=1;
        if(READ2_SDA)receive++;   
		delay_us(3); 
    }					 
    if (!ack)
        IIC2_NAck();//����nACK
    else
        IIC2_Ack(); //����ACK   
    return receive;
}

int8_t II2_ReadData(uint8_t id,uint8_t addr,uint8_t *reg_data,uint8_t len)
{
	u8 i;
	
	IIC2_Start();	//IIC��ʼ
	IIC2_Send_Byte(0xec);	//����д��ַ
	if(IIC2_Wait_Ack())
	{
		printf("COMM FAIL\r\n");
		return -4;
	}
	IIC2_Send_Byte(addr);								//����Ҫд�ļĴ�����ַ
	if(IIC2_Wait_Ack())
		return -4;
	IIC2_Send_Byte(BME280_READ_ADDR);		//���Ͷ���ַ
	if(IIC2_Wait_Ack())
		return -4;
	for(i=0;i<len;i++)
	{
		if(i >= len-1)
			*reg_data++ = IIC2_Read_Byte(0);
		else
			*reg_data++ = IIC2_Read_Byte(1);
	}
	IIC2_Stop();													//IIC����
	
	return 0;
}


int8_t II2_WriteData(uint8_t id,uint8_t addr,uint8_t *reg_data,uint8_t len)
{
	u8 i;
	
	IIC2_Start();	//IIC��ʼ
	IIC2_Send_Byte(BME280_WRITE_ADDR);	//����д��ַ
	if(IIC2_Wait_Ack())
		return -4;
	IIC2_Send_Byte(addr);								//����Ҫд�ļĴ�����ַ
	if(IIC2_Wait_Ack())
		return -4;
	IIC2_Send_Byte(BME280_READ_ADDR);		//����д�Ĵ�����ַ
	if(IIC2_Wait_Ack())
		return -4;
	IIC2_Send_Byte(*reg_data);					//����Ҫд�ļĴ�������
	if(IIC2_Wait_Ack())
	return -4;
	IIC2_Stop();												//IIC����
	
	return 0;
}