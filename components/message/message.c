#include <nvs_flash.h>
#include <string.h>
#include <esp_log.h>

#include "message.h"
#include "mqtt_connection.h"

static const char *TAG = "Message";

unsigned char disaplyLine = 0;

void publish_message(message_t * messagePointer)
{
	//har valueString[sizeof(messagePointer->stringValue)] = {0};

	switch (messagePointer->valueType){

		case MESSAGE_INT:
			sprintf(messagePointer->stringValue, "%d", messagePointer->intValue);
		break;

		case MESSAGE_FLOAT:
			sprintf(messagePointer->stringValue, "%.4f", messagePointer->floatValue);
		break;

		case MESSAGE_DOUBLE:
			sprintf(messagePointer->stringValue, "%.8f", messagePointer->doubleValue);
		break;
/*
		case MESSAGE_STRING:
			sprintf(messagePointer->stringValue, "%s", messagePointer->stringValue);
		break;*/
	}

	ESP_LOGI(TAG, "Message : topic=%d, valueType=%d, value=%s",
		messagePointer->topicType,
		messagePointer->valueType,
		messagePointer->stringValue
	);

	nvs_handle nvsHandle;
	ESP_ERROR_CHECK(nvs_open("BeelineNVS", NVS_READONLY, &nvsHandle));

	char handle[32];
	strcat(handle, "InRt");

	unsigned char routing;
	nvs_get_u8(nvsHandle, handle, &routing);

	nvs_close(nvsHandle);

	ESP_LOGI(TAG, "Forwarding to MQTT");
	mqtt_conn_queue_add(messagePointer);

/*
	if ((routing >> LORA) & 0x01){
		ESP_LOGI(TAG, "Forwarding to LoRa");
		radioLoRaQueueAdd(messagePointer);
	}

	if ((routing >> MQTT) & 0x01){
		ESP_LOGI(TAG, "Forwarding to MQTT");
		mqtt_conn_queue_add(messagePointer);
	}

	if ((routing >> ELASTICSEARCH) & 0x01){
		ESP_LOGI(TAG, "Forwarding to Elasticsearch");
		elasticQueueAdd(messagePointer);
	}

	if ((routing >> DISPLAY) & 0x01){
		ESP_LOGI(TAG, "Forwarding to Display");
		displayQueueAdd(messagePointer);
	}*/

}

void messageNVSReset(char * from, unsigned char defaults)
{
	nvs_handle nvsHandle;
	ESP_ERROR_CHECK(nvs_open("BeelineNVS", NVS_READWRITE, &nvsHandle));

	char handle[32];
	strcpy(handle, from);
	strcat(handle, "InRt");

	ESP_ERROR_CHECK(nvs_set_u8(nvsHandle, handle, defaults));

	ESP_ERROR_CHECK(nvs_commit(nvsHandle));

	nvs_close(nvsHandle);
}