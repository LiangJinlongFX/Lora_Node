#ifndef __DELAY_H
#define __DELAY_H
/**
  * @file    delay.h
  * @author  Liang
  * @version V1.0.0
  * @date    2019-1-10
  * @brief	 适用于STM32F103系列
  **/

#include "sys.h"  

	 
void delay_init(void);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#endif





























