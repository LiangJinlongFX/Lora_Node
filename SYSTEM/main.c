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
	
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
	SpiInit();
	LED_Init();
	Adc_Init();
	BH1750_Init();		//��ʼ�����մ�����
	BH1750_SetMode(BH1750FVI_H_MODE_CONT);	//���ù��մ�����Ϊ1lx�������ģʽ
	BME280_Init(&Global_BME280);		//��ʼ������������
	set_sensor_data_normal_mode(&Global_BME280);	//���û���������ģʽ
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

