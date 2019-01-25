#include "sys.h"
#include "usart1.h"
#include "delay.h"
#include "led.h"
#include "bme280.h"
#include "spi.h"
#include "radio.h"
#include "bh1750.h"
#include "adc.h"
#include "my_iic2.h"
#include "lora_network.h"
 
int main(void)
{
	u16 i;
	float val;
	u8 udata;
	u8 status;
	struct bme280_dev dev;
	int8_t rslt = BME280_OK;
	
	tRadioDriver *Radio = NULL;
	
	delay_init();	    	 //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //设置NVIC中断分组2:2位抢占优先级，2位响应优先级
	uart_init(115200);	 //串口初始化为115200
	BH1750_Init();
	SpiInit();
	LED_Init();
	Adc_Init();
	IIC2_Init();
	BH1750_SetMode(BH1750FVI_H_MODE_CONT);
	
	Radio = RadioDriverInit();
	Radio->Init();
	Radio->StartRx();
	
//	dev.dev_id = BME280_I2C_ADDR_PRIM;
//	dev.intf = BME280_I2C_INTF;
//	dev.read = SPI2_ReadBuffer;
//	dev.write = SPI2_WriteBuffer;
//	dev.delay_ms = delay_ms;
//	
//	rslt = bme280_init(&dev);

//	printf("%d\r\n",udata);
//	II2_ReadData(0,0xd0,&udata,1);
//	printf("%x\r\n",udata);
	
 	while(1)
	{
//		i=BH1750_ReadData();
//		printf("val=%d\r\n",i);
//		LEDR = 0;
//		LEDG = 0;
//		LEDB = 0;
//		delay_ms(500);
//		LEDR = 1;
//		LEDG = 1;
//		LEDB = 1;
//		delay_ms(500);
//	udata = 128;
//	II2_ReadData(0,0xd0,&udata,1);
//	printf("%x\r\n",udata);
		if(Radio->Process( ) == RF_RX_DONE)
		{
			LEDG = 0;
			delay_ms(300);
			LEDG = 1;
			Radio->StartRx();
		}
	}
}

