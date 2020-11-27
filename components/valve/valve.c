#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <driver/adc.h>
#include <esp_log.h>

#include "valve.h"
#include "message.h"

static const char *TAG = "Valve";

static const int OPERATION_TIME = 5000;

// Pin corresponding to the AMP enable signal, turn on amp only when audio is going to be played
static const gpio_num_t VALVE_ENABLE_PIN = GPIO_NUM_22;

static const gpio_num_t VALVE_2_FOWARD_PIN = GPIO_NUM_16;
static const gpio_num_t VALVE_2_REVERSE_PIN = GPIO_NUM_17;
static const gpio_num_t VALVE_1_FOWARD_PIN = GPIO_NUM_18;
static const gpio_num_t VALVE_1_REVERSE_PIN = GPIO_NUM_19;

static TaskHandle_t openValve1Task = NULL;
static TaskHandle_t closeValve1Task = NULL;
static TaskHandle_t openValve2Task = NULL;
static TaskHandle_t closeValve2Task = NULL;

static int valve1Stat = VALVE_CLOSED;
static int valve2Stat = VALVE_CLOSED;

void enable_motor_controller()
{
	gpio_set_level(VALVE_ENABLE_PIN, 1);

	ESP_LOGI(TAG, "Motor controller enabled");
}

void disable_motor_controller()
{
	gpio_set_level(VALVE_ENABLE_PIN, 0);

	ESP_LOGI(TAG, "Motor controller disabled");
}

static void open_valve1_task(void *pvParameters)
{
	uint32_t ulNotifiedValue;

	/* This task is also defined within an infinite loop. */
	for (;;)
	{
		/* Block to wait for a notification */
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (ulNotifiedValue > 0)
		{
			/* Enables the motor controller before opening/closing the valve */
			enable_motor_controller();

			/* Perform any processing necessitated by the interrupt. */
			gpio_set_level(VALVE_1_FOWARD_PIN, 1);
			vTaskDelay(OPERATION_TIME / portTICK_RATE_MS);
			gpio_set_level(VALVE_1_FOWARD_PIN, 0);

			/* once the operation is complete turn off the motor controller till next operation */
			disable_motor_controller();

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

	/* This task is also defined within an infinite loop. */
	for (;;)
	{
		/* Block to wait for a notification */
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (ulNotifiedValue > 0)
		{
			/* Enables the motor controller before opening/closing the valve */
			enable_motor_controller();

			/* Perform any processing necessitated by the interrupt. */
			gpio_set_level(VALVE_1_REVERSE_PIN, 1);
			vTaskDelay(OPERATION_TIME / portTICK_RATE_MS);
			gpio_set_level(VALVE_1_REVERSE_PIN, 0);

			/* once the operation is complete turn off the motor controller till next operation */
			disable_motor_controller();

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

	/* This task is also defined within an infinite loop. */
	for (;;)
	{
		/* Block to wait for a notification */
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (ulNotifiedValue > 0)
		{
			/* Enables the motor controller before opening/closing the valve */
			enable_motor_controller();

			/* Perform any processing necessitated by the interrupt. */
			gpio_set_level(VALVE_2_FOWARD_PIN, 1);
			vTaskDelay(OPERATION_TIME / portTICK_RATE_MS);
			gpio_set_level(VALVE_2_FOWARD_PIN, 0);

			/* once the operation is complete turn off the motor controller till next operation */
			disable_motor_controller();

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

	/* This task is also defined within an infinite loop. */
	for (;;)
	{
		/* Block to wait for a notification */
		ulNotifiedValue = ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		if (ulNotifiedValue > 0)
		{
			/* Enables the motor controller before opening/closing the valve */
			enable_motor_controller();

			/* Perform any processing necessitated by the interrupt. */
			gpio_set_level(VALVE_2_REVERSE_PIN, 1);
			vTaskDelay(OPERATION_TIME / portTICK_RATE_MS);
			gpio_set_level(VALVE_2_REVERSE_PIN, 0);

			/* once the operation is complete turn off the motor controller till next operation */
			disable_motor_controller();

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

void valve_init(void)
{
	gpio_config_t gpioConfig = {};

	gpioConfig.intr_type = GPIO_PIN_INTR_DISABLE;
    gpioConfig.mode = GPIO_MODE_OUTPUT;
    gpioConfig.pull_down_en = GPIO_PULLUP_ENABLE;
    gpioConfig.pull_up_en = GPIO_PULLDOWN_DISABLE;
    gpioConfig.pin_bit_mask = 
		  (1ULL << VALVE_ENABLE_PIN)
		| (1ULL << VALVE_1_FOWARD_PIN)
		| (1ULL << VALVE_1_REVERSE_PIN)
		| (1ULL << VALVE_2_FOWARD_PIN)
		| (1ULL << VALVE_2_REVERSE_PIN);

	ESP_ERROR_CHECK(gpio_config(&gpioConfig));

	BaseType_t taskResult;

	taskResult = xTaskCreate(&open_valve1_task, "open_valve_1", 2048, NULL, 3, &openValve1Task);
	if (taskResult != pdTRUE) {
		assert(pdFAIL);
	}

	taskResult = xTaskCreate(&close_valve1_task, "close_valve_1", 2048, NULL, 3, &closeValve1Task);
	if (taskResult != pdTRUE) {
		assert(pdFAIL);
	}

	taskResult = xTaskCreate(&open_valve2_task, "open_valve_2", 2048, NULL, 3, &openValve2Task);
	if (taskResult != pdTRUE) {
		assert(pdFAIL);
	}

	taskResult = xTaskCreate(&close_valve2_task, "close_valve_2", 2048, NULL, 3, &closeValve2Task);
	if (taskResult != pdTRUE) {
		assert(pdFAIL);
	}
}