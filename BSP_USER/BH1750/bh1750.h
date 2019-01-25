#ifndef _BH1750_H
#define _BH1750_H

#include "sys.h"

//BH1750FVI��ַ
#define BH1750FVI_ADDRESS 0x46	//ADDR="L"
//BH1750FVIָ�
#define BH1750FVI_POWERDOWN 0x00	//�ϵ�
#define BH1750FVI_POWERON		0x01	//�ϵ�
#define BH1750FVI_RESET			0x07	//����
#define BH1750FVI_H_MODE_CONT  0x10	//����H�ֱ���ģʽ   1lx
#define BH1750FVI_H_MODE2_CONT 0x11	//����H�ֱ���ģʽ2  0.5lx
#define BH1750FVI_L_MODE_CONT  0x13	//����L�ֱ���ģʽ   41lx
#define BH1750FVI_H_MODE_ONCE  0x20	//����H�ֱ���ģʽ  �������Ϊ�ϵ�ģʽ
#define BH1750FVI_H_MODE2_ONCE 0x21	//����H�ֱ���ģʽ2  �������Ϊ�ϵ�ģʽ
#define BH1750FVI_L_MODE_ONCE  0x23	//����L�ֱ���ģʽ  �������Ϊ�ϵ�ģʽ

void BH1750_Init(void);
u8 BH1750_SetMode(u8 cmd);
u16 BH1750_ReadData(void);
u16 BH1750_ReadData_Once(void);
float BH1750_ReadRealData(void);

#endif