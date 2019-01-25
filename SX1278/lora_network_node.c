#include "sys.h"
#include "radio.h"
#include "stdio.h"
#include "delay.h"
#include "lora_network.h"

#if defined(USED_LORANETWORK_NODE)

LoraNode_Data Global_SensorDataBuff;	//ȫ�ִ�����������
LocalNode_INFO Global_NodeInfo;

/**
 * lora ���ؽڵ�  �����ϴ��㲥���ں��ʵ�ʱ���ϴ�����
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_UploadData(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	
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
	while(timeout <= 500)
	{
		status = Radio->Process();
		if(status==RF_TX_DONE)
		{
			printf("RF_TX_DONE\r\n");
			Radio->StartRx();
		}
		if(status==RF_RX_DONE)
		{
			printf("RF_RX_DONE\r\n");
			Radio->GetRxPacket(Buffer, (uint16_t*)&buffer_size);
			//������յ������ݰ������ϼ��������ݰ���С
			if(buffer_size != sizeof(LoraNetwork_Gateway_Pack))
				continue;
			else
			{
				//��ԭ���������ݰ�
				lora_memcpy((void*)&rev_pack,(void*)Buffer,sizeof(LoraNetwork_Gateway_Pack));
				//���ԷǴ˽ڵ�ĵ������ݰ�
				if(rev_pack.node_id != node_info->LocalNode_Mac)
					continue;
				//���Բ���ע����Ӧ�����ݰ�
				if(rev_pack.pack_type != LORA_REGACK)
					continue;
				//��ȡע����Ϣ,������ȷ��־
				if(LoraNode_RegAck(&rev_pack,node_info) == LORA_OK)
				{
					//����һ����Ӧ
					LoraNode_SendAckPack(Radio,node_info);
					return LORA_OK;
				}
			}
		}
		delay_ms(10);
		timeout++;
	}
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
	LoraNetwork_Node_Pack send_pack;
	
	// ���������Ч��
	if(Radio==NULL || node_info->LocalNode_id == 0)
		return LORA_NODE_OFFLINE;
	// ��װ�������ݰ�
	send_pack.node_id = node_info->LocalNode_id;
	send_pack.pack_type = LORA_NODE_UPLOAD;
	send_pack.node_mac = node_info->LocalNode_Mac;
	send_pack.rev_delay_time = 0;
	send_pack.rev_Rssi = 0;
	send_pack.Load_length = sizeof(Global_SensorDataBuff);
	lora_memcpy(send_pack.Load,&Global_SensorDataBuff,sizeof(Global_SensorDataBuff));
	// �������ݰ�
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	
	return LORA_OK;
}

#endif
