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
	
	delay_init();	    	 //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //����NVIC�жϷ���2:2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	 //���ڳ�ʼ��Ϊ115200
	SpiInit();					 //��ʼLORASPI�ӿ�
	LED_Init();					 //��ʼLEDIO��
	Adc_Init();					 //��ʼ��ADC
	BH1750_Init();		   //��ʼ�����մ�����
	BH1750_SetMode(BH1750FVI_H_MODE_CONT);	//���ù��մ�����Ϊ1lx�������ģʽ
	status = BME280_Init(&Global_BME280);		//��ʼ������������
	/* ������������ʼ������,��Ƴ��� */
	if(status)
	{
		LEDR = 0;
		while(1);
	}
	set_sensor_data_normal_mode(&Global_BME280);	//���û���������ģʽ
	//��ʼ��LORAģ��
	Radio = RadioDriverInit();
	Radio->Init();
	//��ʼ���ڵ�Ӳ��Ψһ��ʶ
	Global_NodeInfo.LocalNode_Mac = LORA_LOCAL_MAC;
	//����ע���������
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
	/* �쳣״̬�˳�LORA�����߳� */
	while(1)
	{
		LEDR = 0;
		delay_ms(200);
		LEDR = 1;
		delay_ms(200);		
	}
}

