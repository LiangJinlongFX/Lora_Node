#include "lora_network.h"
#include "radio.h"
#include "rtthread.h"
#include "stdio.h"
#include "debug.h"

#if defined(USED_LORANETWORK_GATEWAY)

Lora_Node *Global_NodeList = NULL;								//全局节点设备注册列表
LoraNode_Data Global_SensorData[LORA_NODE_MAX];		//全局节点传感器缓冲区
LoraGateway_Status Global_GatewayStatus;					//全局节点轮询数组

/**
 * lora gateway lora收发状态处理
 * @param   
 * @return 
 * @brief 
 **/
void LoraNetowrk_Gateway_Process(tRadioDriver *Radio)
{
	uint8_t lora_status;
	uint8_t lora_buffer[100];
	uint16_t lora_buffersize;
	LoraNetwork_Node_Pack rev_pack;
	
	// 初始化注册链表
	lora_status = LoraNode_Init(&Global_NodeList);
	if(lora_status != LORA_OK)
	{
		logging_error("Node_Init_ERROR!");
	}
	// 发送重新注册通知
	LoraGateway_SendReregPack(Radio);
	while(1)
	{
		lora_status = Radio->Process();
		// 当lora发送数据成功后立即返回接收状态
		if(lora_status == RF_TX_DONE)
		{
			Radio->StartRx();
		}
		// 当lora接收到数据
		if(lora_status == RF_RX_DONE)
		{
			//提取lora接收数据
			Radio->GetRxPacket(lora_buffer,&lora_buffersize);
			logging_debug("RX DONE,size=%d",sizeof(LoraNetwork_Node_Pack));
			if(lora_buffersize != sizeof(LoraNetwork_Node_Pack))
			{
				Radio->StartRx();
				continue;
			}
			else
			{
				//提取节点数据包
				lora_memcpy(&rev_pack,lora_buffer,sizeof(LoraNetwork_Node_Pack));
			}
			//节点数据包解析处理
			LoraNetowrk_NodePack_Resolver(Radio,&rev_pack);
		}
		rt_thread_delay(10);
	}
}

/**
 * 节点数据包解析
 * @param   
 * @return 
 * @brief 
 **/
void LoraNetowrk_NodePack_Resolver(tRadioDriver *Radio,LoraNetwork_Node_Pack* rev_pack)
{
	// 判断接收到的数据包类型
	switch(rev_pack->pack_type)
	{
		// 接收到节点注册请求
		case LORA_REG:
		{
			logging_debug("节点请求注册");
			LoraNode_Reg(Radio,rev_pack);
		}break;
		// 接收到节点一般响应
		case LOEA_ACK:
		{
			logging_debug("收到节点一般响应");
			LoraNode_AckResole(rev_pack);
		}break;
		// 接收到节点传感器数据包
		case LORA_NODE_UPLOAD:
		{
			logging_debug("收到节点传感器信息");
			LoraNode_GetSensorData(rev_pack);
		}break;
	}
}

/**
 * 集中器发送重新注册广播包
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraGateway_SendReregPack(tRadioDriver *Radio)
{
	LoraNetwork_Gateway_Pack pack;
	
	//打包数据包
	pack.node_id = 0;		//为广播包
	pack.pack_type = LORA_REREG;
	//发送数据包
	Radio->SetTxPacket(&pack,sizeof(pack));
}

/**
 * 集中器发送上传数据请求广播包
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraGateway_UploadBroadcast(tRadioDriver *Radio,Lora_Node *node_list)
{
	LoraNetwork_Gateway_Pack pack;
	Lora_Node *p;
	uint8_t i,node_num;		//当前节点数量
	uint8_t node_datalist[20]; 
	
	//生成节点数据上传队列
	p = node_list;	//指向注册链表头指针
	i = 0;
	do{
		//跳过首节点
		if(p->node_id == 0)
		{
			p = p->pNext;
			continue;
		}
		node_datalist[i] = p->node_mac;
		i++;
		p = p->pNext;
	}while(p == NULL);
	node_datalist[i] = 0;	//以0为有效结束标志
	//复制到集中器状态信息中
	lora_memcpy(Global_GatewayStatus.node_list,node_datalist,i);	//复制数据上传排序表
	//打包数据包
	pack.node_id = 0;		//为广播包
	pack.pack_type = LORA_NODE_UPLOAD;	//为请求数据广播包
	pack.Load_length = i-1;	//当前要传数据的节点数
	lora_memcpy(pack.Load,node_datalist,i);	//复制数据上传排序表
	//发送数据包
	Radio->SetTxPacket(&pack,sizeof(pack));
}


/**
 * lora 节点提取数据包中传感器信息
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_GetSensorData(LoraNetwork_Node_Pack* rev_pack)
{
	uint8_t node_id;
	LoraNode_Data* DataBuff;
	// 如果有效负载长度不等于传感器数据结构体大小,返回数据错误
	if(rev_pack->Load_length != sizeof(LoraNode_Data))
		return LORA_DATA_ERROR;
	// 拷贝数据
	rt_memcpy(&Global_SensorData[rev_pack->node_id],rev_pack->Load,sizeof(LoraNode_Data));
	// 记录此次该节点数据已被提取
	Global_GatewayStatus.node_list[rev_pack->node_id] = NODE_STATUS_UPDATA;
	return LORA_OK;
}

/**
 * lora节点一般响应处理
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_AckResole(LoraNetwork_Node_Pack* rev_pack)
{
	Lora_Node* node;
	
	node = LoraNode_Find(Global_NodeList,rev_pack->node_id);
	switch(node->node_status)
	{
		case NODE_STATUS_REG:
		{
			node->node_status = NODE_STATUS_ONLINE;
		}break;
	}
	
}

/**
 * lora节点注册处理
 * @param   
 * @return 
 * @brief 
 **/
Lora_Sataus LoraNode_Reg(tRadioDriver *Radio, LoraNetwork_Node_Pack* rev_pack)
{
	Lora_Node* Node_New;
	LoraNetwork_Gateway_Pack Pack_Structure;
	
	// 在节点注册列表新增节点
	Node_New = LoraNode_Append(Global_NodeList);
	if(Node_New == NULL)  logging_warning("NEW NODE REG ERROR!");
	// 初始化新节点登记信息
	// 暂不支持在ONENET平台的自动注册,改为默认在ONENET注册的三个设备
	if(Node_New->node_id == 1)
	{
		sprintf(Node_New->device_id,"514914374");		//ONENET 节点1设备ID
		sprintf(Node_New->device_key,"yrk6EP8KBUpZeFcoqPLGGtNEzlQ=");	//ONENET 节点1设备密钥
		sprintf(Node_New->node_name,"node1");				//ONENET 节点设备名称
		Node_New->node_status = NODE_STATUS_REG;	  //设备状态改为注册状态
	}
	else if(Node_New->node_id == 2)
	{
		sprintf(Node_New->device_id,"514914374");		//ONENET 节点1设备ID
		sprintf(Node_New->device_key,"yrk6EP8KBUpZeFcoqPLGGtNEzlQ=");	//ONENET 节点1设备密钥
		sprintf(Node_New->node_name,"node2");				//ONENET 节点设备名称
		Node_New->node_status = NODE_STATUS_REG;	  //设备状态改为注册状态
	}
	else if(Node_New->node_id == 3)
	{
		sprintf(Node_New->device_id,"514914374");		//ONENET 节点1设备ID
		sprintf(Node_New->device_key,"yrk6EP8KBUpZeFcoqPLGGtNEzlQ=");	//ONENET 节点1设备密钥
		sprintf(Node_New->node_name,"node3");				//ONENET 节点设备名称
		Node_New->node_status = NODE_STATUS_REG;	  //设备状态改为注册状态
	}
	// 生成注册响应数据包
	Pack_Structure.node_id = 0;              //所有节点接收
	Pack_Structure.pack_type = LORA_REGACK;  //数据包类型为注册响应类
	Pack_Structure.Load_length = 2;					 //有效负载为2字节
	Pack_Structure.Load[0] = rev_pack->node_mac;	//负载0字节为请求注册的节点硬件地址
	Pack_Structure.Load[1] = Node_New->node_id;
	// 发送注册响应数据包
	Radio->SetTxPacket(&Pack_Structure,sizeof(Pack_Structure));
}

/**
 * 初始化lora节点信息链表
 * @param   
 * @return 
 * @brief 头结点不代表任何一个独立节点，其代表所有注册节点
 **/
Lora_Sataus LoraNode_Init(Lora_Node** pHead)
{
	// 给首节点分配内存空间
	*pHead = (Lora_Node*)rt_malloc(sizeof(Lora_Node));
	// 分配空间失败则返回系统错误
	if(*pHead == NULL)
		return LORA_SYSTEM_ERROR;
	// 初始化头节点信息
	(*pHead)->node_id = 0;
	sprintf((*pHead)->node_name,"all");
	(*pHead)->node_status = 0;
	(*pHead)->pNext = NULL;
	
	return LORA_OK;
}

/**
 * 在节点信息链表新增节点设备
 * @param   pHead[节点设备信息链表头指针]
 * @return  Lora_Sataus[LORA执行情况]
 * @brief 
 **/
Lora_Node* LoraNode_Append(Lora_Node* pHead)
{
	Lora_Node* r = pHead;
	Lora_Node* pNew = (Lora_Node*)rt_malloc(sizeof(Lora_Node));
	if(pNew == NULL)
		return NULL;
	// 让尾指针循环到当前最后一个节点
	while(r->pNext != NULL)
		r = r->pNext;
	// 将新增的节点指针接到尾指针并分配节点序列号
	pNew->node_id = r->node_id;
	r->pNext = pNew;
	
	return pNew;
}

/**
 * 查询并提取注册链表某一结点
 * @param   
 * @return 
 * @brief 
 **/
Lora_Node* LoraNode_Find(Lora_Node* pHead, uint8_t node_id)
{
	Lora_Node *p,*r;
	if(pHead == NULL)
		return NULL;
	r = pHead;
	while(r->node_id != node_id && r->pNext != NULL)
	{
		r = r->pNext;
	}
	if(r->node_id == node_id)
		return r;
	else
		return NULL;	
}

/**
 * 在节点信息链表删除节点设备
 * @param   pHead[节点设备信息链表头指针]  node_id[节点设备序列号]
 * @return  Lora_Sataus[LORA执行情况]
 * @brief 
 **/
Lora_Sataus LoraNode_Delete(Lora_Node* pHead, uint8_t node_id)
{
	Lora_Node *pb,*pf; 
	pb = pHead;
	if(pHead == NULL)
		return LORA_SYSTEM_ERROR;
	// 循环查找要删除的结点
	while(pb->node_id != node_id && pb->pNext != NULL)
	{
		pf = pb;
		pb = pb->pNext;
	}
	// 如果找到要删除的结点
	if(pb->node_id == node_id)
	{
		if(pb == pHead)
		{
			return LORA_SYSTEM_ERROR;
		}
		else
		{
			pf->pNext = pb->pNext;
			rt_free(pb);
		}
	}
	else
	{
		return LORA_SYSTEM_ERROR;
	}
	
	return LORA_OK;
}

#endif



