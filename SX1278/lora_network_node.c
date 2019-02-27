#include "sys.h"
#include "radio.h"
#include "stdio.h"
#include "delay.h"
#include "lora_network.h"
/* 传感器驱动头文件 */
#include "bh1750.h"
#include "adc.h"
#include "bme280_app.h"
#include "led.h"

#if defined(USED_LORANETWORK_NODE)

LocalNode_INFO Global_NodeInfo;

/**
 * lora 本地节点  监听上传广播并在合适的时间上传数据
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
	//如果是第一个发送
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
		/* 超过最大的发送窗口时限 */
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
			//如果数据包符合节点数据包大小
			if(buffer_size == sizeof(LoraNetwork_Node_Pack))
			{
				//还原节点数据包
				lora_memcpy((void*)&rev_pack,(void*)Buffer,sizeof(LoraNetwork_Node_Pack));
				//没有监听到前一个节点发送数据包
				printf("current node id: %d",rev_pack.node_id);
				if(rev_pack.node_id != (No-1))
				{
					Radio->StartRx();
					continue;
				}
				//监听到前一个节点在传输数据,延时后发送数据
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
 * nodec接收状态机
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
			//如果数据包符合集中器数据包大小
			if(buffer_size == sizeof(LoraNetwork_Gateway_Pack))
			{
				//还原集中器数据包
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
 * node处理集中器数据包
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_GatewayPack_Process(tRadioDriver *Radio,LocalNode_INFO *node_info,LoraNetwork_Gateway_Pack *rev_pack)
{
	uint8_t i,No;
	uint8_t node_list[20];
	
	//如果是广播包
	if(rev_pack->node_id == 0)
	{
		//如果是请求数据
		if(rev_pack->pack_type == LORA_NODE_UPLOAD)
		{
			printf("gateway pack type: LORA_NODE_UPLOAD\r\n");
			//复制数据地址
			lora_memcpy(node_list,rev_pack->Load,rev_pack->Load_length);
			//获取本节点上传数据序列号
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
	while(timeout <= 1000)
	{
		status = Radio->Process();
		// 当lora发送数据成功后立即返回接收状态
		if(status == RF_TX_DONE)
		{
			printf("TX DONE\r\n");
			Radio->StartRx();
		}
		else if(status==RF_RX_DONE)
		{
			printf("RF_RX_DONE\r\n");
			Radio->GetRxPacket(Buffer, (uint16_t*)&buffer_size);
			//如果接收到的数据包不符合集中器数据包大小
			if(buffer_size != sizeof(LoraNetwork_Gateway_Pack))
			{
				printf("No Gateway_Pack\r\n");
				Radio->StartRx();
				continue;
			}
			else
			{
				//还原集中器数据包
				lora_memcpy((void*)&rev_pack,(void*)Buffer,sizeof(LoraNetwork_Gateway_Pack));
				//忽略非此节点的单播数据包
				if(rev_pack.node_id != node_info->LocalNode_Mac)
				{
					printf("RX Continue\r\n");
					Radio->StartRx();
					continue;
				}
				//忽略不是注册响应的数据包
				if(rev_pack.pack_type != LORA_REGACK)
				{
					printf("RX Continue\r\n");
					Radio->StartRx();
					continue;
				}
				//提取注册信息,返回正确标志
				if(LoraNode_RegAck(&rev_pack,node_info) == LORA_OK)
				{
					/* 此延时是必须的!!!,等待gateway切换为接收状态才发送 */
					delay_ms(500);
					//发送一般响应
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
	uint16_t i=0;
	uint8_t lora_status;
	LoraNetwork_Node_Pack send_pack;
	LoraNode_Data SensorData;
	
	// 检查组网有效性
	if(Radio==NULL)
		return LORA_NODE_OFFLINE;
	// 获取传感器数据
	LoraNode_GetSensorData(&SensorData);
	//获取节点MAC
	SensorData.node_mac = node_info->LocalNode_Mac;
	
	// 封装发送数据包
	send_pack.node_id = node_info->LocalNode_id;
	send_pack.pack_type = LORA_NODE_UPLOAD;
	send_pack.node_mac = node_info->LocalNode_Mac;
	send_pack.rev_delay_time = 0;
	send_pack.rev_Rssi = 0;
	send_pack.Load_length = sizeof(SensorData);
	lora_memcpy(send_pack.Load,&SensorData,sizeof(SensorData));
	printf("pack size: %d %d \r\n",sizeof(SensorData),sizeof(send_pack));
	// 发送数据包
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
 * 获取传感器数据
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_GetSensorData(LoraNode_Data *SensorData_Buff)
{
	struct bme280_data comp_data;
	
	//读取光照强度
	SensorData_Buff->LX = BH1750_ReadRealData();
	//读取环境参数
	bme280_get_sensor_data(BME280_ALL, &comp_data, &Global_BME280);
	SensorData_Buff->temp = comp_data.temperature;
	SensorData_Buff->pressure = comp_data.pressure;
	SensorData_Buff->hum = comp_data.humidity;
	//读取TVOC参数
	SensorData_Buff->TVOC = Get_TVOC_Val();
	//读取电池电压
	SensorData_Buff->node_VBAT = (int)Get_VBAT()*100;
	//无附带信息
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
