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
	UA_StatusCode writeDate(const char *pValue, int nNodeListSize, const char *pItem, uint16_t nUaTypes);
	void getObjectList();
	void clientClose();
	void addNode(char *pNodeName, uint16_t nUaTypes, char *pBrowseName);

private:
	UA_Client *m_client;
	map<string, int> m_mapBrowse;
	bool m_bConnected;
};

#endif // RTPAGEMANAGER_H
