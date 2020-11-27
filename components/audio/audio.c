#include "audio.h"
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <driver/i2s.h>
#include <driver/adc.h>
#include <esp_log.h>

static const char *TAG = "Audio";

// Pin corresponding to the AMP enable signal, turn on amp only when audio is going to be played
static const gpio_num_t AUDIO_AMP_SD_PIN = GPIO_NUM_25;

static xQueueHandle audioQueue = NULL;

static void PlayTask(void *arg)
{
	for ( ;; )
	{
		audio_r data;
		if (!xQueueReceive(audioQueue, &data, 4000 / portTICK_RATE_MS)) {
			continue;
		}

		ESP_LOGW(TAG, "Turn ON audio amp");
		ESP_ERROR_CHECK(gpio_set_level(AUDIO_AMP_SD_PIN, 1));

		// Wait to play the audio
		vTaskDelay(100);

		ESP_LOGW(TAG, "Audio play started, lenght : %d", data.length);

		size_t bytesWritten;
		i2s_write(I2S_NUM_0, data.buffer, data.length, &bytesWritten, portMAX_DELAY);
		i2s_zero_dma_buffer(I2S_NUM_0);

		vTaskDelay(100);

		ESP_LOGW(TAG, "Turn OFF audio amp");
		ESP_ERROR_CHECK(gpio_set_level(AUDIO_AMP_SD_PIN, 0));
	}
}

void audio_init(void)
{
	// Configure the amplifier shutdown signal
	gpio_config_t gpioConfig = {};

	gpioConfig.mode = GPIO_MODE_OUTPUT;
	gpioConfig.pin_bit_mask = 1ULL << AUDIO_AMP_SD_PIN;

	ESP_ERROR_CHECK(gpio_config(&gpioConfig));

	// Configure I2S to send the audio to.
	i2s_config_t i2sConfig= {};

	i2sConfig.mode = I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_DAC_BUILT_IN;
	i2sConfig.sample_rate = 22050;
	i2sConfig.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
	i2sConfig.communication_format = I2S_COMM_FORMAT_I2S_MSB;
	i2sConfig.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
	i2sConfig.dma_buf_count = 8;
	i2sConfig.dma_buf_len = 64;

	ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2sConfig, 0, NULL));
	ESP_ERROR_CHECK(i2s_set_dac_mode(I2S_DAC_CHANNEL_LEFT_EN));

	// Create task for playing sounds so that our main task isn't blocked
	audioQueue = xQueueCreate(2, sizeof(audio_r));
	assert(audioQueue);

	// Create a task on the second core
	BaseType_t taskResult = xTaskCreatePinnedToCore(&PlayTask, "I2S Task", 4096, NULL, 5, NULL, 1);
	if (taskResult != pdTRUE) {
		assert(pdFAIL);
	}

	ESP_LOGW(TAG, "Audio initialized.");
}

void audioPlay(uint8_t* buffer, int length)
{
	audio_r data = {};

	data.buffer = buffer;
	data.length = length;

	if (!uxQueueSpacesAvailable(audioQueue)) {
		ESP_LOGE(TAG, "No room in queue for audio.");
		return;
	}

	ESP_LOGW(TAG, "Sending audio to queue.");

	xQueueSendToBack(audioQueue, &data, portMAX_DELAY);
}

void audioPlayAuthorized(void)
{

}

void audioPlayDenied(void)
{

}