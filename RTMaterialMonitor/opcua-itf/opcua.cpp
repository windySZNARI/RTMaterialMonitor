#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include "libopcua.h"
#include <map>
using namespace std;

map<string,int> m_mapBrowse;
UA_Client *m_client;
bool m_bConnected;

int client_connect(char *pAddr);
void clientClose();
UA_ReadResponse readData(int nNodeListSize, const char *pItem);
UA_StatusCode writeDate(const char *pValue, int nWriteSize, const char *pItem, uint16_t nUaTypes );
UA_BrowseResponse getObjectList();
void addNode(char *pNodeName, uint16_t nUaTypes, char *pBrowseName);

int main()
{
	string v="";

	//发送opcda的需要的地址
	//连接opcua服务器
	m_bConnected = false;
	if(client_connect("opc.tcp://192.168.118.108:16664") != 0)
	{
		printf("链接opcua服务失败\n");
		return -1 	;
	}
		//增加节点到ua服务
	string OPCServer = "Softing.OPCToolboxDemo_ServerDA.1";
	string itemName = "time.local.second";
	
	printf("%s\n",(itemName+OPCServer).c_str());
	if(m_mapBrowse.find(itemName+OPCServer) == m_mapBrowse.end())
	{
		addNode((char *)itemName.c_str(), UA_TYPES_STRING,(char *)OPCServer.c_str());
		m_mapBrowse[itemName+OPCServer] = 1;
	}
	
	UA_ReadResponse rResp = readData(1, itemName.c_str());

	if (rResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD 
		&& rResp.resultsSize > 0 && rResp.results[0].hasValue 
		&& UA_Variant_isScalar(&rResp.results[0].value))
	{
		switch (rResp.results[0].value.type->typeIndex)
		{
		case UA_TYPES_INT32:
			{
			UA_Int32 nValue = *(UA_Int32*)rResp.results[0].value.data;
			char buf[512] = {0};
			sprintf(buf, "%d", nValue);
			v = buf;
			}
			printf("3\n");
			break;
		case UA_TYPES_STRING:
			{
			UA_String str = *(UA_String *)rResp.results[0].value.data;
			int length = str.length+1;
			char *buf = (char *)malloc(length);
			//snprintf(buf,length,"%s",(char *)str.data);
			for (int i = 0;  i < length -1; i++)
			{
				buf[i] = ((char *)str.data)[i];
			
			}
			buf[length-1] = '\0';
			v = buf;
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
		return -1;
	}

	printf("v = %s\n",v.c_str());

	return 0;

}


int client_connect(char *pAddr)
{
	if(m_client == NULL)
		m_client = UA_Client_new(UA_ClientConfig_standard);
	
	if (m_client == NULL)
	{
		printf("malloc fail\n");
		return -1;
	}
	UA_StatusCode retval = UA_STATUSCODE_GOOD;
	if(!m_bConnected)
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

/*
typedef struct {
    UA_ExpandedNodeId parentNodeId;
    UA_NodeId referenceTypeId;
    UA_ExpandedNodeId requestedNewNodeId;
    UA_QualifiedName browseName;
    UA_NodeClass nodeClass;
    UA_ExtensionObject nodeAttributes;
    UA_ExpandedNodeId typeDefinition;
} UA_AddNodesItem;
*/



UA_ReadResponse readData(int nNodeListSize, const char *pItem)
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

UA_StatusCode writeDate(const char *pValue, int nNodeListSize, const char *pItem, uint16_t nUaTypes )
{

	printf("write data\n");
	int iRet = 0;

	UA_WriteRequest wReq;
	UA_WriteRequest_init(&wReq);
	wReq.nodesToWrite = UA_WriteValue_new();
	wReq.nodesToWriteSize = nNodeListSize;


	wReq.nodesToWrite[0].nodeId = UA_NODEID_STRING_ALLOC(1, pItem); /* assume this node exists */
	wReq.nodesToWrite[0].attributeId = UA_ATTRIBUTEID_VALUE;
	wReq.nodesToWrite[0].value.hasValue = true;
	wReq.nodesToWrite[0].value.value.type = &UA_TYPES[nUaTypes];
	wReq.nodesToWrite[0].value.value.storageType = UA_VARIANT_DATA_NODELETE;//do not free the integer on deletion

	UA_String str; 
	str.length = strlen(pValue);
    str.data = (UA_Byte*)pValue;
	UA_Variant_init(&(wReq.nodesToWrite[0].value.value));
	UA_Variant_setScalarCopy(&(wReq.nodesToWrite[0].value.value), &str, &UA_TYPES[nUaTypes]);
		
	UA_WriteResponse wResp = UA_Client_Service_write(m_client, wReq);

	if (wResp.responseHeader.serviceResult != UA_STATUSCODE_GOOD)
	{
		printf("write data fail\n");
		iRet = -1;
	}

	UA_WriteRequest_deleteMembers(&wReq);
	UA_WriteResponse_deleteMembers(&wResp);
	return iRet;
}


UA_BrowseResponse getObjectList()
{
	// Browse some objects
	printf("Browsing nodes in objects folder:\n");
	UA_BrowseRequest bReq;
	UA_BrowseRequest_init(&bReq);
	bReq.requestedMaxReferencesPerNode = 0;
	bReq.nodesToBrowse = UA_BrowseDescription_new();
	bReq.nodesToBrowseSize = 1;
	bReq.nodesToBrowse[0].nodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER); //browse objects folder
	bReq.nodesToBrowse[0].resultMask = UA_BROWSERESULTMASK_ALL; //return everything

	UA_BrowseResponse bResp = UA_Client_Service_browse(m_client, bReq);
	for (size_t i = 0; i < bResp.resultsSize; ++i)
	{
		for (size_t j = 0; j < bResp.results[i].referencesSize; ++j)
		{
			UA_ReferenceDescription *ref = &(bResp.results[i].references[j]);
			if (ref->nodeId.nodeId.identifierType == UA_NODEIDTYPE_STRING)
			{
				string strNodeId = string((char *)ref->nodeId.nodeId.identifier.string.data, (int)ref->nodeId.nodeId.identifier.string.length);
				string strEndponit = string((char *)ref->browseName.name.data, (int)ref->browseName.name.length);
				if(m_mapBrowse.find(strNodeId+strEndponit) == m_mapBrowse.end())
				{
					printf("node and endpoint name [%s]\n",(strNodeId+strEndponit).c_str());
					m_mapBrowse[strNodeId+strEndponit] = 1;
				}
			}
		}
	}
	
	
	UA_BrowseRequest_deleteMembers(&bReq);
	UA_BrowseResponse_deleteMembers(&bResp);
}

void clientClose()
{
	m_bConnected = false;
	m_mapBrowse.clear();
	UA_Client_disconnect(m_client);
	UA_Client_delete(m_client);
	if (m_client != NULL)
	{
	
		m_client = NULL;
	}
}

void addNode(char *pNodeName, uint16_t nUaTypes, char *pBrowseName)
{

	printf("add new node\n");
	UA_NodeId var_id;
	UA_VariableAttributes var_attr;
	UA_VariableAttributes_init(&var_attr);
	var_attr.displayName = UA_LOCALIZEDTEXT("en_US", pNodeName);
	var_attr.description = UA_LOCALIZEDTEXT("en_US", pNodeName);

	void *value = UA_new(&UA_TYPES[nUaTypes]);

	/* This does not copy the value */
	UA_Variant_setScalar(&var_attr.value, value, &UA_TYPES[nUaTypes]); //UA_TYPES_STRING
	var_attr.dataType = UA_TYPES[nUaTypes].typeId;

	UA_StatusCode retval = UA_Client_addVariableNode(m_client,
		UA_NODEID_STRING(1, pNodeName), // Assign new/random NodeID  
		UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER),
		UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES),
		UA_QUALIFIEDNAME(0, pBrowseName),
		UA_NODEID_NULL, // no variable type
		var_attr, &var_id);
	if (retval == UA_STATUSCODE_GOOD)
		printf("Created 'NewVariable' with numeric NodeID %u\n", var_id.identifier.numeric);

}

