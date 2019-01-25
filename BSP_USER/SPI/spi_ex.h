#ifndef __SPI_H
#define __SPI_H
/**
  * @file    spi.c
  * @author  Liang
  * @version V1.0.0
  * @date    2019-1-10
  * @brief	
  **/
#include "sys.h"

#define SPI2_NSS PBout(12)//PB12
 	  	    													  
void SPI2_Init(void);			 //初始化SPI口
void SPI2_SetSpeed(u8 SpeedSet); //设置SPI速度   
unsigned char SPI2_ReadWriteByte(unsigned char TxData);//SPI总线读写一个字节

uint8_t SPI2_ReadBuffer(uint8_t dev_id,uint8_t addr, uint8_t *buffer, uint8_t size );
uint8_t SPI2_WriteBuffer(uint8_t dev_id,uint8_t addr, uint8_t *buffer, uint8_t size );
		 
#endif

