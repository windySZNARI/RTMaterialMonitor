#pragma   once 

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include "open62541.h"
#include <map>
#include <QThread>
using namespace std;

int main()
{
    UA_Client *client = UA_Client_new(UA_ClientConfig_standard);
    UA_StatusCode retval = UA_Client_connect(client, "opc.tcp://localhost:16664");
    if(retval != UA_STATUSCODE_GOOD) {
        UA_Client_delete(client);
        return (int)retval;
    }
    
    UA_Int32 value = 0;
    // Read node's value
    printf("\nReading the value of node (1, \"the.answer\"):\n");
    UA_ReadRequest rReq;
    UA_ReadRequest_init(&rReq);
    rReq.nodesToRead =  (UA_ReadValueId*)UA_Array_new(1, &UA_TYPES[UA_TYPES_READVALUEID]);
    rReq.nodesToReadSize = 1;
    rReq.nodesToRead[0].nodeId = UA_NODEID_STRING_ALLOC(1, "the.answer"); /* assume this node exists */
    rReq.nodesToRead[0].attributeId = UA_ATTRIBUTEID_VALUE;
	UA_ReadResponse rResp;
	while(1)
	{
	    rResp = UA_Client_Service_read(client, rReq);
	    if(rResp.responseHeader.serviceResult == UA_STATUSCODE_GOOD &&
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
    UA_ReadResponse_deleteMembers(&rResp);

	return 0;
}