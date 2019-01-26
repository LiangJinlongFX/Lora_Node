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
	u16 i;
	float val;
	
	tRadioDriver *Radio = NULL;
	struct bme280_data comp_data;
	
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 //串口初始化为115200
	SpiInit();
	LED_Init();
	Adc_Init();
	BH1750_Init();		//初始化光照传感器
	BH1750_SetMode(BH1750FVI_H_MODE_CONT);	//设置光照传感器为1lx连续监测模式
	BME280_Init(&Global_BME280);		//初始化环境传感器
	set_sensor_data_normal_mode(&Global_BME280);	//设置环境传感器模式
//	Radio = RadioDriverInit();
//	Radio->Init();
//	Radio->StartRx();
 	while(1)
	{
		printf("Temperature, Pressure, Humidity\r\n");
		bme280_get_sensor_data(BME280_ALL, &comp_data, &Global_BME280);
		print_sensor_data(&comp_data);
		printf("Light=%f\r\n",BH1750_ReadRealData());
		delay_ms(500);
	}
}

