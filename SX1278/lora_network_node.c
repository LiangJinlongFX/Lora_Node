#include "sys.h"
#include "radio.h"
#include "stdio.h"
#include "delay.h"
#include "lora_network.h"

#if defined(USED_LORANETWORK_NODE)

LoraNode_Data Global_SensorDataBuff;	//全局传感器缓存区
LocalNode_INFO Global_NodeInfo;

/**
 * lora 本地节点  监听上传广播并在合适的时间上传数据
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_UploadData(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	
}

/**
 * lora 本地节点  注册入网过程
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
	
	//初始化节点硬件唯一标识
	node_info->LocalNode_Mac = LORA_LOCAL_MAC;
	//初始化节点为离线状态
	node_info->LocalNode_Status = NODE_STATUS_OFFLINE;
	
	// 发送入网请求
	LoraNode_SendRegPack(Radio,node_info);
	printf("send reg_pack\r\n");
	// 等待注册响应信息
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
			//如果接收到的数据包不符合集中器数据包大小
			if(buffer_size != sizeof(LoraNetwork_Gateway_Pack))
				continue;
			else
			{
				//还原集中器数据包
				lora_memcpy((void*)&rev_pack,(void*)Buffer,sizeof(LoraNetwork_Gateway_Pack));
				//忽略非此节点的单播数据包
				if(rev_pack.node_id != node_info->LocalNode_Mac)
					continue;
				//忽略不是注册响应的数据包
				if(rev_pack.pack_type != LORA_REGACK)
					continue;
				//提取注册信息,返回正确标志
				if(LoraNode_RegAck(&rev_pack,node_info) == LORA_OK)
				{
					//发送一般响应
					LoraNode_SendAckPack(Radio,node_info);
					return LORA_OK;
				}
			}
		}
		delay_ms(10);
		timeout++;
	}
	//超时返回错误状态
	return LORA_SYSTEM_ERROR;
}
	
/**
 * lora 本地节点  发送注册入网请求
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_SendRegPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// 检查组网有效性
	if(Radio==NULL || node_info->LocalNode_id != 0)
		return LORA_SYSTEM_ERROR;
	// 封装注册入网请求包
	send_pack.node_id = node_info->LocalNode_Mac;		//数据包
	send_pack.pack_type = LORA_REG;									//注册请求
	send_pack.node_mac = node_info->LocalNode_Mac;	//
	// 发送数据包
	Radio->SetTxPacket(&send_pack,sizeof(LoraNetwork_Node_Pack));
	
	return LORA_OK;
}


/**
 * lora 本地节点  处理注册响应
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_RegAck(LoraNetwork_Gateway_Pack *rev_pack,LocalNode_INFO *node_info)
{
	
	// 如果已经处于注册状态则直接返回
	if(node_info->LocalNode_Status == NODE_STATUS_REG)
		return LORA_OK;
	// 提取负载信息为集中器分配的本节点名称
	lora_memcpy(node_info->LocalNode_Name,rev_pack->Load,rev_pack->Load_length);
	// 变更为注册状态
	node_info->LocalNode_Status = NODE_STATUS_REG;
	
	return LORA_OK;
}

/**
 * lora 本地节点  发送一般响应
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_SendAckPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// 检查组网有效性
	if(Radio==NULL || node_info->LocalNode_Status == NODE_STATUS_OFFLINE)
		return LORA_NODE_OFFLINE;
	// 封装一般响应包
	send_pack.node_id = node_info->LocalNode_Mac;
	send_pack.pack_type = LOEA_ACK;
	send_pack.node_mac = node_info->LocalNode_Mac;
	// 发送数据包
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	
	return LORA_OK;	
}

/**
 * lora 本地节点上传传感器数据
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_SendDataPack(tRadioDriver *Radio,LocalNode_INFO *node_info)
{
	LoraNetwork_Node_Pack send_pack;
	
	// 检查组网有效性
	if(Radio==NULL || node_info->LocalNode_id == 0)
		return LORA_NODE_OFFLINE;
	// 封装发送数据包
	send_pack.node_id = node_info->LocalNode_id;
	send_pack.pack_type = LORA_NODE_UPLOAD;
	send_pack.node_mac = node_info->LocalNode_Mac;
	send_pack.rev_delay_time = 0;
	send_pack.rev_Rssi = 0;
	send_pack.Load_length = sizeof(Global_SensorDataBuff);
	lora_memcpy(send_pack.Load,&Global_SensorDataBuff,sizeof(Global_SensorDataBuff));
	// 发送数据包
	Radio->SetTxPacket(&send_pack,sizeof(send_pack));
	
	return LORA_OK;
}

#endif
