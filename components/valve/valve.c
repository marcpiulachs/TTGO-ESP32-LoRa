#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <driver/adc.h>
#include <esp_log.h>

#include "valve.h"
#include "message.h"

#define TAG "valve"

// Pin corresponding to the AMP enable signal, turn on amp only when audio is going to be played
static const gpio_num_t FOWARD_PIN = GPIO_NUM_5;
static const gpio_num_t REVERSE_PIN = GPIO_NUM_5;

static TaskHandle_t openValve1Task = NULL;
static TaskHandle_t closeValve1Task = NULL;
static TaskHandle_t openValve2Task = NULL;
static TaskHandle_t closeValve2Task = NULL;

static int valve1Stat = VALVE_CLOSED;
static int valve2Stat = VALVE_CLOSED;

static void open_valve1_task(void *pvParameters)
{
	uint32_t ulNotifiedValue;

	for (;;)
	{
		/* Block to wait for a notification */
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (ulNotifiedValue > 0)
		{
			/* Perform any processing necessitated by the interrupt. */
			gpio_set_level(FOWARD_PIN, 1);
			vTaskDelay(500 / portTICK_RATE_MS);
			gpio_set_level(FOWARD_PIN, 0);

			valve1Stat = VALVE_OPENED;
			
			ESP_LOGI(TAG, "Valve opened successfully");

			publish_valve1_stat();
		}
		else
		{
			/* Did not receive a notification within the expected time. */
		}
	}

	vTaskDelete(NULL);
}

static void close_valve1_task(void *pvParameters)
{
	uint32_t ulNotifiedValue;

	for (;;)
	{
		/* Block to wait for a notification */
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (ulNotifiedValue > 0)
		{
			/* Perform any processing necessitated by the interrupt. */
			gpio_set_level(REVERSE_PIN, 1);
			vTaskDelay(500 / portTICK_RATE_MS);
			gpio_set_level(REVERSE_PIN, 0);

			valve1Stat = VALVE_CLOSED;

			ESP_LOGI(TAG, "Valve closed successfully");

			publish_valve1_stat();
		}
		else
		{
			/* Did not receive a notification within the expected time. */
		}
	}

	vTaskDelete(NULL);
}

static void open_valve2_task(void *pvParameters)
{
	uint32_t ulNotifiedValue;

	for (;;)
	{
		/* Block to wait for a notification */
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (ulNotifiedValue > 0)
		{
			/* Perform any processing necessitated by the interrupt. */
			gpio_set_level(FOWARD_PIN, 1);
			vTaskDelay(500 / portTICK_RATE_MS);
			gpio_set_level(FOWARD_PIN, 0);

			valve2Stat = VALVE_OPENED;
			
			ESP_LOGI(TAG, "Valve opened successfully");

			publish_valve2_stat();
		}
		else
		{
			/* Did not receive a notification within the expected time. */
		}
	}

	vTaskDelete(NULL);
}

static void close_valve2_task(void *pvParameters)
{
	uint32_t ulNotifiedValue;

	for (;;)
	{
		/* Block to wait for a notification */
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (ulNotifiedValue > 0)
		{
			/* Perform any processing necessitated by the interrupt. */
			gpio_set_level(REVERSE_PIN, 1);
			vTaskDelay(500 / portTICK_RATE_MS);
			gpio_set_level(REVERSE_PIN, 0);

			valve2Stat = VALVE_CLOSED;

			ESP_LOGI(TAG, "Valve closed successfully");

			publish_valve2_stat();
		}
		else
		{
			/* Did not receive a notification within the expected time. */
		}
	}

	vTaskDelete(NULL);
}

void open_valve1(void)
{
	// open valve
	xTaskNotifyGive (openValve1Task);
}

void close_valve1(void)
{
	// close valve
	xTaskNotifyGive (closeValve1Task);
}

void open_valve2(void)
{
	// open valve
	xTaskNotifyGive (openValve2Task);
}

void close_valve2(void)
{
	// close valve
	xTaskNotifyGive (closeValve2Task);
}

void publish_valve1_stat(void)
{
	message_t message;
	message.valueType = MESSAGE_INT;
	message.topicType = VALVE1_STAT;
	message.intValue = valve1Stat;

	publish_message(&message);
}

void publish_valve2_stat(void)
{
	message_t message;
	message.valueType = MESSAGE_INT;
	message.topicType = VALVE2_STAT;
	message.intValue = valve2Stat;

	publish_message(&message);
}

void valveInit(void)
{
	gpio_config_t gpioConfig = {};

	gpioConfig.mode = GPIO_MODE_OUTPUT;
	gpioConfig.pin_bit_mask = 1ULL << FOWARD_PIN;

	ESP_ERROR_CHECK(gpio_config(&gpioConfig));

	gpioConfig.mode = GPIO_MODE_OUTPUT;
	gpioConfig.pin_bit_mask = 1ULL << REVERSE_PIN;

	ESP_ERROR_CHECK(gpio_config(&gpioConfig));

	BaseType_t result;

	result = xTaskCreate(&open_valve1_task, "Open Valve [1]", 2048, NULL, 3, &openValve1Task);
	assert(result == pdPASS);

	result = xTaskCreate(&close_valve1_task, "Close Valve [1]", 2048, NULL, 3, &closeValve1Task);
	assert(result == pdPASS);

	result = xTaskCreate(&open_valve2_task, "Open Valve [2]", 2048, NULL, 3, &openValve2Task);
	assert(result == pdPASS);

	result = xTaskCreate(&close_valve2_task, "Close Valve [2]", 2048, NULL, 3, &closeValve2Task);
	assert(result == pdPASS);
}