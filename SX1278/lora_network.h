#ifndef _LORA_NETWORK_H
#define _LORA_NETWORK_H

#include "sys.h"
#include "radio.h"

#define USED_LORANETWORK_NODE
//#define USED_LORANETWORK_GATEWAY

#define lora_memcpy memcpy
#define LORA_NODE_MAX 10	//��ǰ֧�����������ڵ���

/* ------------------LORA�ڵ�״̬��------------------- */
#define NODE_STATUS_OFFLINE 0			//����״̬
#define NODE_STATUS_REG			1			//ע��״̬
#define NODE_STATUS_ONLINE	2			//����״̬
#define NODE_STATUS_UPDATA  3			//�Ѹ�������״̬

#if defined(USED_LORANETWORK_NODE)
/* ------------------�ڵ��------------------- */
#define LORA_LOCAL_MAC	0x80			//���ڵ�ΨһӲ����ʶ��
typedef struct
{
	uint8_t LocalNode_id;				//������������������к�
	uint8_t LocalNode_Mac;			//���豸�Դ���Ӳ��ʶ����
	uint8_t LocalNode_Status;		//���豸״̬
	char    LocalNode_Name[20];	//����������������豸����
}LocalNode_INFO;

#endif

/* ------------------��������------------------- */

#define LOAD_LENGTH 50	//���ݸ��س���
#define LORA_BROADCAST_ID	0	//�㲥id
#define LOEA_ACK						0x00    //lora һ����Ӧ
#define LORA_REG		        0x01		//lora node ע������
#define LORA_REGACK         0x02		//lora node ע��Ӧ��
#define LORA_NODE_REMOVE    0x03		//lora node ע������
#define LORA_NODE_REMOVEACK 0x04	  //lora node ע��ȷ��
#define LORA_NODE_UPLOAD    0x05		//lora node �ϴ�����
#define LORA_NODE_LOWPOWER  0x06		//lora node �͵�������
#define LORA_REREG					0x07		//lora gateway����ע������
#define LORA_UPDATA					0x08		//lora gateway������������㲥


typedef enum
{
	LORA_OK = 0,
	LORA_SYSTEM_ERROR,
	LORA_DATA_ERROR,
	LORA_NODE_OFFLINE
}Lora_Sataus;

/* 
 * ���������ݰ� 
 */
typedef struct
{
	uint8_t  node_id;	    			//�ڵ�id,�������ֲ�ͬ�Ľڵ�
	uint8_t	 pack_type;	  			//���ݰ�����
	uint8_t  rev_delay_time;		//�����ӳ�ʱ��
	uint32_t rev_Rssi;					//��һ�ν������ݰ����ź��������
	uint8_t  Load_length;				//���س���
	uint8_t  Load[20]; 					//���ݸ���	
}LoraNetwork_Gateway_Pack;

/*
 * �ڵ����ݰ�
 */
typedef struct
{
	uint8_t  node_id;	    			//�ڵ�id,�������ֲ�ͬ�Ľڵ�
	uint8_t	 pack_type;	  			//���ݰ�����
	uint8_t  node_mac;					//�ڵ�ΨһӲ����ַ
	uint8_t  rev_delay_time;		//�����ӳ�ʱ��
	uint32_t rev_Rssi;					//��һ�ν������ݰ����ź��������
	uint8_t  Load_length;				//���س���
	uint8_t  Load[LOAD_LENGTH]; //���ݸ���	
}LoraNetwork_Node_Pack;



/*
 * lora �ڵ����ݽṹ��
 */
typedef struct
{
	uint8_t  node_mac;	//���������ڵ��ʶ
	uint16_t LX;				//���ն�
	uint16_t TVOC;		  //TVOCŨ��
	int32_t  temp;			//�¶�
	int32_t  hum;				//ʪ��
	int32_t  pressure;	//����ѹǿ
}LoraNode_Data;

/*
 * lora gateway ״̬��Ϣ�ṹ��
 */
typedef struct
{
	uint8_t node_num;		  	//��ǰ�ڵ�����
	uint16_t event_time;		//�¼�ʱ���¼
	uint8_t node_list[20];	//��ǰ�ڵ���ѯ״̬����
}LoraGateway_Status;

/*
 * �ڵ���Ϣ������ṹ��
 */
typedef struct lora_node
{
	uint8_t node_id;				//�ڵ��豸���к�
	uint8_t node_mac;				//�ڵ�ΨһӲ����ַ
	char    node_name[20];	//�ڵ��豸����
	uint8_t node_status;		//�ڵ��豸״̬
	uint16_t node_rssi;			//���һ���豸���ź�״��
	char device_id[50];			//�Ʒ�����������豸id
	char device_key[50];		//�Ʒ�����������豸��Կ
	
	struct lora_node *pNext;	//ָ����һ���ڵ���Ϣ
	
}Lora_Node;

/* -------------------���ؽڵ�����---------------- */
#if defined(USED_LORANETWORK_NODE)

extern LoraNode_Data Global_SensorDataBuff;	//ȫ�ִ�����������
extern LocalNode_INFO Global_NodeInfo;

Lora_Sataus LoraNode_Registe(tRadioDriver *Radio,LocalNode_INFO *node_info);
Lora_Sataus LoraNode_SendDataPack(tRadioDriver *Radio,LocalNode_INFO *node_info);
Lora_Sataus LoraNode_SendRegPack(tRadioDriver *Radio,LocalNode_INFO *node_info);
Lora_Sataus LoraNode_RegAck(LoraNetwork_Gateway_Pack *rev_pack,LocalNode_INFO *node_info);
Lora_Sataus LoraNode_SendAckPack(tRadioDriver *Radio,LocalNode_INFO *node_info);
#endif

#if defined(USED_LORANETWORK_GATEWAY)
/* -------------------�ڵ�ע���б����---------------- */
Lora_Sataus LoraNode_Init(Lora_Node** pHead);
Lora_Node* LoraNode_Append(Lora_Node* pHead);
Lora_Sataus LoraNode_Delete(Lora_Node* pHead, uint8_t node_id);
Lora_Node* LoraNode_Find(Lora_Node* pHead, uint8_t node_id);
/* -------------------�ڵ����ݰ��������---------------- */
Lora_Sataus LoraNode_Reg(tRadioDriver *Radio, LoraNetwork_Node_Pack* rev_pack);
Lora_Sataus LoraNode_AckResole(LoraNetwork_Node_Pack* rev_pack);
Lora_Sataus LoraNode_GetSensorData(LoraNetwork_Node_Pack* rev_pack);
Lora_Sataus LoraGateway_SendReregPack(tRadioDriver *Radio);
/* -------------------�ڵ��շ��������---------------- */
void LoraNetowrk_NodePack_Resolver(tRadioDriver *Radio,LoraNetwork_Node_Pack* rev_pack);
void LoraNetowrk_Gateway_Process(tRadioDriver *Radio);
#endif
#endif