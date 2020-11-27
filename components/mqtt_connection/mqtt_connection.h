#ifndef MQTT_H_
#define MQTT_H_

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <freertos/event_groups.h>

#include "message.h"

void mqtt_conn_init_topics(void);
void mqtt_conn_init(void);
void mqtt_conn_reset_nvs(void);
void mqtt_conn_queue_add(message_t * message);
void mqtt_conn_wifi_connected(void);
void mqtt_conn_wifi_disconnected(void);

void handle_mqtt_event_data(char *topic, char *payload);

#define MQTT_CONNECTED_BIT	BIT0

#endif