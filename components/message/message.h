#ifndef __MESSAGE_QUEUE_H_
#define __MESSAGE_QUEUE_H_

enum messageType_t 
{
	MESSAGE_INT,
	MESSAGE_FLOAT,
	MESSAGE_DOUBLE,
	MESSAGE_STRING,
	MESSAGE_INTERRUPT
};

enum messageTopic_t
{
	BATTERY_STAT,
	RSSI_STAT,
	VALVE1_STAT,
	VALVE2_STAT,
	WIFI_STAT
};

typedef struct{
	char deviceName[16];
	char sensorName[16];
	int valueType;
	int intValue;
	int topicType;
	float floatValue;
	double doubleValue;
	char stringValue[32];
} message_t;

enum messageEndpoint_t {
	LORA = 0,
	MQTT,
	ELASTICSEARCH,
	DISPLAY
};

void publish_message(message_t * messagePointer);
void messageNVSReset(char * from, unsigned char defaults);

#endif