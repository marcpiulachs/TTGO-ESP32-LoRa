#include "audio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <driver/adc.h>
#include <esp_log.h>

#define TAG "audio"
//static const gpio_num_t AUDIO_AMP_SD_PIN = GPIO_NUM_25;

static xQueueHandle gQueue = NULL;

typedef struct
{
	uint8_t* buffer;
	int length;
} QueueData;

static void PlayTask(void *arg)
{
	while(true)
	{
		QueueData data;
		if (!xQueueReceive(gQueue, &data, 4000 / portTICK_RATE_MS))
		{
			continue;
		}

		ESP_LOGW(TAG, "Audio play started, lenght : %d", data.length);

		size_t bytesWritten;
		i2s_write(I2S_NUM_0, data.buffer, data.length, &bytesWritten, portMAX_DELAY);
		i2s_zero_dma_buffer(I2S_NUM_0);

//		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}

void Odroid_InitializeAudio(void)
{
	// Configure the amplifier shutdown signal
/*
	{
		gpio_config_t gpioConfig = {};

		gpioConfig.mode = GPIO_MODE_OUTPUT;
		gpioConfig.pin_bit_mask = 1ULL << AUDIO_AMP_SD_PIN;

		ESP_ERROR_CHECK(gpio_config(&gpioConfig));

		gpio_set_level(AUDIO_AMP_SD_PIN, 1);
	}
*/
	// Configure the I2S driver
	i2s_config_t i2sConfig= {};

	i2sConfig.mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN;
	i2sConfig.sample_rate = 8000;
	i2sConfig.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
	i2sConfig.communication_format = I2S_COMM_FORMAT_I2S_MSB;
	i2sConfig.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
	i2sConfig.dma_buf_count = 8;
	i2sConfig.dma_buf_len = 64;

	ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL));
	ESP_ERROR_CHECK(i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN));

	// Create task for playing sounds so that our main task isn't blocked
	{
		gQueue = xQueueCreate(1, sizeof(QueueData));
		assert(gQueue);

/*
		BaseType_t result = xTaskCreatePinnedToCore(&PlayTask, "I2S Task", 1024, NULL, 5, NULL, 1);
		assert(result == pdPASS);*/

		xTaskCreate(&PlayTask, "I2S Task", 4096, NULL, 5, NULL);
	}

	ESP_LOGW(TAG, "Audio initialized.");
}

void Odroid_PlayAudio(uint8_t* buffer, int length)
{
	QueueData data = {};

	data.buffer = buffer;
	data.length = length;

	if (!uxQueueSpacesAvailable(gQueue)) {
		ESP_LOGE(TAG, "No room in queue for audio.");
		return;
	}

	ESP_LOGW(TAG, "Sending audio to queue.");

	xQueueSendToBack(gQueue, &data, portMAX_DELAY);
}