#include <nvs_flash.h>
#include <nvs.h>
#include <string.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_pm.h>

#include "wifi.h"
#include "wifi_access_point.h"
#include "wifi_client.h"
#include "http.h"
#include "mqtt_connection.h"
#include "die_sensors.h"
#include "datetime.h"
#include "audio.h"
#include "sounds.h"
#include "valve.h"

static const char *TAG = "Main";

void app_main(void) 
{
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

    int configMode = !gpio_get_level(0);

    if (configMode)
	{
    	ESP_LOGW(TAG, "Starting in config mode. Reseting all values to factory defaults.");

    	// Reset all NVS data so we always get known values and don't crash
    	wifiResetNVS();
    	wifiClientResetNVS();
    	mqttConnectionResetNVS();
//    	elasticResetNVS();
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

	valveInit();
    dateTimeInit();
    httpServerInit();
    mqttConnectionInit();
    dieSensorsInit();
	audioInit();                                                  

/*
	while(true)
	{
		audioPlay(dat, sizeof(dat));
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	}*/
}

