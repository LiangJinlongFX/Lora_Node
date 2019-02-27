#include "sys.h"
#include "radio.h"
#include "stdio.h"
#include "delay.h"
#include "lora_network.h"
/* ����������ͷ�ļ� */
#include "bh1750.h"
#include "adc.h"
#include "bme280_app.h"
#include "led.h"

#if defined(USED_LORANETWORK_NODE)

LocalNode_INFO Global_NodeInfo;

/**
 * lora ���ؽڵ�  �����ϴ��㲥���ں��ʵ�ʱ���ϴ�����
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_UploadData(tRadioDriver *Radio,uint8_t No)
{
	uint8_t status;
	uint8_t res;
	uint16_t buffer_size;
	uint16_t i;
	uint16_t overtime;
	uint8_t Buffer[400];
	LoraNetwork_Node_Pack rev_pack;
	
	overtime = 500*No;
	//����ǵ�һ������
	if(No == 0)
	{
		LEDB = 0;
		delay_ms(500);
		LEDB = 1;
		res=LoraNode_SendDataPack(Radio,&Global_NodeInfo);
		if(res != LORA_OK)
			printf("Upload Data ERROR\r\n");
		else 
			printf("Upload Data OK\r\n");
		return LORA_OK;
	}
	printf("max wait time: %dms\r\n",overtime);
	i=0;
	while(1)
	{
		/* �������ķ��ʹ���ʱ�� */
		if(i>overtime)
		{
			printf("wait upload timeout!!!\r\n");
			LEDB = 0;
			delay_ms(500);
			LEDB = 1;
			res=LoraNode_SendDataPack(Radio,&Global_NodeInfo);
			if(res != LORA_OK)
				printf("Upload Data ERROR\r\n");
			else 
				printf("Upload Data OK\r\n");
			break;
		}
		delay_ms(10);
		i++;
		status = Radio->Process();
		if(status==RF_TX_DONE)
		{
			Radio->StartRx();
		}
		else if(status==RF_RX_DONE)
		{
			Radio->GetRxPacket(Buffer, (uint16_t*)&buffer_size);
			//������ݰ����Ͻڵ����ݰ���С
			if(buffer_size == sizeof(LoraNetwork_Node_Pack))
			{
				//��ԭ�ڵ����ݰ�
				lora_memcpy((void*)&rev_pack,(void*)Buffer,sizeof(LoraNetwork_Node_Pack));
				//û�м�����ǰһ���ڵ㷢�����ݰ�
				printf("current node id: %d",rev_pack.node_id);
				if(rev_pack.node_id != (No-1))
				{
					Radio->StartRx();
					continue;
				}
				//������ǰһ���ڵ��ڴ�������,��ʱ��������
				else
				{
					LEDB = 0;
					delay_ms(500);
					LEDB = 1;
					res=LoraNode_SendDataPack(Radio,&Global_NodeInfo);
					if(res != LORA_OK)
						printf("Upload Data ERROR\r\n");
					else 
						printf("Upload Data OK\r\n");
					return LORA_OK;
				}
			}
		}
	}
	return 1;
}

/**
 * nodec����״̬��
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus Loranode_Process(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	uint8_t status;
	uint16_t buffer_size;
	uint8_t Buffer[400];
	LoraNetwork_Gateway_Pack rev_pack;
	
	Radio->StartRx();
	while(1)
	{
		status = Radio->Process();
		if(status==RF_TX_DONE)
		{
			printf("TX DONE\r\n");
			Radio->StartRx();
		}
		else if(status==RF_RX_DONE)
		{
			Radio->GetRxPacket(Buffer, (uint16_t*)&buffer_size);
			printf("RX DONE SIZE: %d\r\n",buffer_size);
			//������ݰ����ϼ��������ݰ���С
			if(buffer_size == sizeof(LoraNetwork_Gateway_Pack))
			{
				//��ԭ���������ݰ�
				lora_memcpy((void*)&rev_pack,(void*)Buffer,sizeof(LoraNetwork_Gateway_Pack));
				printf("rev gateway pack\r\n");
				LoraNode_GatewayPack_Process(Radio,node_info,&rev_pack);
				Radio->StartRx();
			}
		}
		LEDG = 0;
		delay_ms(10);
		LEDG = 1;
	}
}

/**
 * node�����������ݰ�
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_GatewayPack_Process(tRadioDriver *Radio,LocalNode_INFO *node_info,LoraNetwork_Gateway_Pack *rev_pack)
{
	uint8_t i,No;
	uint8_t node_list[20];
	
	//����ǹ㲥��
	if(rev_pack->node_id == 0)
	{
		//�������������
		if(rev_pack->pack_type == LORA_NODE_UPLOAD)
		{
			printf("gateway pack type: LORA_NODE_UPLOAD\r\n");
			//�������ݵ�ַ
			lora_memcpy(node_list,rev_pack->Load,rev_pack->Load_length);
			//��ȡ���ڵ��ϴ��������к�
			for(i=0;i<rev_pack->Load_length;i++)
			{
				if(node_list[i] == (node_info->LocalNode_Mac))
					break;
			}
			No = i;
			node_info->LocalNode_id = No;
			printf("The Node id: %d\r\n",No);
			LoraNode_UploadData(Radio,No);
			return LORA_OK;
		}		
	}
	
	return LORA_OK;
}

/**
 * lora ���ؽڵ�  ע����������
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_Registe(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	uint8_t status;
	uint16_t timeout;
	uint16_t buffer_size;
	uint8_t Buffer[100];
	LoraNetwork_Gateway_Pack rev_pack;
	
	//��ʼ���ڵ�Ӳ��Ψһ��ʶ
	node_info->LocalNode_Mac = LORA_LOCAL_MAC;
	//��ʼ���ڵ�Ϊ����״̬
	node_info->LocalNode_Status = NODE_STATUS_OFFLINE;
	
	// ������������
	LoraNode_SendRegPack(Radio,node_info);
	printf("send reg_pack\r\n");
	// �ȴ�ע����Ӧ��Ϣ
	timeout=0;
	while(timeout <= 1000)
	{
		status = Radio->Process();
		// ��lora�������ݳɹ����������ؽ���״̬
		if(status == RF_TX_DONE)
		{
			printf("TX DONE\r\n");
			Radio->StartRx();
		}
		else if(status==RF_RX_DONE)
		{
			printf("RF_RX_DONE\r\n");
			Radio->GetRxPacket(Buffer, (uint16_t*)&buffer_size);
			//������յ������ݰ������ϼ��������ݰ���С
			if(buffer_size != sizeof(LoraNetwork_Gateway_Pack))
			{
				printf("No Gateway_Pack\r\n");
				Radio->StartRx();
				continue;
			}
			else
			{
				//��ԭ���������ݰ�
				lora_memcpy((void*)&rev_pack,(void*)Buffer,sizeof(LoraNetwork_Gateway_Pack));
				//���ԷǴ˽ڵ�ĵ������ݰ�
				if(rev_pack.node_id != node_info->LocalNode_Mac)
				{
					printf("RX Continue\r\n");
					Radio->StartRx();
					continue;
				}
				//���Բ���ע����Ӧ�����ݰ�
				if(rev_pack.pack_type != LORA_REGACK)
				{
					printf("RX Continue\r\n");
					Radio->StartRx();
					continue;
				}
				//��ȡע����Ϣ,������ȷ��־
				if(LoraNode_RegAck(&rev_pack,node_info) == LORA_OK)
				{
					/* ����ʱ�Ǳ����!!!,�ȴ�gateway�л�Ϊ����״̬�ŷ��� */
					delay_ms(500);
					//����һ����Ӧ
					printf("Send ACK Pack\r\n");
					LoraNode_SendAckPack(Radio,node_info);
					return LORA_OK;
				}
			}
		}
		delay_ms(10);
		timeout++;
	}
	printf("time out\r\n");
	//��ʱ���ش���״̬
	return LORA_SYSTEM_ERROR;
}
	
/**
 * lora ���ؽڵ�  ����ע����������
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_SendRegPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// ���������Ч��
	if(Radio==NULL || node_info->LocalNode_id != 0)
		return LORA_SYSTEM_ERROR;
	// ��װע�����������
	send_pack.node_id = node_info->LocalNode_Mac;		//���ݰ�
	send_pack.pack_type = LORA_REG;									//ע������
	send_pack.node_mac = node_info->LocalNode_Mac;	//
	// �������ݰ�
	Radio->SetTxPacket(&send_pack,sizeof(LoraNetwork_Node_Pack));
	
	return LORA_OK;
}


/**
 * lora ���ؽڵ�  ����ע����Ӧ
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_RegAck(LoraNetwork_Gateway_Pack *rev_pack,LocalNode_INFO *node_info)
{
	
	// ����Ѿ�����ע��״̬��ֱ�ӷ���
	if(node_info->LocalNode_Status == NODE_STATUS_REG)
		return LORA_OK;
	// ��ȡ������ϢΪ����������ı��ڵ�����
	lora_memcpy(node_info->LocalNode_Name,rev_pack->Load,rev_pack->Load_length);
	// ���Ϊע��״̬
	node_info->LocalNode_Status = NODE_STATUS_REG;
	
	return LORA_OK;
}

/**
 * lora ���ؽڵ�  ����һ����Ӧ
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_SendAckPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// ���������Ч��
	if(Radio==NULL || node_info->LocalNode_Status == NODE_STATUS_OFFLINE)
		return LORA_NODE_OFFLINE;
	// ��װһ����Ӧ��
	send_pack.node_id = node_info->LocalNode_Mac;
	send_pack.pack_type = LOEA_ACK;
	send_pack.node_mac = node_info->LocalNode_Mac;
	// �������ݰ�
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	
	return LORA_OK;	
}

/**
 * lora ���ؽڵ��ϴ�����������
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_SendDataPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	uint16_t i=0;
	uint8_t lora_status;
	LoraNetwork_Node_Pack send_pack;
	LoraNode_Data SensorData;
	
	// ���������Ч��
	if(Radio==NULL)
		return LORA_NODE_OFFLINE;
	// ��ȡ����������
	LoraNode_GetSensorData(&SensorData);
	//��ȡ�ڵ�MAC
	SensorData.node_mac = node_info->LocalNode_Mac;
	
	// ��װ�������ݰ�
	send_pack.node_id = node_info->LocalNode_id;
	send_pack.pack_type = LORA_NODE_UPLOAD;
	send_pack.node_mac = node_info->LocalNode_Mac;
	send_pack.rev_delay_time = 0;
	send_pack.rev_Rssi = 0;
	send_pack.Load_length = sizeof(SensorData);
	lora_memcpy(send_pack.Load,&SensorData,sizeof(SensorData));
	printf("pack size: %d %d \r\n",sizeof(SensorData),sizeof(send_pack));
	// �������ݰ�
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	while(Radio->Process() != RF_TX_DONE)
	{
		delay_ms(10);
		i++;
		if(i>300)
		{
			printf("upload data failed!!!\r\n");
			return LORA_NODE_OFFLINE;
		}
	}
	
	return LORA_OK;
}

/**
 * ��ȡ����������
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_GetSensorData(LoraNode_Data *SensorData_Buff)
{
	struct bme280_data comp_data;
	
	//��ȡ����ǿ��
	SensorData_Buff->LX = BH1750_ReadRealData();
	//��ȡ��������
	bme280_get_sensor_data(BME280_ALL, &comp_data, &Global_BME280);
	SensorData_Buff->temp = comp_data.temperature;
	SensorData_Buff->pressure = comp_data.pressure;
	SensorData_Buff->hum = comp_data.humidity;
	//��ȡTVOC����
	SensorData_Buff->TVOC = Get_TVOC_Val();
	//��ȡ��ص�ѹ
	SensorData_Buff->node_VBAT = (int)Get_VBAT()*100;
	//�޸�����Ϣ
	SensorData_Buff->other_info = 0;
	printf("temp : %f\r\n",SensorData_Buff->temp);
	printf("rh   : %f\r\n",SensorData_Buff->hum);
	printf("press: %f\r\n",SensorData_Buff->pressure);
	printf("lx   : %f\r\n",SensorData_Buff->LX);
	printf("tvoc : %f\r\n",SensorData_Buff->TVOC);
	printf("vbat : %d\r\n",SensorData_Buff->node_VBAT);
	
	return LORA_OK;
}

#endif
