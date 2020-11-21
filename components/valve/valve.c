#include "valve.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <driver/adc.h>
#include <esp_log.h>

#define TAG "valve"

// Pin corresponding to the AMP enable signal, turn on amp only when audio is going to be played
static const gpio_num_t FOWARD_PIN = GPIO_NUM_25;
static const gpio_num_t REVERSE_PIN = GPIO_NUM_25;

static void open_valve_task(void *pvParameters)
{
	uint32_t ulNotifiedValue;

	for (;;)
	{
		/* Block to wait for a notification.  Here the RTOS task notification
		is being used as a counting semaphore.  The task's notification value
		is incremented each time the ISR calls vTaskNotifyGiveFromISR(), and
		decremented each time the RTOS task calls ulTaskNotifyTake() - so in
		effect holds a count of the number of outstanding interrupts.  The
		first parameter is set to pdTRUE, clearing the task's notification
		value to 0, meaning each outstanding outstanding deferred interrupt
		event must be processed before ulTaskNotifyTake() is called again. */
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (ulNotifiedValue > 0)
		{
			/* Perform any processing necessitated by the interrupt. */
			gpio_set_level(FOWARD_PIN, 1);
			vTaskDelay(500 / portTICK_RATE_MS);
			gpio_set_level(FOWARD_PIN, 0);
			
			ESP_LOGI(TAG, "Valve opened successfully");
		}
		else
		{
			/* Did not receive a notification within the expected time. */
		}
	}
}

static void close_valve_task(void *pvParameters)
{
	uint32_t ulNotifiedValue;

	for (;;)
	{
		/* Block to wait for a notification.  Here the RTOS task notification
		is being used as a counting semaphore.  The task's notification value
		is incremented each time the ISR calls vTaskNotifyGiveFromISR(), and
		decremented each time the RTOS task calls ulTaskNotifyTake() - so in
		effect holds a count of the number of outstanding interrupts.  The
		first parameter is set to pdTRUE, clearing the task's notification
		value to 0, meaning each outstanding outstanding deferred interrupt
		event must be processed before ulTaskNotifyTake() is called again. */
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (ulNotifiedValue > 0)
		{
			/* Perform any processing necessitated by the interrupt. */
			gpio_set_level(REVERSE_PIN, 1);
			vTaskDelay(500 / portTICK_RATE_MS);
			gpio_set_level(REVERSE_PIN, 0);

			ESP_LOGI(TAG, "Valve closed successfully");
		}
		else
		{
			/* Did not receive a notification within the expected time. */
		}
	}
}

void open_valve()
{
	// open valve
	xTaskNotifyGive( open_valve_task );	
}

void close_valve()
{
	// close valve
	xTaskNotifyGive( close_valve_task );	
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

	result = xTaskCreate(&open_valve_task, "Open Valve", 2048, NULL, 3, NULL);
	assert(result == pdPASS);

	result = xTaskCreate(&close_valve_task, "Close Valve", 2048, NULL, 3, NULL);
	assert(result == pdPASS);
}