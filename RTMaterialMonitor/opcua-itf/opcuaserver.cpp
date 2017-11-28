#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include "libopcua.h"
#include <map>
#include <pthread.h>
#include <signal.h>

using namespace std;

UA_Boolean running = true;
static void stopHandler(int sig) 
{
    running = false;
}


static void onRead(void *handle, const UA_NodeId nodeid, const UA_Variant *data, const UA_NumericRange *range) 
{
    //UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "onRead; handle is: %i", (uintptr_t)handle);
}

static void onWrite(void *h, const UA_NodeId nodeid, const UA_Variant *data, const UA_NumericRange *range) 
{
    //UA_LOG_INFO(logger, UA_LOGCATEGORY_USERLAND, "onWrite; handle: %i", (uintptr_t)h);
}


void *func(void *threadid)
{
	UA_Server *server = (UA_Server *)threadid;
	while(1)
	{
		sleep(30);	
	    /* add a variable node to the address space */
	    UA_VariableAttributes attr;
	    UA_VariableAttributes_init(&attr);
	    UA_Int32 myInteger = 42;
	    UA_Variant_setScalar(&attr.value, &myInteger, &UA_TYPES[UA_TYPES_INT32]);
	    attr.description = UA_LOCALIZEDTEXT("en_US","the answer");
	    attr.displayName = UA_LOCALIZEDTEXT("en_US","the answer");
	    UA_NodeId myIntegerNodeId = UA_NODEID_STRING(1, "the.answer");
	    UA_QualifiedName myIntegerName = UA_QUALIFIEDNAME(1, "the answer");
	    UA_NodeId parentNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_OBJECTSFOLDER);
	    UA_NodeId parentReferenceNodeId = UA_NODEID_NUMERIC(0, UA_NS0ID_ORGANIZES);
	    UA_Server_addVariableNode(server, myIntegerNodeId, parentNodeId,
	                              parentReferenceNodeId, myIntegerName,
	                              UA_NODEID_NULL, attr, NULL, NULL);
	
	  //  UA_ValueCallback callback = {(void*)7, onRead, onWrite};
	  //  UA_Server_setVariableNode_valueCallback(server, myIntegerNodeId, callback);		
		
	}
	
	//pthread_exit(NULL);
}


int main()
{
	signal(SIGINT,  stopHandler);
    signal(SIGTERM, stopHandler);

    UA_ServerConfig config = UA_ServerConfig_standard;
    UA_ServerNetworkLayer nl = UA_ServerNetworkLayerTCP(UA_ConnectionConfig_standard, 16664);
    config.networkLayers = &nl;
    config.networkLayersSize = 1;
    UA_Server *server = UA_Server_new(config);
	pthread_t id;
	void *ret;
	int i,retv;
	
	retv=pthread_create(&id,NULL,func,(void *)server);
	if (retv!=0)
	{
		printf ("Create pthread error!/n");
		return 1;
	}

    UA_StatusCode retval = UA_Server_run(server, &running);
    UA_Server_delete(server);
    nl.deleteMembers(&nl);
    pthread_join(id,&ret);
    
    
	return 0;

}

