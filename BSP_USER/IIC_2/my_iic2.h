#ifndef __MY_IIC2_H
#define __MY_IIC2_H
#include "sys.h"
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

//IO��������
 
#define SDA2_IN()  {GPIOB->CRH&=0X0FFFFFFF;GPIOB->CRH|=(u32)8<<28;}
#define SDA2_OUT() {GPIOB->CRH&=0X0FFFFFFF;GPIOB->CRH|=(u32)3<<28;}

//IO��������	 
#define IIC2_SCL    PBout(13) //SCL
#define IIC2_SDA    PBout(15) //SDA	
#define READ2_SDA   PBin(15)  //����SDA 

//IIC���в�������
void IIC2_Init(void);                //��ʼ��IIC��IO��				 
void IIC2_Start(void);				//����IIC��ʼ�ź�
void IIC2_Stop(void);	  			//����IICֹͣ�ź�
void IIC2_Send_Byte(u8 txd);			//IIC����һ���ֽ�
u8 IIC2_Read_Byte(unsigned char ack);//IIC��ȡһ���ֽ�
u8 IIC2_Wait_Ack(void); 							//IIC�ȴ�ACK�ź�
void IIC2_Ack(void);									//IIC����ACK�ź�
void IIC2_NAck(void);									//IIC������ACK�ź�

#define BME280_READ_ADDR	0xED
#define BME280_WRITE_ADDR	0xEC

u8 IIC2_Read_1Byte(u8 SlaveAddress,u8 Reg_Address,u8 *Reg_Data);
u8 IIC2_Write_1Byte(u8 SlaveAddress,u8 REG_Address,u8 REG_data);

u8 IIC2_Write_nByte(u8 SlaveAddress, u8 REG_Address, u16 len, u8 *buf);
u8 IIC2_Read_nByte(u8 SlaveAddress, u8 REG_Address, u16 len, u8 *buf);

int8_t user_i2c_write(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);
int8_t user_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint8_t *reg_data, uint16_t len);


#endif