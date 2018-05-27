#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "MQTTAsync.h"
#include <errno.h>

#if !defined(WIN32)
#include <unistd.h>
#else
#include <windows.h>
#endif

#define ADDRESS       "tcp://192.168.1.175:1883"
#define CLIENTID      "Galileo_UMIK"
#define TOPIC         "weight"
#define QOS           1
#define TIMEOUT       10000L

volatile MQTTAsync_token deliveredtoken;

int disc_finished = 0;
int subscribed = 0;
int finished = 0;

void connlost(void *context, char *cause)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	int rc;

	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);

	printf("Reconnecting\n");
	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
 		finished = 1;
	}
}


void onDisconnect(void* context, MQTTAsync_successData* response)
{
	printf("Successful disconnection\n");
	finished = 1;
}

void onConnectFailure(void* context, MQTTAsync_failureData* response)
{
	printf("Connect failed, rc %d\n", response ? response->code : 0);
	finished = 1;
}

void onSend(void* context, MQTTAsync_successData* response)
{
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_disconnectOptions opts = MQTTAsync_disconnectOptions_initializer;
	int rc;

	printf("Message with token value %d delivery confirmed\n", response->token);

	opts.onSuccess = onDisconnect;
	opts.context = client;

	if ((rc = MQTTAsync_disconnect(client, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start sendMessage, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
}

void onConnect(void* context, MQTTAsync_successData* response)
{
	FILE *fp;
    char buffer[8];
    char c;
    int i = 0;
	int byteCount = 0;
    if((fp=fopen("weight.txt", "r")) == NULL){
        printf ("Nie moge otworzyc pliku weight.txt do odczytu!\n");
        exit(1);
    }
	else {
		for (i=0; i<8; i++) {
			fscanf(fp, "%c", &c);
			if (feof(fp))
				break;
			if (c != '\0' && c != '\n') {
				buffer[i] = c;
				printf("%c", buffer[i]);
				byteCount++;
			}
		}
	printf("\n");
	}
    fclose(fp);
	char payload[byteCount];
	for (i = 0; i < byteCount; i++)
		payload[i] = buffer[i];
	//char strPayload[strlen(payload)+1];
	//strcpy(strPayload, payload);
	
	MQTTAsync client = (MQTTAsync)context;
	MQTTAsync_responseOptions opts = MQTTAsync_responseOptions_initializer;
	MQTTAsync_message pubmsg = MQTTAsync_message_initializer;
	int rc;
	printf("Successful connection\n");
	//printf("%s", strPayload);
	opts.onSuccess = onSend;
	opts.context = client;
	pubmsg.payload = payload;
	pubmsg.payloadlen = byteCount;
	pubmsg.qos = QOS;
	pubmsg.retained = 0;
	deliveredtoken = 0;
	if ((rc = MQTTAsync_sendMessage(client, TOPIC, &pubmsg, &opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start sendMessage, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}
	printf("Message sent.\n");
}

int main(int argc, char* argv[])
{
	MQTTAsync client;
	MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
	MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
	MQTTAsync_token token;
	int rc;
	int ch;
	MQTTAsync_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
	MQTTAsync_setCallbacks(client, NULL, connlost, NULL, NULL);

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.onSuccess = onConnect;
	conn_opts.onFailure = onConnectFailure;
	conn_opts.context = client;
		
	if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS)
	{
		printf("Failed to start connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	printf("Waiting for publication of weight\n"
		 "on topic %s for client with ClientID: %s\n",
		 TOPIC, CLIENTID);
	while (!finished)
		#if defined(WIN32)
			Sleep(100);
		#else
			usleep(10000L);
		#endif

	MQTTAsync_destroy(&client);
 	return rc;
}
  
