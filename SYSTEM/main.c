#include "sys.h"
#include "usart1.h"
#include "delay.h"
#include "led.h"
#include "spi.h"
#include "radio.h"
#include "bh1750.h"
#include "adc.h"
#include "lora_network.h"
#include "bme280_app.h"
 
int main(void)
{
	u8 status;
	
	tRadioDriver *Radio = NULL;
	
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 //串口初始化为115200
	SpiInit();					 //初始LORASPI接口
	LED_Init();					 //初始LEDIO口
	Adc_Init();					 //初始化ADC
	BH1750_Init();		   //初始化光照传感器
	BH1750_SetMode(BH1750FVI_H_MODE_CONT);	//设置光照传感器为1lx连续监测模式
	status = BME280_Init(&Global_BME280);		//初始化环境传感器
	/* 如果存在外设初始化错误,红灯常亮 */
	if(status)
	{
		LEDR = 0;
		while(1);
	}
	set_sensor_data_normal_mode(&Global_BME280);	//设置环境传感器模式
	//初始化LORA模块
	Radio = RadioDriverInit();
	Radio->Init();
	//初始化节点硬件唯一标识
	Global_NodeInfo.LocalNode_Mac = LORA_LOCAL_MAC;
	//发起注册请求过程
	printf("start registe!\r\n");
	status = LoraNode_Registe(Radio,&Global_NodeInfo);
	if(status == LORA_SYSTEM_ERROR)
	{
		printf("Registe TIMEOUT!!!\r\n");
		while(1)
		{
			LEDR = 0;
			delay_ms(200);
			LEDR = 1;
			delay_ms(200);					
		}
	}
	printf("start rev gateway\r\n");
	Loranode_Process(Radio,&Global_NodeInfo);
	/* 异常状态退出LORA处理线程 */
	while(1)
	{
		LEDR = 0;
		delay_ms(200);
		LEDR = 1;
		delay_ms(200);		
	}
}

