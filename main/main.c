#include <nvs_flash.h>
#include <nvs.h>
#include <string.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_pm.h>

//#include "beeline.h"

#include "wifi.h"
#include "wifi_access_point.h"
#include "wifi_client.h"

#include "http.h"
#include "mqtt_connection.h"
#include "die_sensors.h"
#include "elastic.h"
//#include "display.h"
// #include "radio.h"
#include "datetime.h"
//#include "radio.h"
#include "audio.h"
#include "sounds.h"
#include <driver/i2s.h>
// #include "os.h"
// #include "sys/param.h"
// #include "crypto/base64.h"

static const char *TAG = "Main";

void makenoise()
{
	// Configure the I2S driver

	{
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
	}
       while(true)
       {
    	   	   size_t bytesWritten;
       			i2s_write(I2S_NUM_0, dat, sizeof(dat), &bytesWritten, portMAX_DELAY);
       			i2s_zero_dma_buffer(I2S_NUM_0);

       		 vTaskDelay(1000 / portTICK_PERIOD_MS);
       		ESP_LOGW(TAG, "Su puta madre.");
       }

       while(true)
       {
			size_t bytesWritten;
		i2s_write(I2S_NUM_0, dat, sizeof(dat), &bytesWritten, portMAX_DELAY);
		i2s_zero_dma_buffer(I2S_NUM_0);

		vTaskDelay(1000 / portTICK_PERIOD_MS);

		
	   }

}

void app_main(void) 
{
	makenoise();

	esp_err_t espError;

	ESP_LOGW(TAG, "Starting up device....");
    ESP_LOGI(TAG, "Free memory: %d bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "IDF version: %s", esp_get_idf_version());

    //Initialize NVS
	espError = nvs_flash_init();

    if (espError == ESP_ERR_NVS_NO_FREE_PAGES) {
		ESP_ERROR_CHECK(nvs_flash_erase());
      	ESP_ERROR_CHECK(nvs_flash_init());
	}

	// Set device unique ID
    nvs_handle nvsHandle;
	ESP_ERROR_CHECK(nvs_open("BeelineNVS", NVS_READWRITE, &nvsHandle));

	char deviceName[16] = {0};
	size_t nvsLength = sizeof(deviceName);
	espError = nvs_get_str(nvsHandle, "uniqueName", deviceName, &nvsLength);

	if (espError == ESP_ERR_NVS_NOT_FOUND)
	{
		uint8_t mac[6];
	    char id_string[8] = {0};
	    esp_read_mac(mac, ESP_MAC_WIFI_STA);

	    id_string[0] = 'a' + ((mac[3] >> 0) & 0x0F);
	    id_string[1] = 'a' + ((mac[3] >> 4) & 0x0F);
	    id_string[2] = 'a' + ((mac[4] >> 0) & 0x0F);
	    id_string[3] = 'a' + ((mac[4] >> 4) & 0x0F);
	    id_string[4] = 'a' + ((mac[5] >> 0) & 0x0F);
	    id_string[5] = 'a' + ((mac[5] >> 4) & 0x0F);
	    id_string[6] = 0;

	    ESP_LOGW(TAG, "id_string %s, B:%d, 1:%d, 2:%d, A1:%d, A2:%d",
	    	id_string,
	    	mac[3],
	    	((mac[3] >> 0) & 0x0F),
	    	((mac[3] >> 4) & 0x0F),
	    	id_string[0],
	    	id_string[1]
	    );

		ESP_ERROR_CHECK(nvs_set_str(nvsHandle, "uniqueName", id_string));
		ESP_ERROR_CHECK(nvs_commit(nvsHandle));
		
		nvs_close(nvsHandle);
	}

    ESP_LOGW(TAG, "Device name: %s", deviceName);
	
	// Setup config button
	gpio_config_t io_conf;
    io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (0b1 << 0);
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    ESP_LOGW(TAG, "Hold PRG for config mode.");

    vTaskDelay(2000 / portTICK_PERIOD_MS);

    int configMode = 0;

    configMode = !gpio_get_level(0);

    if (configMode){

    	ESP_LOGW(TAG, "Starting in config mode. Reseting all values to factory defaults.");

    	// Reset all NVS data so we always get known values and don't crash
    	wifiResetNVS();
    	wifiClientResetNVS();
    	mqttConnectionResetNVS();
    	elasticResetNVS();
    	datTimeResetNVS();
  //  	radioResetNVS();
    	dieSensorsResetNVS();

		/* Starting up as a configuration AP */
    	wifiAccessPointInit();
    }
    else
	{
    	ESP_LOGW(TAG, "Starting in normal mode.");
    
		/* Starting up as a network client device */
		wifiClientInit();
    }

    #if CONFIG_PM_ENABLE
	    // Configure dynamic frequency scaling:
	    // maximum and minimum frequencies are set in sdkconfig,
	    // automatic light sleep is enabled if tickless idle support is enabled.
	    esp_pm_config_esp32_t pm_config = {
	            .max_freq_mhz = CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ,
	            .min_freq_mhz = CONFIG_ESP32_DEFAULT_CPU_FREQ_MHZ,
		#if CONFIG_FREERTOS_USE_TICKLESS_IDLE
	            .light_sleep_enable = true
		#endif
    	};
    	ESP_ERROR_CHECK( esp_pm_configure(&pm_config) );
	#endif // CONFIG_PM_ENABLE

/*
    dateTimeInit();

    httpServerInit();

    mqttConnectionInit();

    displayInit();

    elasticInit();

    dieSensorsInit();
*/
	//Odroid_InitializeAudio();


	//Odroid_PlayAudio(dat, sizeof(dat));
}

