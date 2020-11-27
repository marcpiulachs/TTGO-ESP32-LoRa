#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>

static const char *TAG = "Button";

// Pin corresponding to the AMP enable signal, turn on amp only when audio is going to be played
static const gpio_num_t BUTTON_PIN = GPIO_NUM_23;

#define ESP_INTR_FLAG_DEFAULT 0

SemaphoreHandle_t semaphore = NULL;
 
void IRAM_ATTR button_isr_handler(void* arg) {
  xSemaphoreGiveFromISR(semaphore, NULL);
}

void button_task(void* arg) {
  for(;;) {
    if(xSemaphoreTake(semaphore,portMAX_DELAY) == pdTRUE) {
      //ESP_LOGW(TAG, "Oh, button pushed!\n");
    }
  }
}

void button_init(void)
{
	// Create a binary semaphore
	semaphore = xSemaphoreCreateBinary();

	// Setup the button GPIO pin
	gpio_pad_select_gpio(BUTTON_PIN);

	// Quite obvious, a button is a input
	gpio_set_direction(BUTTON_PIN, GPIO_MODE_INPUT);

	// Trigger the interrupt when going from HIGH -> LOW ( == pushing button)
	gpio_set_intr_type(BUTTON_PIN, GPIO_INTR_NEGEDGE);

	// Associate button_task method as a callback
	xTaskCreate(button_task, "button_task", 4096, NULL, 10, NULL);

	// Install default ISR service 
	gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

	// Add our custom button handler to ISR
	gpio_isr_handler_add(BUTTON_PIN, button_isr_handler, NULL);

	// Report to log
	ESP_LOGW(TAG, "Button initialized.");
}