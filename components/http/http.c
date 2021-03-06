
#include <nvs.h>
#include <sys/param.h>
#include <esp_http_server.h>
#include <http_parser.h>
#include <esp_log.h>

#include "http.h"

#include "wifi.h"

#include "config_reset_html.h"
#include "config_device_html.h"
#include "config_diesensors_html.h"
#include "config_display_html.h"
#include "config_elasticsearch_html.h"
#include "config_lora_html.h"
#include "config_mqtt_html.h"
#include "config_ntp_html.h"
#include "config_wifi_html.h"
#include "index_html.h"
#include "javascript_js.h"
#include "menu_css.h"
#include "menu_html.h"
#include "style_css.h"

static const char *TAG = "Http";

static void httpServerURLDecode(char * input, int length) {

    char * output = input;
    char hex[3] = "\0\0\0";

    while (input[0] != '\0') {

    	if (!length--){
    		break;
    	}


    	if (input[0] == '+') {
    		input[0] = ' ';
    		input+= 1;
    	}

    	else if (input[0] == '%') {

    		hex[0] = input[1];
    		hex[1] = input[2];
    		output[0] = strtol(hex, NULL, 16);
    		input+= 3;
    	}

    	else{
    		output[0] = input[0];
    		input+= 1;
    	}

    	output+= 1;
    }

    output[0] = '\0';
}

static char * httpServerParseValues(tokens_t * tokens, char * buffer, const char * rowDelimiter, const char * valueDelimiter, const char * endMatch){

	tokens->length = 0;

	// Start parsing the values by creating a new string from the payload
	char * token = strtok(buffer, rowDelimiter);

	char * end = buffer + strlen(buffer);

	// break apart the string getting all the parts delimited by &
	while (token != NULL) {

		if (strlen(endMatch) > 0){
			end = token + strlen(token) + 1;

			if (strncmp(end, endMatch, strlen(endMatch)) == 0) {
				end+= strlen(endMatch);
				break;
			}
		}

		tokens->tokens[tokens->length++].key = token;


		token = strtok(NULL, rowDelimiter);
	}

	// Re-parse the strigns and break them apart into key / value pairs
	for (unsigned int index = 0; index < tokens->length; index++){

		tokens->tokens[index].key = strtok(tokens->tokens[index].key, valueDelimiter);

		tokens->tokens[index].value = strtok(NULL, valueDelimiter);

		// If the value is NULL, make it point to an empty string.
		if (tokens->tokens[index].value == NULL){
			tokens->tokens[index].value = tokens->tokens[index].key + strlen(tokens->tokens[index].key);
		}

		httpServerURLDecode(tokens->tokens[index].key, MAX_HTTP_SSI_VALUE_LENGTH);
		httpServerURLDecode(tokens->tokens[index].value, MAX_HTTP_SSI_VALUE_LENGTH);
	}

	return end;
}


static char * httpServerGetTokenValue(tokens_t * tokens, const char * key){

	for (unsigned int index = 0; index < tokens->length; index++){

		if (strcmp(tokens->tokens[index].key, key) == 0){
			return tokens->tokens[index].value;
		}
	}

	return NULL;
}

#define START_SSI "<!--#"
#define END_SSI "-->"

static void httpGetBoolTagKey(char * ssiTagKeyTrimed, const char * ssiTagKey){

	char * end = strstr(ssiTagKey, ">>");

	if (!end){
		strcpy(ssiTagKeyTrimed, ssiTagKey);
	}

	int length = end - ssiTagKey;

	strncpy(ssiTagKeyTrimed, ssiTagKey, length);
}

static int httpGetBoolTagRotate(const char * ssiTagKey){

	char * rotateString = strstr(ssiTagKey, ">>");

	return rotateString ? atoi(rotateString + 2) : 0;
}

static void httpReaplceSSI(char * outBuffer, const char * fileStart, const char * fileEnd, const ssiTag_t * ssiTags, int ssiTagsLength) {

	char * file = (char * ) fileStart;
	char * out = outBuffer;

	int ssiTagsIndex = 0;
	char ssiKey[MAX_HTTP_SSI_KEY_LENGTH] = {"\0"};


	nvs_handle nvsHandle;
	ESP_ERROR_CHECK(nvs_open("BeelineNVS", NVS_READWRITE, &nvsHandle));

	unsigned int appendLength = 0;

	while (true)  {

		if ( ((fileEnd - file) < sizeof(START_SSI)) || (strncmp(file, START_SSI, strlen(START_SSI)) != 0) ) {


			appendLength = 1;

			if ((file + appendLength) > fileEnd) {
				nvs_close(nvsHandle);
				return;
			}

			strncpy(out, file, appendLength);

			file+= appendLength;
			out+= appendLength;

			continue;
		}

		file+= strlen(START_SSI);

		ssiKey[0] = '\0';

		while ( ((fileEnd - file) < sizeof(END_SSI)) || (strncmp(file, END_SSI, strlen(END_SSI)) != 0) ) {

			appendLength = 1;

			if ((file + appendLength) > fileEnd) {
				nvs_close(nvsHandle);
				return;
			}

			if (strlen(ssiKey) >= (sizeof(ssiKey) - 1)){
				file+= appendLength;
				continue;
			}

			strncat(ssiKey, file, appendLength);

			file+= appendLength;
		}

		file+= sizeof(END_SSI) - 1;

		char replaceSSIValue[MAX_HTTP_SSI_VALUE_LENGTH];
		strcpy(replaceSSIValue, "SSI Value Unhandled");

		for (ssiTagsIndex = 0; ssiTagsIndex < ssiTagsLength; ssiTagsIndex++) {

			const ssiTag_t ssiTag = ssiTags[ssiTagsIndex];

			if (strcmp(ssiKey, ssiTag.key) != 0) {
				continue;
			}


			char nvsStringValue[CONFIG_HTTP_NVS_MAX_STRING_LENGTH];
			size_t nvsLength = sizeof(nvsStringValue);
			uint32_t nvsIntValue;
			uint8_t nvsCharValue;
			char ssiKeyTrim[MAX_HTTP_SSI_KEY_LENGTH] = {"\0"};
			int rotate = 0;

			switch (ssiTag.type) {

				case SSI_TYPE_TEXT:
					nvs_get_str(nvsHandle, ssiTag.key, nvsStringValue, &nvsLength);
					nvsStringValue[nvsLength] = '\0';
					sprintf(replaceSSIValue, "<input type=\"text\" name=\"%s\" value=\"%s\" />", ssiTag.key, nvsStringValue);
				break;

				case SSI_TYPE_DISAPLY_TEXT:
					nvs_get_str(nvsHandle, ssiTag.key, nvsStringValue, &nvsLength);
					nvsStringValue[nvsLength] = '\0';
					sprintf(replaceSSIValue, "%s", nvsStringValue);
				break;

				case SSI_TYPE_TEXTAREA:
					nvs_get_str(nvsHandle, ssiTag.key, nvsStringValue, &nvsLength);
					nvsStringValue[nvsLength] = '\0';
					sprintf(replaceSSIValue, "<textarea name=\"%s\">%s</textarea>", ssiTag.key, nvsStringValue);
				break;

				case SSI_TYPE_PASSWORD:
					nvs_get_str(nvsHandle, ssiTag.key, nvsStringValue, &nvsLength);
					nvsStringValue[nvsLength] = '\0';
					sprintf(replaceSSIValue, "<input type=\"password\" name=\"%s\" value=\"%s\" />", ssiTag.key, nvsStringValue);
				break;

				case SSI_TYPE_INTEGER:
					nvs_get_u32(nvsHandle, ssiTag.key, &nvsIntValue);
					itoa(nvsIntValue, nvsStringValue, 10);
					sprintf(replaceSSIValue, "<input type=\"number\" name=\"%s\" value=\"%s\" />", ssiTag.key, nvsStringValue);
				break;

				case SSI_TYPE_CHECKBOX:

					httpGetBoolTagKey(ssiKeyTrim, ssiTag.key);
					rotate = httpGetBoolTagRotate(ssiTag.key);

					nvs_get_u8(nvsHandle, ssiKeyTrim, &nvsCharValue);

					nvsCharValue = (nvsCharValue >> rotate) & 0x01;

					strcpy(nvsStringValue, nvsCharValue ? "checked" : "");

					sprintf(replaceSSIValue, "\
							<input type=\"hidden\" id=\"_%s\" name=\"%s\" value=\"%d\">\
							<input type=\"checkbox\" id=\"%s\" onchange=\"document.getElementById('_%s').value = this.checked?1:0;\" %s>\
						",
						ssiTag.key, ssiTag.key, nvsCharValue,
						ssiTag.key, ssiTag.key, nvsStringValue
					);
				break;
			}

			break;
		}
		// strcpy(replaceSSIValue, "TEST");

		appendLength = strlen(replaceSSIValue);
		strncpy(out, replaceSSIValue, appendLength);
		out+= appendLength;
	}

	nvs_close(nvsHandle);
}

static esp_err_t httpGetPost(httpd_req_t *req, char * postString, unsigned int postStringLength){

	int ret, remaining = req->content_len;

    while (remaining > 0) {
        /* Read the data for the request */
        if ((ret = httpd_req_recv(req, postString, MIN(remaining, postStringLength))) < 0) {
            return ESP_FAIL;
        }

        remaining -= ret;
    }

    postString[req->content_len] = '\0';

    return ESP_OK;
}

static void httpServerSavePost(httpd_req_t * req, char * buffer, unsigned int bufferLength, const ssiTag_t * ssiTags, int ssiTagsLength){

	ESP_ERROR_CHECK(httpGetPost(req, buffer, bufferLength));

	printf("Got post data :%s\n", buffer);

	tokens_t post;
	httpServerParseValues(&post, buffer, "&", "=", "\0");

	nvs_handle nvsHandle;
	ESP_ERROR_CHECK(nvs_open("BeelineNVS", NVS_READWRITE, &nvsHandle));

    for (int ssiTagsIndex = 0; ssiTagsIndex < ssiTagsLength; ssiTagsIndex++) {

		const ssiTag_t ssiTag = ssiTags[ssiTagsIndex];

		char * value = httpServerGetTokenValue(&post, ssiTag.key);

		if (!value){
			continue;
		}

		char ssiKeyTrim[MAX_HTTP_SSI_KEY_LENGTH] = {"\0"};
		int rotate = 0;
		unsigned char nvsCharValue = 0;

		switch (ssiTag.type) {

			case SSI_TYPE_DISAPLY_TEXT:
				// Display values should never save
			break;

			case SSI_TYPE_TEXT:
			case SSI_TYPE_PASSWORD:
			case SSI_TYPE_TEXTAREA:
				ESP_ERROR_CHECK(nvs_set_str(nvsHandle, ssiTag.key, value));
			break;

			case SSI_TYPE_INTEGER:
				ESP_ERROR_CHECK(nvs_set_u32(nvsHandle, ssiTag.key, atoi(value)));
			break;

			case SSI_TYPE_CHECKBOX:

				httpGetBoolTagKey(ssiKeyTrim, ssiTag.key);
				rotate = httpGetBoolTagRotate(ssiTag.key);

				nvs_get_u8(nvsHandle, ssiKeyTrim, &nvsCharValue);

				if (atoi(value)) {
					nvsCharValue|= (0x01 << rotate);
				}
				else{
					nvsCharValue&= ~(0x01 << rotate);
				}

				ESP_ERROR_CHECK(nvs_set_u8(nvsHandle, ssiKeyTrim, nvsCharValue));
				ESP_ERROR_CHECK(nvs_commit(nvsHandle));
			break;
		}


		ESP_ERROR_CHECK(nvs_commit(nvsHandle));
	}

   	nvs_close(nvsHandle);
}

esp_err_t httpRespond(httpd_req_t *req, const char * fileStart, const char * fileEnd, const ssiTag_t * ssiTags, int ssiTagsLength) {

	wifi_used();

	char outBuffer[4096];

	if (req->method == HTTP_POST){
		httpServerSavePost(req, outBuffer, sizeof(outBuffer), ssiTags, ssiTagsLength);
	}


	if (ssiTags) {
		httpReaplceSSI(outBuffer, fileStart, fileEnd, ssiTags, ssiTagsLength);
	}

	else{
		strncpy(outBuffer, fileStart, fileEnd - fileStart);
	}


	return httpd_resp_send(req, outBuffer, strlen(outBuffer));
}

void stop_webserver(httpd_handle_t server) {

	// Stop the httpd server
    ESP_LOGI(TAG, "Stopping httpd server");

    // Stop the httpd server
    httpd_stop(server);
}


static httpd_handle_t start_webserver(void) {

	wifi_used();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    
	config.stack_size = 8192;
    config.max_uri_handlers = 32;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting httpd server on port: %d", config.server_port);

    httpd_handle_t server;
    if (httpd_start(&server, &config) == ESP_OK) 
	{    
		// Set URI handlers
        ESP_LOGI(TAG, "http: Registering URI handlers");

		httpPageConfigResetHTMLInit(server);
        httpPageConfigDeviceHTMLInit(server);
        httpPageConfigDieSensorsHTMLInit(server);
        httpPageConfigDisplayHTMLInit(server);
        httpPageConfigElasticsearchHTMLInit(server);
        httpPageConfigLoRaHTMLInit(server);
        httpPageConfigMQTTHTMLInit(server);
        httpPageConfigNTPHTMLInit(server);
        httpPageConfigWiFiHTMLInit(server);
        httpPageIndexHTMLInit(server);
        httpPageJavascriptJSInit(server);
        httpPageMenuCSSInit(server);
        httpPageMenuHTMLInit(server);
        httpPageStyleCSSInit(server);

        return server;
    }

    ESP_LOGE(TAG, "http: / Error starting server!");
    return NULL;
}

static void httpServerTask(void *arg){

	EventBits_t EventBits;
	httpd_handle_t server = NULL;

	while (true){

		vTaskDelay(4000 / portTICK_RATE_MS);

		EventBits = xEventGroupWaitBits(wifiGetEventGroup(), WIFI_CONNECTED_BIT, false, true, 4000 / portTICK_RATE_MS);

		if (WIFI_CONNECTED_BIT & EventBits) {

			/* Start the web server */
	        if (server == NULL) {
	            server = start_webserver();
	        }
		}

		else{
			/* Stop the webserver */
	        if (server) {
	            stop_webserver(server);
	            server = NULL;
	        }
		}
	}

	vTaskDelete(NULL);
    return;
}

void httpd_init()
{
	BaseType_t taskResult = xTaskCreate(&httpServerTask, "http", 8192, NULL, 12, NULL);
	if (taskResult != pdTRUE) {
		assert(pdFAIL);
	}
}