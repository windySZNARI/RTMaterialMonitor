#include "rtPageManager.h"
#include <QThread>
#include <windows.h>
#include <functional>
#include <thread>
#include <atomic>

rtPageManager::rtPageManager(QObject *parent)
	: QObject(parent)
{
	m_client = nullptr;
}

rtPageManager::~rtPageManager()
{

}

int rtPageManager::client_connect(char* pAddr)
{
	if (m_client == NULL)
		m_client = UA_Client_new(UA_ClientConfig_standard);

	if (m_client == NULL)
	{
		printf("malloc fail\n");
		return -1;
	}
	UA_StatusCode retval = UA_STATUSCODE_GOOD;
	if (!m_bConnected)
	{
		retval = UA_Client_connect(m_client, pAddr);
		m_bConnected = true;
		if (retval != UA_STATUSCODE_GOOD)
		{
			printf("UA_Client_connect fail,error'code[%x]\n", retval);
			//UA_Client_delete(m_client);
			clientClose();

		}
		getObjectList();
	}
	return retval;
}

UA_ReadResponse rtPageManager::readData(int nNodeListSize, const char *pItem)
{
	printf("read data\n");
	int i = 0;
	UA_ReadRequest rReq;
	UA_ReadRequest_init(&rReq);
	//rReq.nodesToRead = UA_ReadValueId_new();
	rReq.nodesToRead = (UA_ReadValueId*)UA_Array_new(1, &UA_TYPES[UA_TYPES_READVALUEID]);
	rReq.nodesToReadSize = nNodeListSize;


	rReq.nodesToRead[0].nodeId = UA_NODEID_STRING_ALLOC(1, pItem); /* assume this node exists */
	rReq.nodesToRead[0].attributeId = UA_ATTRIBUTEID_VALUE;

	UA_ReadResponse rResp = UA_Client_Service_read(m_client, rReq);
	UA_ReadRequest_deleteMembers(&rReq);
	//UA_ReadResponse_deleteMembers(&rResp);
	return rResp;
}

void rtPageManager::readThread()
{
	string OPCServer = "http://examples.freeopcua.github.io";
	string itemName = "CollectProcessNum";

	printf("%s\n", (itemName + OPCServer).c_str());
	if (m_mapBrowse.find(itemName + OPCServer) == m_mapBrowse.end())
	{
		addNode((char *)itemName.c_str(), UA_TYPES_STRING, (char *)OPCServer.c_str());
		m_mapBrowse[itemName + OPCServer] = 1;
	}

	while (1)
	{
		UA_ReadResponse rResp = readData(1, itemName.c_str());
		string strContent;
		if (rResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD
			&& rResp.resultsSize > 0 && rResp.results[0].hasValue
			&& UA_Variant_isScalar(&rResp.results[0].value))
		{			
			switch (rResp.results[0].value.type->typeIndex)
			{
			case UA_TYPES_UINT32:
			case UA_TYPES_INT32:
			{
				UA_Int32 nValue = *(UA_Int32*)rResp.results[0].value.data;
				char buf[512] = { 0 };
				sprintf(buf, "%d", nValue);
				strContent = buf;
			}
			printf("3\n");
			break;
			case UA_TYPES_STRING:
			{
				UA_String str = *(UA_String *)rResp.results[0].value.data;
				int length = str.length + 1;
				char *buf = (char *)malloc(length);
				//snprintf(buf,length,"%s",(char *)str.data);
				for (int i = 0; i < length - 1; i++)
				{
					buf[i] = ((char *)str.data)[i];

				}
				buf[length - 1] = '\0';
				strContent = buf;
				free(buf);
				UA_ReadResponse_deleteMembers(&rResp);
			}
			break;
			default:
				break;
			}
		}
		else
		{
			printf("读数据失败\n");
		}

		printf("Content = %s\n", strContent.c_str());

		Sleep(500);
	}
}

void rtPageManager::initOpcuaClient()
{
	/*
	if (!client)
	{
		client = UA_Client_new(UA_ClientConfig_standard);
	}
	UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://192.168.31.240:12689");
	if (retval != UA_STATUSCODE_GOOD) {
		UA_Client_delete(client);
	}

	UA_Int32 value = 0;
	// Read node's value
	printf("\nReading the value of node (1, \"the.answer\"):\n");
	UA_ReadRequest rReq;
	UA_ReadRequest_init(&rReq);
	rReq.nodesToRead = (UA_ReadValueId*)UA_Array_new(1, &UA_TYPES[UA_TYPES_READVALUEID]);
	rReq.nodesToReadSize = 1;
	rReq.nodesToRead[0].nodeId = UA_NODEID_STRING_ALLOC(1, "the.answer"); /* assume this node exists */ /*
	rReq.nodesToRead[0].attributeId = UA_ATTRIBUTEID_VALUE;
	UA_ReadResponse rResp;
	while (1)
	{
		rResp = UA_Client_Service_read(client, rReq);
		if (rResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD &&
			rResp.resultsSize > 0 && rResp.results[0].hasValue &&
			UA_Variant_isScalar(&rResp.results[0].value) &&
			rResp.results[0].value.type == &UA_TYPES[UA_TYPES_INT32])
		{
			value = *(UA_Int32*)rResp.results[0].value.data;
			printf("++++++++ the value is: %i\n", value);
			break;
		}
		else
		{
			printf("======== no node\n");
		}
		QThread::sleep(2);
	}
	UA_ReadRequest_deleteMembers(&rReq);
	UA_ReadResponse_deleteMembers(&rResp);*/

	string v = "";

	//发送opcda的需要的地址
	//连接opcua服务器
	m_bConnected = false;
	if (client_connect("opc.tcp://192.168.31.240:12689") != 0)
	{
		printf("链接opcua服务失败\n");
		return;
	}

	//增加节点到ua服务
	
	std::thread t1(&rtPageManager::readThread, this);
	t1.join();
}
