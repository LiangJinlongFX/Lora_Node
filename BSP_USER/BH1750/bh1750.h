#ifndef _BH1750_H
#define _BH1750_H

#include "sys.h"

//BH1750FVI地址
#define BH1750FVI_ADDRESS 0x46	//ADDR="L"
//BH1750FVI指令集
#define BH1750FVI_POWERDOWN 0x00	//断电
#define BH1750FVI_POWERON		0x01	//上电
#define BH1750FVI_RESET			0x07	//重置
#define BH1750FVI_H_MODE_CONT  0x10	//连续H分辨率模式   1lx
#define BH1750FVI_H_MODE2_CONT 0x11	//连续H分辨率模式2  0.5lx
#define BH1750FVI_L_MODE_CONT  0x13	//连续L分辨率模式   41lx
#define BH1750FVI_H_MODE_ONCE  0x20	//单次H分辨率模式  测量后变为断电模式
#define BH1750FVI_H_MODE2_ONCE 0x21	//单次H分辨率模式2  测量后变为断电模式
#define BH1750FVI_L_MODE_ONCE  0x23	//单次L分辨率模式  测量后变为断电模式

void BH1750_Init(void);
u8 BH1750_SetMode(u8 cmd);
u16 BH1750_ReadData(void);
u16 BH1750_ReadData_Once(void);
float BH1750_ReadRealData(void);

#endif