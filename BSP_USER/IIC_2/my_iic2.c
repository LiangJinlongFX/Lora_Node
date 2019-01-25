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
	RCC_APB2PeriphClockCmd(	RCC_APB2Periph_GPIOB, ENABLE );	//使能GPIOB时钟
	   
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP ;   //推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB,GPIO_Pin_12|GPIO_Pin_13|GPIO_Pin_15);
	GPIO_ResetBits(GPIOB,GPIO_Pin_14);		//地址位为0
}

//产生IIC起始信号
void IIC2_Start(void)
{
	SDA2_OUT();     //sda线输出
	IIC2_SDA=1;	  	  
	IIC2_SCL=1;
	delay_us(4);
 	IIC2_SDA=0;//START:when CLK is high,DATA change form high to low 
	delay_us(4);
	IIC2_SCL=0;//钳住I2C总线，准备发送或接收数据 
}	

//产生IIC停止信号
void IIC2_Stop(void)
{
	SDA2_OUT();//sda线输出
	IIC2_SCL=0;
	IIC2_SDA=0;//STOP:when CLK is high DATA change form low to high
 	delay_us(4);
	IIC2_SCL=1; 
	IIC2_SDA=1;//发送I2C总线结束信号
	delay_us(4);							   	
}

//等待应答信号到来
//返回值：1，接收应答失败
//        0，接收应答成功
u8 IIC2_Wait_Ack(void)
{
	u16 ucErrTime=0;
	SDA2_IN();      //SDA设置为输入  	
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
	IIC2_SCL=0;//时钟输出0 	   
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
		delay_us(5);   //对TEA5767这三个延时都是必须的
		IIC2_SCL=1;
		delay_us(5); 
		IIC2_SCL=0;	
		delay_us(5);
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

int8_t II2_ReadData(uint8_t id,uint8_t addr,uint8_t *reg_data,uint8_t len)
{
	u8 i;
	
	IIC2_Start();	//IIC开始
	IIC2_Send_Byte(0xec);	//发送写地址
	if(IIC2_Wait_Ack())
	{
		printf("COMM FAIL\r\n");
		return -4;
	}
	IIC2_Send_Byte(addr);								//发送要写的寄存器地址
	if(IIC2_Wait_Ack())
		return -4;
	IIC2_Send_Byte(BME280_READ_ADDR);		//发送读地址
	if(IIC2_Wait_Ack())
		return -4;
	for(i=0;i<len;i++)
	{
		if(i >= len-1)
			*reg_data++ = IIC2_Read_Byte(0);
		else
			*reg_data++ = IIC2_Read_Byte(1);
	}
	IIC2_Stop();													//IIC结束
	
	return 0;
}


int8_t II2_WriteData(uint8_t id,uint8_t addr,uint8_t *reg_data,uint8_t len)
{
	u8 i;
	
	IIC2_Start();	//IIC开始
	IIC2_Send_Byte(BME280_WRITE_ADDR);	//发送写地址
	if(IIC2_Wait_Ack())
		return -4;
	IIC2_Send_Byte(addr);								//发送要写的寄存器地址
	if(IIC2_Wait_Ack())
		return -4;
	IIC2_Send_Byte(BME280_READ_ADDR);		//发送写寄存器地址
	if(IIC2_Wait_Ack())
		return -4;
	IIC2_Send_Byte(*reg_data);					//发送要写的寄存器数据
	if(IIC2_Wait_Ack())
	return -4;
	IIC2_Stop();												//IIC结束
	
	return 0;
}