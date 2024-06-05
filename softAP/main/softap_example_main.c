/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include "esp_netif.h"

#include "driver/gpio.h"


#include "driver/uart.h"
#include "string.h"

static const int RX_BUF_SIZE = 1024;

#define LED			GPIO_NUM_27
#define DIRECTION	GPIO_NUM_26

#define TXD_PIN 	(GPIO_NUM_17)
#define RXD_PIN 	(GPIO_NUM_16)

#define UART		UART_NUM_2


/* The examples use WiFi configuration that you can set via project configuration menu.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

static const char *TAG = "webserver";

int sendData(const char* logName, const char* data)
{
    const int len = strlen(data);
    const int txBytes = uart_write_bytes(UART, data, len);
    ESP_LOGI(logName, "Wrote %d bytes", txBytes);
    return txBytes;
}


/* An HTTP GET handler */
static esp_err_t LD_OFF_handler(httpd_req_t *req)
{
    esp_err_t error;
    ESP_LOGI(TAG, "LED_turned_OFF!");
    gpio_set_level(LED, 0);
    const char *response = (const char *)req->user_ctx;
    error = httpd_resp_send(req, response, strlen(response));
    if(error != ESP_OK)
    	ESP_LOGI(TAG, "Error %d while sending Response", error);
    else
    	ESP_LOGI(TAG, "Response sent successfully");
    return error;
}

//static esp_err_t LD_ON_handler(httpd_req_t *req)
//{
//    esp_err_t error;
//    ESP_LOGI(TAG, "LED_turned_ON!");
//    gpio_set_level(LED, 1);
//    gpio_set_level(DIRECTION, 1);
//
//    const char *response = (const char *)req->user_ctx;
//    error = httpd_resp_send(req, response, strlen(response));
//    if(error != ESP_OK)
//    	ESP_LOGI(TAG, "Error %d while sending Response", error);
//    else
//    	ESP_LOGI(TAG, "Response sent successfully");
//    return error;
//}
esp_err_t LD_ON_handler(httpd_req_t *req) {

	ESP_LOGI(TAG, "LED_turned_ON!");
	static uint8_t direct = 0;
	static uint8_t enb_drv = 0;

	if(!enb_drv)
		gpio_set_level(LED, 1);

	if(!direct){
		direct = 1;
		gpio_set_level(DIRECTION, direct);
		ESP_LOGI(TAG, "FARWARD");
	}else{
		direct = 0;
		gpio_set_level(DIRECTION, direct);
		ESP_LOGI(TAG, "REVERSE");
	}

//    char direction[20];
//    /* Check if URL contains the direction parameter */
//    if (httpd_query_key_value(req->uri, "direction", direction, sizeof(direction)) == ESP_OK) {
//        /* If the direction parameter exists, use it */
//    } else {
//        /* Default direction */
//        strcpy(direction, "forward");
//    }
//
//    const char *status = strcmp(direction, "reverse") == 0 ? "Против часовой" : "По часовой";
	// Установка текста направления в зависимости от состояния direct
	    const char *status = direct == 1 ? "Против часовой" : "По часовой";
	    const char *next_direction = direct == 1 ? "forward" : "reverse";

    const char *html_response = "<!DOCTYPE html>\
<html>\
<head>\
<meta charset=\"utf-8\">\
<title>Ввод цифр 0-10000</title>\
<style>\
.button {\
  border: none;\
  color: white;\
  padding: 15px 32px;\
  text-align: center;\
  text-decoration: none;\
  display: inline-block;\
  font-size: 16px;\
  margin: 4px 2px;\
  cursor: pointer;\
}\
\
\
    .slider-container {\
    display: flex;\
    flex-direction: column;\
    align-items: center;\
    justify-content: center;\
    height: 300px;\
}\
\
\
    .slider-label {\
    font-size: 20px;\
    margin-bottom: 20px;\
}\
\
\
    input[type=range].vertical {\
    writing-mode: bt-lr;\
    -webkit-appearance: slider-vertical;\
    width: 20px;\
    height: 200px;\
    padding: 0 5px;\
}\
.button1 {background-color: #000000;}\
.button2 {background-color: #4CAF50;} \
</style>\
</head>\
<body>\
\
<h1>Дипломная работа: Разработка системы управления бесконтактным двигателем постоянного тока</h1>\
<p>Включить/выключить двигатель BLDC : </p>\
<h3> Статус двигателя: Включён </h3>\
\
<button class=\"button button1\" onclick= \"window.location.href='/LD_OFF'\">Выключить двигатель</button>\
<h3> Направление вала: %s </h3>\
<button class=\"button button2\" onclick=\"window.location.href='/LD_ON?direction=%s'\">Изменить направление</button>\
\
<div class=\"slider-container\">\
    <label for=\"number-slider\" class=\"slider-label\">Задать частоту (от 1 до 20) кГц:</label>\
    <form action=\"\" method=\"post\">\
    <input type=\"range\" id=\"number-slider\" name=\"number\" list=\"tickmarks\" min=\"890\" max=\"3560\" step=\"890\" value=\"890\" class=\"vertical\">\
    <datalist id=\"tickmarks\">\
        <option value=\"890\">\
        <option value=\"1780\">\
        <option value=\"2670\">\
        <option value=\"3560\">\
    </datalist>\
    <output for=\"number-slider\" id=\"slider-value\">1кГц</output>\
    <button type=\"submit\">Отправить</button>\
</div>\
</body>\
</html>";

    char response[2048];
       snprintf(response, sizeof(response), html_response, status, next_direction);
       httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

//static esp_err_t farward_handler(httpd_req_t *req)
//{
//    esp_err_t error;
//    ESP_LOGI(TAG, "Changed direction is forward");
//
//    gpio_set_level(DIRECTION, 1);
//
//    const char *response = (const char *)req->user_ctx;
//    error = httpd_resp_send(req, response, strlen(response));
//    if(error != ESP_OK)
//    	ESP_LOGI(TAG, "Error %d while sending Response", error);
//    else
//    	ESP_LOGI(TAG, "Response sent successfully");
//    return error;
//}

static esp_err_t reverse_handler(httpd_req_t *req)
{
    esp_err_t error;
    ESP_LOGI(TAG, "Changed direction is reverese");

    gpio_set_level(DIRECTION, 0);
    const char *response = (const char *)req->user_ctx;
    error = httpd_resp_send(req, response, strlen(response));
    if(error != ESP_OK)
    	ESP_LOGI(TAG, "Error %d while sending Response", error);
    else
    	ESP_LOGI(TAG, "Response sent successfully");
    return error;
}


esp_err_t submit_post_handler(httpd_req_t *req) {

	ESP_LOGI(TAG,"handler_get_data called");
    char content[100];

    size_t recv_size = MIN(req->content_len, sizeof(content));
    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
		if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
			httpd_resp_send_408(req);
		}
		return ESP_FAIL;
	}

    //Ищем символ равенства + смещаемся на +1. Указатель теперь за равеннством
    char* value_start = strchr(content, '=') + 1;

    int number = atoi(value_start);

//    //Вычитсаем из общих данных слово "number = "
//    int value_length = strlen(content);// - (value_start - content);
//
//    //Создаю массив символов с размерностью 1-4 символов
//    char value[value_length + 1];
//    //char* z_pnt = '0';
//
//    //Копирую содержимое в массив
//    strncpy(value, value_start, value_length);
//
//    //в конце символ "новой строки"
//    value[value_length] = '\0';
    // Создаем буфер для хранения строки
    char buffer[20]; // Допустим, что числа могут быть до 10 символов

    // Преобразуем число в строку
    int length = snprintf(buffer, sizeof(buffer), "%d", number);

    // Отправляем строку по UART
    uart_write_bytes(UART, buffer, length);
//    uart_write_bytes(UART, value, strlen(value));
    //httpd_resp_send(req, "Data received successfully", HTTPD_RESP_USE_STRLEN);
    return ESP_OK;

}

static const httpd_uri_t submit = {
    .uri       = "/LD_ON",
    .method    = HTTP_POST,
    .handler   = submit_post_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t submit_reverse = {
    .uri       = "/LD_ON_REVERSE",
    .method    = HTTP_POST,
    .handler   = submit_post_handler,
    .user_ctx  = NULL
};

static const httpd_uri_t dir_reverse = {
    .uri       = "/LD_ON_REVERESE",
    .method    = HTTP_GET,
    .handler   = reverse_handler,
	.user_ctx  = "<!DOCTYPE html>\
		<html>\
		<head>\
		<meta charset=\"utf-8\">\
		<title>Ввод цифр 0-10000</title>\
		<style>\
		.button {\
		  border: none;\
		  color: white;\
		  padding: 15px 32px;\
		  text-align: center;\
		  text-decoration: none;\
		  display: inline-block;\
		  font-size: 16px;\
		  margin: 4px 2px;\
		  cursor: pointer;\
		}\
		\
		\
			.slider-container {\
			display: flex;\
			flex-direction: column;\
			align-items: center;\
			justify-content: center;\
			height: 300px;\
		}\
		\
		\
		    .slider-label {\
		    font-size: 20px;\
		    margin-bottom: 20px;\
		}\
		\
		\
		    input[type=range].vertical {\
		    writing-mode: bt-lr;\
		    -webkit-appearance: slider-vertical;\
		    width: 20px;\
		    height: 200px;\
		    padding: 0 5px;\
		}\
		.button1 {background-color: #000000;}\
		.button2 {background-color: #4CAF50;} \
		</style>\
		</head>\
		<body>\
		\
		<h1>Дипломная работа: Разработка системы управления бесконтактным двигателем постоянного тока</h1>\
		<p>Включить/выключить двигатель BLDC : </p>\
		<h3> Статус двигателя: Включён </h3>\
		\
		<button class=\"button button1\" onclick= \"window.location.href='/LD_OFF'\">Выключить двигатель</button>\
		<h3> Направление вала: Против часовой </h3>\
		<button class=\"button button2\" onclick=\"window.location.href='/LD_ON'\">Изменить направление</button>\
		\
\
<div class=\"slider-container\">\
    <label for=\"number-slider\" class=\"slider-label\">Задать частоту (от 1 до 20) кГц:</label>\
    <form action=\"\" method=\"post\">\
    <input type=\"range\" id=\"number-slider\" name=\"number\" list=\"tickmarks\" min=\"890\" max=\"3560\" step=\"890\" value=\"890\" class=\"vertical\">\
    <datalist id=\"tickmarks\">\
        <option value=\"890\">\
        <option value=\"890\">\
        <option value=\"1780\">\
        <option value=\"2670\">\
        <option value=\"3560\">\
    </datalist>\
    <output for=\"number-slider\" id=\"slider-value\">1кГц</output>\
    <button type=\"submit\">Отправить</button>\
</div>\
</body>\
</html>"
};




//static const httpd_uri_t dir_farward = {
//    .uri       = "/LD_ON_FARWARD",
//    .method    = HTTP_GET,
//    .handler   = farward_handler,
//	.user_ctx  = NULL
//};

static const httpd_uri_t LD_OFF = {
    .uri       = "/LD_OFF",
    .method    = HTTP_GET,
    .handler   = LD_OFF_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = "<!DOCTYPE html>\
<html>\
<head>\
<meta charset=\"utf-8\">\
<style>\
.button {\
  border: none;\
  color: white;\
  padding: 15px 32px;\
  text-align: center;\
  text-decoration: none;\
  display: inline-block;\
  font-size: 16px;\
  margin: 4px 2px;\
  cursor: pointer;\
}\
\
.button1 {background-color: #04AA6D;} /* Green */\
</style>\
</head>\
<body>\
\
<h1>Дипломная работа: Разработка системы управления бесконтактным двигателем постоянного тока</h1>\
<p>Включить/выключить двигатель BLDC : </p>\
<h3> Статус двигателя: Выключен </h3>\
\
<button class=\"button button1\" onclick= \"window.location.href='/LD_ON'\">Включить двигатель</button>\
\
</body>\
</html>"
};











static const httpd_uri_t LD_ON = {
    .uri       = "/LD_ON",
    .method    = HTTP_GET,
    .handler   = LD_ON_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx  = NULL
};


esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}


static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &LD_OFF);

        httpd_register_uri_handler(server, &LD_ON);
        httpd_register_uri_handler(server, &submit);

        httpd_register_uri_handler(server, &dir_reverse);
        httpd_register_uri_handler(server, &submit_reverse);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

static void config_LED(void){

	gpio_reset_pin(LED);
	gpio_reset_pin(DIRECTION);

	gpio_set_direction (LED, GPIO_MODE_OUTPUT);
	gpio_set_direction (DIRECTION, GPIO_MODE_OUTPUT);

}
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
#ifdef CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
#else /* CONFIG_ESP_WIFI_SOFTAP_SAE_SUPPORT */
            .authmode = WIFI_AUTH_WPA2_PSK,
#endif
            .pmf_cfg = {
                    .required = true,
            },
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

/****************************************************UART****************************************************/
void uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_driver_install(UART, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART, &uart_config);
    uart_set_pin(UART, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}



static void tx_task(void *arg)
{
    static const char *TX_TASK_TAG = "TX_TASK";
    esp_log_level_set(TX_TASK_TAG, ESP_LOG_INFO);
    while (1) {
        sendData(TX_TASK_TAG, "Hello world");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    while (1) {
        const int rxBytes = uart_read_bytes(UART, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        if (rxBytes > 0) {
            data[rxBytes] = 0;
            ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%s'", rxBytes, data);
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
        }
    }
    free(data);
}

void app_main(void)
{
	static httpd_handle_t server = NULL;

	uart_init();
	config_LED();

//Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  ESP_ERROR_CHECK(nvs_flash_erase());
	  ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
	wifi_init_softap();

	ESP_ERROR_CHECK(esp_netif_init());

	ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
//    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
	//while(1){}
//    xTaskCreate(rx_task, "uart_rx_task", 1024*2, NULL, configMAX_PRIORITIES, NULL);
//    xTaskCreate(tx_task, "uart_tx_task", 1024*2, NULL, configMAX_PRIORITIES-1, NULL);
}
