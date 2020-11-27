#ifndef WIFI_H_
#define WIFI_H_

#include "esp_wifi.h"
#include "esp_wifi_types.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

void wifi_init(void);
void wifi_reset_nvs(void);
EventGroupHandle_t wifiGetEventGroup(void);
void wifi_used(void);
void publish_wifi_stat (void);

#define WIFI_CONNECTED_BIT	BIT0

#endif /* WIFI_H_ */