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
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	//ʹ��GPIOBʱ��
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_15); 	//PB13,PB15 �����
	
}

//����IIC��ʼ�ź�
void IIC2_Start(void)
{
	SDA2_OUT();     //sda�����
	IIC2_SDA=1;	  	  
	IIC2_SCL=1;
	delay_us(4);
 	IIC2_SDA=0;	//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC2_SCL=0;	//ǯסI2C���ߣ�׼�����ͻ�������� 
}	

//����IICֹͣ�ź�
void IIC2_Stop(void)
{
	SDA2_OUT();		//sda�����
	IIC2_SCL=0;
	IIC2_SDA=0;		//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC2_SCL=1; 
	delay_us(4);
	IIC2_SDA=1;		//����I2C���߽����ź�						   	
}

//�ȴ�Ӧ���źŵ���
//����ֵ��1������Ӧ��ʧ��
//        0������Ӧ��ɹ�
u8 IIC2_Wait_Ack(void)
{
	u16 ucErrTime=0;
	
	SDA2_OUT();
	IIC2_SDA=1;delay_us(1);	   
	IIC2_SCL=1;delay_us(1);
	SDA2_IN();      //SDA����Ϊ����  	
	while(READ2_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC2_Stop();
			return 1;
		}
	}
	IIC2_SCL=0;//ʱ�����0
	delay_us(1);
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
		delay_us(3);   //��TEA5767��������ʱ���Ǳ����
		IIC2_SCL=1;
		delay_us(3); 
		IIC2_SCL=0;	
		delay_us(3);
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

u8 IIC2_Read_1Byte(u8 SlaveAddress,u8 Reg_Address,u8 *Reg_Data)
{
	IIC2_Start();	//IIC��ʼ
	IIC2_Send_Byte(SlaveAddress<<1);	//����д��ַ
	if(IIC2_Wait_Ack())
	{
		printf("FAIL\r\n");
		return -4;
	}
	printf("send:%x\r\n",Reg_Address);
	IIC2_Send_Byte(Reg_Address);
	if(IIC2_Wait_Ack())
	{
		printf("FAIL\r\n");
		return -4;
	}
	IIC2_Start();	//IIC��ʼ
	IIC2_Send_Byte(SlaveAddress<<1 | 0x01);	//���Ͷ���ַ
	if(IIC2_Wait_Ack())
	{
		printf("FAIL\r\n");
		return -4;
	}
	*Reg_Data = IIC2_Read_Byte(0);
	IIC2_Stop();									//IIC����
	
}

u8 IIC2_Write_1Byte(u8 SlaveAddress,u8 REG_Address,u8 REG_data)
{
	IIC2_Start();	//IIC��ʼ
	IIC2_Send_Byte(SlaveAddress<<1);	//����д��ַ
	if(IIC2_Wait_Ack())
	{
		printf("FAIL\r\n");
//		return -4;
	}
	IIC2_Send_Byte(REG_Address);
	IIC2_Wait_Ack();
	IIC2_Send_Byte(REG_data);
	IIC2_Wait_Ack();
	IIC2_Stop();									//IIC����		
}

/****************** API���� ***************************/
/**
 * IIC2д���ֽ�
 * @param   
 * @return 
 * @brief 
 **/
u8 IIC2_Write_nByte(u8 SlaveAddress, u8 REG_Address, u16 len, u8 *buf)
{	
	IIC2_Start();	//IIC��ʼ
	IIC2_Send_Byte(SlaveAddress<<1); 
	if(IIC2_Wait_Ack())
	{
		return 1;
	}
	IIC2_Send_Byte(REG_Address); 
	if(IIC2_Wait_Ack())
	{
		return 1;
	}
	while(len--) 
	{
		IIC2_Send_Byte(*buf++); 
		IIC2_Wait_Ack();
	}
	IIC2_Stop();									//IIC����	
	return 0;
}
/**
 * IIC2�����ֽ�
 * @param   
 * @return 
 * @brief 
 **/
u8 IIC2_Read_nByte(u8 SlaveAddress, u8 REG_Address, u16 len, u8 *buf)
{	
	IIC2_Start();	//IIC��ʼ
	IIC2_Send_Byte(SlaveAddress<<1); 
	if(IIC2_Wait_Ack())
	{
		return 1;
	}
	IIC2_Send_Byte(REG_Address); 
	if(IIC2_Wait_Ack())
	{
		return 1;
	}	
	IIC2_Start();	//IIC��ʼ
	IIC2_Send_Byte(SlaveAddress<<1 | 0x01); 
	if(IIC2_Wait_Ack())
	{
		return 1;
	}	
	while(len) 
	{
		if(len == 1)
		{
			*buf = IIC2_Read_Byte(0);
		}
		else
		{
			*buf = IIC2_Read_Byte(1);
		}
		buf++;
		len--;
	}
	IIC2_Stop();									//IIC����	
	return 0;
}

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	return IIC2_Write_nByte(dev_id, reg_addr, len, reg_data);
}

int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len)
{
	return IIC2_Read_nByte(dev_id, reg_addr, len, reg_data);
}