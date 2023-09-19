#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <sys/param.h>
#include <esp_http_server.h>
#include "WiFi.h"
#include "WebServer.h"
#include "stdbool.h"

#include "ctype.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "driver/gpio.h"

// LED
#define BLUE_LED 2              // Built-in LED in ESP32

#define WIFI_CONNECTED_BIT      BIT0 // WiFi connected bit
#define WIFI_DISCONNECTED_BIT   BIT1 // WiFi disconnected bit
#define WIFI_FAIL_BIT           BIT2 // WiFi fail bit

#define MAX_RETRY     20        // Number of retry to connect to WiFi
#define TIME_OUT_WIFI 120       // Seconds before connect with default configuration

// ======================= GLOBAL VARIABLES ============================
bool loop = false; bool parametters = false;


EventGroupHandle_t s_wifi_event_group;
// =====================================================================

void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    static int retry_conn = 0;
    if(event_base == WIFI_EVENT){   // If the event is a WiFi event
        switch (event_id){
            case WIFI_EVENT_STA_START:{         // If the ESP started as STA
                    printf("STARTED AS STA...\n");
                    printf("CONNECTING TO WIFI...\n");
                    esp_err_t wifi = esp_wifi_connect();    // Connecting to WiFi
                    if(wifi!=ESP_OK){
                        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                        printf("WIFI NOT CONNECTED...\n");
                    }
                    break;
                }
            case WIFI_EVENT_STA_CONNECTED:{     // If the ESP is connected to an AP
                printf("SUCCESSFULLY CONNECTED TO AP!\n");
                printf("WAITING FOR IP...\n");
                break;
            }
            case WIFI_EVENT_STA_DISCONNECTED:{  // If the ESP is disconnected from an AP
                printf("DISCONNECTED FROM AP!\n");
                xEventGroupSetBits(s_wifi_event_group, WIFI_DISCONNECTED_BIT);
                
                if(retry_conn<MAX_RETRY){
                    esp_wifi_connect(); // Trying to reconnect
                    retry_conn++;       // Increment retry counter
                    printf("RETRY CONNECTION N°: %d OF %d\n", retry_conn, MAX_RETRY);
                }
                else{
                    printf("MAX RETRY REACHED! RESETING LOOP...\n");
                    xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);  // Flag to reset ESP
                    retry_conn=0;                                           // Reset retry counter
                }
                break;
            }
            case WIFI_EVENT_AP_START:{          // If the ESP started as AP
                printf("STARTED AS AP...\n");
                break;
            }
            case WIFI_EVENT_AP_STOP:{           // If the ESP stopped as AP
                printf("STOPED AS AP...\n");
                break;
            }
            default:{
                break;
            }
        }
    }
    else if(event_base == IP_EVENT){                        // If the event is an IP event
        switch (event_id){
            case IP_EVENT_STA_GOT_IP:{
                xEventGroupClearBits(s_wifi_event_group, WIFI_DISCONNECTED_BIT);
                xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
                printf("IP OBTAINED!\n\n");
                retry_conn=0;       // Reset retry counter
                break;
            }
            case IP_EVENT_STA_LOST_IP:{
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                printf("IP LOST!\n");
                break;
            }
            default:{
                break;
            }
        }
    }
}

void wait_wifi_event(EventGroupHandle_t, bool*, bool*);


void app_main(void)
{
    bool wifi_ok = false; 
    int time_out = TIME_OUT_WIFI;

    static httpd_handle_t server    = NULL;
    s_wifi_event_group              = xEventGroupCreate(); // Create event group for wifi events
   
    gpio_reset_pin(BLUE_LED);
    gpio_set_direction(BLUE_LED, GPIO_MODE_OUTPUT);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    printf("\nNVS INITIALIZED SUCCESSFULLY!\n");

    ESP_ERROR_CHECK(esp_netif_init());
    printf("\nTCP/IP PROTOCOL INITIALIZED SUCCESSFULLY!\n");

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    printf("\nEVENT LOOP CRETEAD SUCCESSFULLY!\n");

    /* Setting the Wi-Fi and IP handler */
    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);

    esp_event_handler_instance_t instance_got_ip;
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

    
    while(true){
        // Start ESP32 in Acces Point mode
        esp_netif_t *ap_object = wifi_init_softap();

        server = start_webserver();                // Start the WebServer for enter parametters
        while(!parametters){
            vTaskDelay(1000/portTICK_PERIOD_MS);
            time_out--;
            if(time_out==0){                    // If time out, then start with default configuration
                time_out=TIME_OUT_WIFI;
                parametters=true;

                nvs_handle_t my_handle;
                nvs_open("wifi",NVS_READWRITE, &my_handle);             // Open the nvs in read/write mode
                size_t required_size;

                nvs_get_str(my_handle, "SSID", NULL, &required_size);   // Get the required size, and value of the SSID from NVS
                char *wifi_ssid = malloc(required_size);
                nvs_get_str(my_handle, "SSID", wifi_ssid, &required_size);
                
                nvs_get_str(my_handle, "PSWD", NULL, &required_size);   // Get the required size, and value of the PSWD from NVS
                char *wifi_pswd = malloc(required_size);
                nvs_get_str(my_handle, "PSWD", wifi_pswd, &required_size);

                printf("STARTING DEFAULT CONNECTION...\n");
                printf("SSID: %s\n", wifi_ssid);
                printf("PSWD: %s\n\n", wifi_pswd);

                nvs_close(my_handle);
                free(wifi_pswd);
                free(wifi_ssid);
            }
        }
        parametters=false;                          // Reset parametters flag
        time_out = TIME_OUT_WIFI;                   // Reset time out counter
        stop_webserver(server);                     // Stop the WebServer
        esp_wifi_stop();                            // Stop WiFi
        esp_netif_destroy_default_wifi(ap_object);  // Destroy default WiFi AP
        
        esp_netif_t *sta_object = wifi_init_sta();  // Start the ESP32 in Station mode

        // Wait indefinitely for wifi either connected or failed
        wait_wifi_event(s_wifi_event_group, &wifi_ok, &loop);

        EventBits_t check_conn;

        while(loop){
            if(wifi_ok){
                gpio_set_level(BLUE_LED, 1);
                vTaskDelay(1000/portTICK_PERIOD_MS);
                gpio_set_level(BLUE_LED, 0);
                vTaskDelay(1000/portTICK_PERIOD_MS);

                check_conn = xEventGroupGetBits(s_wifi_event_group);        // Check if the ESP is connected or disconnected
                if(check_conn & WIFI_DISCONNECTED_BIT){wifi_ok = false;}    // If disconnected, then flag to reconnect
            }
            else{
                printf("WIFI DISCONNETED!\n");
                check_conn = xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdTRUE,
                                                pdFALSE, portMAX_DELAY);    // Wait indefinitely for wifi either connected or failed
                if(check_conn & WIFI_CONNECTED_BIT){wifi_ok = true;}
                else{loop=false;}
            }
        }

        esp_wifi_stop();                            // Stop WiFi
        esp_netif_destroy_default_wifi(sta_object); // Destroy default WiFi STA
    }
}

void wait_wifi_event(EventGroupHandle_t wifi, bool *wifi_ok, bool *loop){
    EventBits_t bits = xEventGroupWaitBits(wifi, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT, pdTRUE, pdFALSE, portMAX_DELAY);
    
    if(bits & WIFI_CONNECTED_BIT){
        *wifi_ok = true; 
        *loop = true;
    }
    else{
        *wifi_ok = false;
        *loop = false;
    }
}


/*
static esp_err_t favicon_get_handler(httpd_req_t *req)
{
	extern const unsigned char favicon_ico_start[] asm("_binary_favicon_ico_start");
	extern const unsigned char favicon_ico_end[] asm("_binary_favicon_ico_end");
	const size_t favicon_ico_size = (favicon_ico_end - favicon_ico_start);
	httpd_resp_set_type(req, "image/x-icon");
	httpd_resp_send(req, (const char *)favicon_ico_start, favicon_ico_size);
	return ESP_OK;
} 
*/