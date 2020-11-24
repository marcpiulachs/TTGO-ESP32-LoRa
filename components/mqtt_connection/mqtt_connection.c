#include <nvs.h>
#include <mqtt_client.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <esp_log.h>
#include <sys/param.h>

#include "wifi.h"
// #include "http.h"
#include "mqtt_connection.h"
#include "message.h"
#include "valve.h"
#include "die_sensors.h"
#include "audio.h"

static esp_mqtt_client_handle_t client;
static esp_mqtt_client_config_t mqtt_cfg;

static xQueueHandle mqttConnectionQueue = NULL;

static EventGroupHandle_t mqttConnectionEventGroup;

static const char *TAG = "MQTT";

char deviceName[16] = {0};

char mqttOutTopic[64] = {0};

char mqttValve1CmndTopic[64] = {0};
char mqttValve1StatTopic[64] = {0};
char mqttValve1TeleTopic[64] = {0};

char mqttValve2CmndTopic[64] = {0};
char mqttValve2StatTopic[64] = {0};
char mqttValve2TeleTopic[64] = {0};

char mqttRSSIStatTopic[64] = {0};
char mqttRSSITeleTopic[64] = {0};

char mqttBatteryStatTopic[64] = {0};
char mqttBatteryTeleTopic[64] = {0};

char mqttWifiStatTopic[64] = {0};
char mqttWifiTeleTopic[64] = {0};

char mqttRequestCmndTopic[64] = {0};
char mqttRequestTeleTopic[64] = {0};

static esp_err_t mqtt_event_handler(esp_mqtt_event_handle_t event) 
{
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    // your_context_t *context = event->context;
    switch (event->event_id) 
	{
        case MQTT_EVENT_CONNECTED:
        	xEventGroupSetBits(mqttConnectionEventGroup, MQTT_CONNECTED_BIT);
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			
			ESP_LOGI(TAG, "Subscribe to topic : %s", mqttValve1CmndTopic);
            msg_id = esp_mqtt_client_subscribe(client, mqttValve1CmndTopic, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			ESP_LOGI(TAG, "Subscribe to topic : %s", mqttValve1StatTopic);
            msg_id = esp_mqtt_client_subscribe(client, mqttValve1StatTopic, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			ESP_LOGI(TAG, "Subscribe to topic : %s", mqttValve2CmndTopic);
            msg_id = esp_mqtt_client_subscribe(client, mqttValve2CmndTopic, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			ESP_LOGI(TAG, "Subscribe to topic : %s", mqttValve2StatTopic);
            msg_id = esp_mqtt_client_subscribe(client, mqttValve2StatTopic, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			ESP_LOGI(TAG, "Subscribe to topic : %s", mqttRSSIStatTopic);
            msg_id = esp_mqtt_client_subscribe(client, mqttRSSIStatTopic, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			ESP_LOGI(TAG, "Subscribe to topic : %s", mqttBatteryStatTopic);
            msg_id = esp_mqtt_client_subscribe(client, mqttBatteryStatTopic, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			ESP_LOGI(TAG, "Subscribe to topic : %s", mqttWifiStatTopic);
            msg_id = esp_mqtt_client_subscribe(client, mqttWifiStatTopic, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

			ESP_LOGI(TAG, "Subscribe to topic : %s", mqttRequestCmndTopic);
            msg_id = esp_mqtt_client_subscribe(client, mqttRequestCmndTopic, 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);


			/*
            msg_id = esp_mqtt_client_publish(client, "/topic/qos1", "data_3", 0, 1, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos0", 0);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_subscribe(client, "/topic/qos1", 1);
            ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

            msg_id = esp_mqtt_client_unsubscribe(client, "/topic/qos1");
            ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
			*/

		break;

		case MQTT_EVENT_BEFORE_CONNECT:
			// wifi_used();
		break;

        case MQTT_EVENT_DISCONNECTED:
        	xEventGroupClearBits(mqttConnectionEventGroup, MQTT_CONNECTED_BIT);
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
		break;

        case MQTT_EVENT_SUBSCRIBED:
        	wifi_used();
            ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
		break;

        case MQTT_EVENT_UNSUBSCRIBED:
        	wifi_used();
            ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
		break;

        case MQTT_EVENT_PUBLISHED:
        	wifi_used();
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
		break;

        case MQTT_EVENT_DATA:
        	wifi_used();
			ESP_LOGI(TAG, "MQTT_EVENT_DATA");
			
			printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
			printf("DATA=%.*s\r\n", event->data_len, event->data);			

			// Make safe copies, then call business logic handler

            // Get the topic name
            char *topic = malloc(event->topic_len + 1);
            memcpy(topic, event->topic, event->topic_len);
            topic[event->topic_len] = '\0';

            // Get the data
            char *data = malloc(event->data_len + 1);
            memcpy(data, event->data, event->data_len);
            data[event->data_len] = '\0';

    		// Handle it already
    		handle_mqtt_event_data(topic, data);

 			free(topic);
            free(data);

			break;

        case MQTT_EVENT_ERROR:
            ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
		break;

		case MQTT_EVENT_ANY:
			ESP_LOGI(TAG, "MQTT_EVENT_ANY");
			break;
    }
    return ESP_OK;
}

void replace_topic_token(char *str, char *orig, char *rep, char *result)
{
  static char buffer[64];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    strcpy(result, str);

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$
  buffer[p-str] = '\0';

  sprintf(buffer+(p-str), "%s%s", rep, p+strlen(orig));

  strcpy(result, buffer);
}

void init_topic_routes (void)
{
	size_t nvsLength;
	nvs_handle nvsHandle;
	ESP_ERROR_CHECK(nvs_open("BeelineNVS", NVS_READONLY, &nvsHandle));

	nvsLength = sizeof(deviceName);
	ESP_ERROR_CHECK(nvs_get_str(nvsHandle, "uniqueName", deviceName, &nvsLength));

	nvsLength = sizeof(mqttOutTopic);
	ESP_ERROR_CHECK(nvs_get_str(nvsHandle, "mqttOutTopic", mqttOutTopic, &nvsLength));

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttValve1CmndTopic);
	replace_topic_token(mqttValve1CmndTopic, "{topic}", "valve1", mqttValve1CmndTopic);
	replace_topic_token(mqttValve1CmndTopic, "{prefix}", "cmnd", mqttValve1CmndTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttValve1StatTopic);
	replace_topic_token(mqttValve1StatTopic, "{topic}", "valve1", mqttValve1StatTopic);
	replace_topic_token(mqttValve1StatTopic, "{prefix}", "stat", mqttValve1StatTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttValve1TeleTopic);
	replace_topic_token(mqttValve1TeleTopic, "{topic}", "valve1", mqttValve1TeleTopic);
	replace_topic_token(mqttValve1TeleTopic, "{prefix}", "tele", mqttValve1TeleTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttValve2CmndTopic);
	replace_topic_token(mqttValve2CmndTopic, "{topic}", "valve2", mqttValve2CmndTopic);
	replace_topic_token(mqttValve2CmndTopic, "{prefix}", "cmnd", mqttValve2CmndTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttValve2StatTopic);
	replace_topic_token(mqttValve2StatTopic, "{topic}", "valve2", mqttValve2StatTopic);
	replace_topic_token(mqttValve2StatTopic, "{prefix}", "stat", mqttValve2StatTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttValve2TeleTopic);
	replace_topic_token(mqttValve2TeleTopic, "{topic}", "valve2", mqttValve2TeleTopic);
	replace_topic_token(mqttValve2TeleTopic, "{prefix}", "tele", mqttValve2TeleTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttRSSIStatTopic);
	replace_topic_token(mqttRSSIStatTopic, "{topic}", "rssi", mqttRSSIStatTopic);
	replace_topic_token(mqttRSSIStatTopic, "{prefix}", "stat", mqttRSSIStatTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttRSSITeleTopic);
	replace_topic_token(mqttRSSITeleTopic, "{topic}", "rssi", mqttRSSITeleTopic);
	replace_topic_token(mqttRSSITeleTopic, "{prefix}", "tele", mqttRSSITeleTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttBatteryStatTopic);
	replace_topic_token(mqttBatteryStatTopic, "{topic}", "battery", mqttBatteryStatTopic);
	replace_topic_token(mqttBatteryStatTopic, "{prefix}", "stat", mqttBatteryStatTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttBatteryTeleTopic);
	replace_topic_token(mqttBatteryTeleTopic, "{topic}", "battery", mqttBatteryTeleTopic);
	replace_topic_token(mqttBatteryTeleTopic, "{prefix}", "tele", mqttBatteryTeleTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttWifiStatTopic);
	replace_topic_token(mqttWifiStatTopic, "{topic}", "wifi", mqttWifiStatTopic);
	replace_topic_token(mqttWifiStatTopic, "{prefix}", "stat", mqttWifiStatTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttWifiTeleTopic);
	replace_topic_token(mqttWifiTeleTopic, "{topic}", "wifi", mqttWifiTeleTopic);
	replace_topic_token(mqttWifiTeleTopic, "{prefix}", "tele", mqttWifiTeleTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttRequestTeleTopic);
	replace_topic_token(mqttRequestTeleTopic, "{topic}", "request", mqttRequestTeleTopic);
	replace_topic_token(mqttRequestTeleTopic, "{prefix}", "tele", mqttRequestTeleTopic);

	replace_topic_token(mqttOutTopic, "{device}", deviceName, mqttRequestCmndTopic);
	replace_topic_token(mqttRequestCmndTopic, "{topic}", "request", mqttRequestCmndTopic);
	replace_topic_token(mqttRequestCmndTopic, "{prefix}", "cmnd", mqttRequestCmndTopic);	

	nvs_close(nvsHandle);
}

/*
 * MQTT : Handle business logic
 */
void handle_mqtt_event_data(char *topic, char *payload) 
{
	if (strcmp(topic, mqttValve1CmndTopic) == 0) 
	{
		if (strcmp(payload, "OPEN") == 0) 
		{
			ESP_LOGI(TAG, "Valve[1] OPEN requested");
			open_valve1();
		}
		else if (strcmp(payload, "CLOSE") == 0) 
		{
			ESP_LOGI(TAG, "Valve[1] CLOSE requested");
			close_valve1();
		}
		else
		{
			ESP_LOGE(TAG, "Unknown payload received %s", payload);
		}		
	} 
	else if (strcmp(topic, mqttValve2CmndTopic) == 0) 
	{
		if (strcmp(payload, "OPEN") == 0) 
		{
			ESP_LOGI(TAG, "Valve[2] OPEN requested");
			open_valve2();
		}
		else if (strcmp(payload, "CLOSE") == 0) 
		{
			ESP_LOGI(TAG, "Valve[2] CLOSE requested");
			close_valve2();
		}
		else
		{
			ESP_LOGE(TAG, "Unknown payload received %s", payload);
		}		
	}
	else if (strcmp(topic, mqttValve1StatTopic) == 0) 
	{
		ESP_LOGI(TAG, "Valve[1] STAT requested");
		publish_valve1_stat();
	}
	else if (strcmp(topic, mqttValve2StatTopic) == 0) 
	{
		ESP_LOGI(TAG, "Valve[2] STAT requested");
		publish_valve2_stat();
	}
	else if (strcmp(topic, mqttBatteryStatTopic) == 0) 
	{
		ESP_LOGI(TAG, "Battery STAT requested");
		publish_battery_stat();
	}
	else if (strcmp(topic, mqttRSSIStatTopic) == 0) 
	{
		ESP_LOGI(TAG, "RSSI STAT requested");
		publish_rssi_stat();
	}
	else if (strcmp(topic, mqttWifiStatTopic) == 0) 
	{
		ESP_LOGI(TAG, "WiFi STAT requested");
		publish_wifi_stat();
	}
	else if (strcmp(topic, mqttRequestCmndTopic) == 0) 
	{
		if (strcmp(payload, "AUTHORIZED") == 0)
		{
			ESP_LOGI(TAG, "Service request authorized received");
			audioPlayAuthorized();
		}
		else if (strcmp(payload, "DENIED") == 0)
		{
			ESP_LOGI(TAG, "Service requested denied received");
			audioPlayDenied();
		}
		else
		{
			ESP_LOGE(TAG, "Unknown payload received %s", payload);
		}
	}	
}

void mqttConnectionSetClient(void)
{
	nvs_handle nvsHandle;
	ESP_ERROR_CHECK(nvs_open("BeelineNVS", NVS_READONLY, &nvsHandle));

    size_t nvsLength;

    char host[CONFIG_HTTP_NVS_MAX_STRING_LENGTH];
    nvsLength = sizeof(host);
	nvs_get_str(nvsHandle, "mqttHost", host, &nvsLength);

	unsigned int port;
	nvs_get_u32(nvsHandle, "mqttPort", &port);

	char username[CONFIG_HTTP_NVS_MAX_STRING_LENGTH];
	nvsLength = sizeof(username);
	nvs_get_str(nvsHandle, "mqttUsername", username, &nvsLength);

	char password[CONFIG_HTTP_NVS_MAX_STRING_LENGTH];
	nvsLength = sizeof(password);
	nvs_get_str(nvsHandle, "mqttPassword", password, &nvsLength);

	unsigned int keepalive;
	nvs_get_u32(nvsHandle, "mqttKeepalive", &keepalive);

	char uniqueName[16];
	nvsLength = sizeof(uniqueName);
	nvs_get_str(nvsHandle, "uniqueName", uniqueName, &nvsLength);

	nvs_close(nvsHandle);

	ESP_LOGI(TAG, "Got MQTT host details...\n");

	mqtt_cfg.host = host;
    mqtt_cfg.port = port;
    mqtt_cfg.client_id = uniqueName;
    mqtt_cfg.username = username;
    mqtt_cfg.password = password;
    mqtt_cfg.keepalive = keepalive;
    mqtt_cfg.event_handle = mqtt_event_handler;
    // mqtt_cfg.user_context = (void *)your_context;

    ESP_LOGI(TAG, "Connecting to %s ...\n", host);

    client = esp_mqtt_client_init(&mqtt_cfg);
}

void mqttConnectionWiFiConnected(void)
{
	mqttConnectionSetClient();
	esp_mqtt_client_start(client);
}

void mqttConnectionWiFiDisconnected(void)
{
	esp_mqtt_client_stop(client);
}

static void mqttConnectionTask(void *arg)
{
	nvs_handle nvsHandle;
	ESP_ERROR_CHECK(nvs_open("BeelineNVS", NVS_READONLY, &nvsHandle));

	size_t nvsLength;

	char mqttOutTopic[CONFIG_HTTP_NVS_MAX_STRING_LENGTH];
	nvsLength = sizeof(mqttOutTopic);
	nvs_get_str(nvsHandle, "mqttOutTopic", mqttOutTopic, &nvsLength);

	nvs_close(nvsHandle);

	while (true){

		message_t message;
		if (!xQueueReceive(mqttConnectionQueue, &message, 4000 / portTICK_RATE_MS)) {
			continue;
		}

		wifi_used();

		EventBits_t eventBits = xEventGroupWaitBits(mqttConnectionEventGroup, MQTT_CONNECTED_BIT, false, true, 30000 / portTICK_RATE_MS);

		if (!(eventBits & MQTT_CONNECTED_BIT)) {
			ESP_LOGE(TAG, "Not connected after message received. Skipping message.");
			continue;
		}

		char mqttTopic[64];
		char mqttValue[64];
		switch(message.topicType)
		{
			case BATTERY_STAT:
				strcpy(mqttTopic, mqttBatteryTeleTopic);
				break;
			case RSSI_STAT:
				strcpy(mqttTopic, mqttRSSITeleTopic);
				break;			
			case VALVE1_STAT:
				strcpy(mqttTopic, mqttValve1TeleTopic);
				break;			
			case VALVE2_STAT:
				strcpy(mqttTopic, mqttValve2TeleTopic);
				break;
			case WIFI_STAT:
				strcpy(mqttTopic, mqttWifiTeleTopic);
				break;							
		}

		strcpy(mqttValue, message.stringValue);

		ESP_LOGI(TAG, "Publishing MQTT message to broker");
		int msg_id = esp_mqtt_client_publish(client, mqttTopic, mqttValue, 0, 1, 0);
		ESP_LOGI(TAG, "T: %s, V: %s -> MQTT %d\n", mqttTopic, mqttValue, msg_id);
		ESP_LOGI(TAG, "Sent publish successful, msg_id=%d", msg_id);
	}
}

void mqttConnectionQueueAdd(message_t * message) 
{
	if (!uxQueueSpacesAvailable(mqttConnectionQueue)) {
		ESP_LOGE(TAG, "No room in queue for message.");
		return;
	}

	xQueueSend(mqttConnectionQueue, message, 0);
}

void mqttConnectionInit(void)
{
	mqttConnectionEventGroup = xEventGroupCreate();

	mqttConnectionQueue = xQueueCreate(10, sizeof(message_t));
	assert(mqttConnectionQueue);

	BaseType_t taskResult = xTaskCreate(&mqttConnectionTask, "mqttConnection", 4096, NULL, 13, NULL);
	if (taskResult != pdTRUE) {
		assert(pdFAIL);
	}

	init_topic_routes();

	wifi_used();
}

void mqttConnectionResetNVS(void) 
{
	nvs_handle nvsHandle;

	ESP_ERROR_CHECK(nvs_open("BeelineNVS", NVS_READWRITE, &nvsHandle));

	ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "mqttHost", "mqtt.server.example.com"));
	ESP_ERROR_CHECK(nvs_set_u32(nvsHandle, "mqttPort", 1883));
	ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "mqttUsername", "Username"));
	ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "mqttPassword", "Password"));
	ESP_ERROR_CHECK(nvs_set_u32(nvsHandle, "mqttKeepalive", 30));
	ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "mqttOutTopic", "/hdo/{device}/{topic}/{prefix}"));

	ESP_ERROR_CHECK(nvs_commit(nvsHandle));

	nvs_close(nvsHandle);

	messageNVSReset("mqtt", 0x00);
}