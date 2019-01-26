#include "my_iic2.h"
#include "delay.h"
#include "stdio.h"
//////////////////////////////////////////////////////////////////////////////////	 
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//ALIENTEK战舰STM32开发板
//IIC驱动 代码	   
//正点原子@ALIENTEK
//技术论坛:www.openedv.com
//修改日期:2012/9/9
//版本：V1.0
//版权所有，盗版必究。
//Copyright(C) 广州市星翼电子科技有限公司 2009-2019
//All rights reserved									  
//////////////////////////////////////////////////////////////////////////////////
 
//初始化IIC
void IIC2_Init(void)
{					     
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	//使能GPIOB时钟
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_15); 	//PB13,PB15 输出高
	
}

//产生IIC起始信号
void IIC2_Start(void)
{
	SDA2_OUT();     //sda线输出
	IIC2_SDA=1;	  	  
	IIC2_SCL=1;
	delay_us(4);
 	IIC2_SDA=0;	//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC2_SCL=0;	//钳住I2C总线，准备发送或接收数据 
}	

//产生IIC停止信号
void IIC2_Stop(void)
{
	SDA2_OUT();		//sda线输出
	IIC2_SCL=0;
	IIC2_SDA=0;		//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC2_SCL=1; 
	delay_us(4);
	IIC2_SDA=1;		//发送I2C总线结束信号						   	
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC2_Wait_Ack(void)
{
	u16 ucErrTime=0;
	
	SDA2_OUT();
	IIC2_SDA=1;delay_us(1);	   
	IIC2_SCL=1;delay_us(1);
	SDA2_IN();      //SDA设置为输入  	
	while(READ2_SDA)
	{
		ucErrTime++;
		if(ucErrTime>250)
		{
			IIC2_Stop();
			return 1;
		}
	}
	IIC2_SCL=0;//时钟输出0
	delay_us(1);
	return 0;  
} 

//产生ACK应答
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
//不产生ACK应答		    
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

//IIC发送一个字节
//返回从机有无应答
//1，有应答
//0，无应答			  
void IIC2_Send_Byte(u8 txd)
{                        
  u8 t;   
	SDA2_OUT(); 	    
  IIC2_SCL=0;//拉低时钟开始数据传输
  for(t=0;t<8;t++)
  {              
        //IIC_SDA=(txd&0x80)>>7;
		if((txd&0x80)>>7)
			IIC2_SDA=1;
		else
			IIC2_SDA=0;
		txd<<=1; 	  
		delay_us(3);   //对TEA5767这三个延时都是必须的
		IIC2_SCL=1;
		delay_us(3); 
		IIC2_SCL=0;	
		delay_us(3);
   }	 
} 


//读1个字节，ack=1时，发送ACK，ack=0，发送nACK   
u8 IIC2_Read_Byte(unsigned char ack)
{
	unsigned char i,receive=0;
	SDA2_IN();//SDA设置为输入
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
        IIC2_NAck();//发送nACK
    else
        IIC2_Ack(); //发送ACK   
    return receive;
}

u8 IIC2_Read_1Byte(u8 SlaveAddress,u8 Reg_Address,u8 *Reg_Data)
{
	IIC2_Start();	//IIC开始
	IIC2_Send_Byte(SlaveAddress<<1);	//发送写地址
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
	IIC2_Start();	//IIC开始
	IIC2_Send_Byte(SlaveAddress<<1 | 0x01);	//发送读地址
	if(IIC2_Wait_Ack())
	{
		printf("FAIL\r\n");
		return -4;
	}
	*Reg_Data = IIC2_Read_Byte(0);
	IIC2_Stop();									//IIC结束
	
}

u8 IIC2_Write_1Byte(u8 SlaveAddress,u8 REG_Address,u8 REG_data)
{
	IIC2_Start();	//IIC开始
	IIC2_Send_Byte(SlaveAddress<<1);	//发送写地址
	if(IIC2_Wait_Ack())
	{
		printf("FAIL\r\n");
//		return -4;
	}
	IIC2_Send_Byte(REG_Address);
	IIC2_Wait_Ack();
	IIC2_Send_Byte(REG_data);
	IIC2_Wait_Ack();
	IIC2_Stop();									//IIC结束		
}

/****************** API函数 ***************************/
/**
 * IIC2写多字节
 * @param   
 * @return 
 * @brief 
 **/
u8 IIC2_Write_nByte(u8 SlaveAddress, u8 REG_Address, u16 len, u8 *buf)
{	
	IIC2_Start();	//IIC开始
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
	IIC2_Stop();									//IIC结束	
	return 0;
}
/**
 * IIC2读多字节
 * @param   
 * @return 
 * @brief 
 **/
u8 IIC2_Read_nByte(u8 SlaveAddress, u8 REG_Address, u16 len, u8 *buf)
{	
	IIC2_Start();	//IIC开始
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
	IIC2_Start();	//IIC开始
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
	IIC2_Stop();									//IIC结束	
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