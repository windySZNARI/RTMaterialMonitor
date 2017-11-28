#ifndef RTPAGEMANAGER_H
#define RTPAGEMANAGER_H

#include <QObject>
#include "open62541.h"
#include <map>
using namespace std;

class rtPageManager : public QObject
{
	Q_OBJECT

public:
	rtPageManager(QObject *parent = NULL);
	~rtPageManager();

	void initOpcuaClient();

private:
	void readThread();

	int client_connect(char *pAddr);
	UA_ReadResponse readData(int nNodeListSize, const char *pItem);

	UA_StatusCode writeDate(const char *pValue, int nNodeListSize, const char *pItem, uint16_t nUaTypes)
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


	void getObjectList()
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
					if (m_mapBrowse.find(strNodeId + strEndponit) == m_mapBrowse.end())
					{
						printf("node and endpoint name [%s]\n", (strNodeId + strEndponit).c_str());
						m_mapBrowse[strNodeId + strEndponit] = 1;
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

private:
	UA_Client *m_client;
	map<string, int> m_mapBrowse;
	bool m_bConnected;
};

#endif // RTPAGEMANAGER_H
