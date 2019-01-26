#ifndef __MY_IIC2_H
#define __MY_IIC2_H
#include "sys.h"
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

//IO方向设置
 
#define SDA2_IN()  {GPIOB->CRH&=0X0FFFFFFF;GPIOB->CRH|=(u32)8<<28;}
#define SDA2_OUT() {GPIOB->CRH&=0X0FFFFFFF;GPIOB->CRH|=(u32)3<<28;}

//IO操作函数	 
#define IIC2_SCL    PBout(13) //SCL
#define IIC2_SDA    PBout(15) //SDA	
#define READ2_SDA   PBin(15)  //输入SDA 

//IIC所有操作函数
void IIC2_Init(void);                //初始化IIC的IO口				 
void IIC2_Start(void);				//发送IIC开始信号
void IIC2_Stop(void);	  			//发送IIC停止信号
void IIC2_Send_Byte(u8 txd);			//IIC发送一个字节
u8 IIC2_Read_Byte(unsigned char ack);//IIC读取一个字节
u8 IIC2_Wait_Ack(void); 							//IIC等待ACK信号
void IIC2_Ack(void);									//IIC发送ACK信号
void IIC2_NAck(void);									//IIC不发送ACK信号

#define BME280_READ_ADDR	0xED
#define BME280_WRITE_ADDR	0xEC

u8 IIC2_Read_1Byte(u8 SlaveAddress,u8 Reg_Address,u8 *Reg_Data);
u8 IIC2_Write_1Byte(u8 SlaveAddress,u8 REG_Address,u8 REG_data);

u8 IIC2_Write_nByte(u8 SlaveAddress, u8 REG_Address, u16 len, u8 *buf);
u8 IIC2_Read_nByte(u8 SlaveAddress, u8 REG_Address, u16 len, u8 *buf);

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);


#endif