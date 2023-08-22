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
#define BLUE_LED 2

#define WIFI_CONNECTED_BIT BIT0 // WiFi connected bit
#define WIFI_FAIL_BIT      BIT1 // WiFi fail bit

#define MAX_RETRY     20        // Number of retry to connect to WiFi
#define TIME_OUT_WIFI 120       // Seconds before connect with default configuration

// ======================= GLOBAL VARIABLES ============================
bool wifi_ok = false, start_connection = false;
bool parametters = false;
bool loop = false;
int retry_conn = 0, i=0;
size_t required_size;

nvs_handle_t my_handle;
EventGroupHandle_t s_wifi_event_group;
esp_netif_t *sta_object, *ap_object;
// =====================================================================

// String to upper case function
void str_capitol(char* cadena)
{
    for(int i=0; cadena[i]!='\0';i++){
        cadena[i]=toupper((const char)cadena[i]);
    }
    return;
}

/* void wifi_event_handler
* Its parametters are the event base, event ID, and event data.
* WIFI_EVENT: When a WiFi event occurs, the following can happen:
*               1 - WIFI_EVENT_STA_START:           Indicates that the ESP is ready to connect to a WiFi network.
*               2 - WIFI_EVENT_STA_CONNECTED:       Indicates that the ESP is connected to a WiFi network.
*               3 - WIFI_EVENT_STA_DISCONNECTED:    Indicates that the ESP is disconnected from a WiFi network.
*               4 - WIFI_EVENT_AP_START:            Indicates that the ESP is ready to create an AP.
* IP_EVENT: When a IP event occurs, the following can happen:
*               1 - IP_EVENT_STA_GOT_IP:            Indicates that the ESP has obtained an IP address.
*/
void wifi_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data){
    if(event_base == WIFI_EVENT){
        if (event_id == WIFI_EVENT_STA_START){
            vTaskDelay(100/portTICK_PERIOD_MS);
            printf("CONNECTING WIFI...\n");
            esp_err_t wifi = esp_wifi_connect();    // Connecting
            if(wifi!=ESP_OK){
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
                printf("WIFI NOT CONNECTD...\n");
            }
            else{
                printf("WIFI CONNECTED...\n\n");
            }
            
        }
        else if(event_id == WIFI_EVENT_STA_CONNECTED){
            printf("SUCCESSFULLY CONNECTED TO AP!\n");
        }
        else if(event_id == WIFI_EVENT_STA_DISCONNECTED){
            wifi_ok=false;
            if(retry_conn<MAX_RETRY){
                if(loop==true){
                    esp_wifi_connect(); // Trying to reconnect
                    retry_conn++;
                    printf("RETRY CONNECTION N°: %d OF %d\n", retry_conn, MAX_RETRY);
                    for(i=0; i<5;i++){
                        vTaskDelay(1000/portTICK_PERIOD_MS);
                    }
                }
            }
            else{
                xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT); // Flag to reset ESP
                retry_conn=0;
            }
        }
        else if(event_id == WIFI_EVENT_AP_START){
            printf("AP STARTED!\n");
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
        else if(event_id == WIFI_EVENT_AP_STOP){
            printf("AP STOPPED!\n");
            vTaskDelay(1000/portTICK_PERIOD_MS);
        }
    }
    else if(event_base == IP_EVENT){
        if(event_id == IP_EVENT_STA_GOT_IP){
            wifi_ok=true;
            retry_conn=0;
            printf("IP OBTAINED!\n\n");
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }      
    }
}

void app_main(void)
{
    gpio_reset_pin(BLUE_LED);
    gpio_set_direction(BLUE_LED, GPIO_MODE_OUTPUT);

    int time_out=TIME_OUT_WIFI;

    static httpd_handle_t server = NULL;
    s_wifi_event_group = xEventGroupCreate(); // Create event group for wifi events
   
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
        /* Start ESP32 in Acces Point mode */
        ap_object = wifi_init_softap();
        ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
        ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));
        
        /* Start the WebServer and wait for configurations, then, stop it and detach handlers*/
        server = start_webserver();
        while(parametters != true){
            vTaskDelay(1000/portTICK_PERIOD_MS);
            time_out--;
            if(time_out==0){
                time_out=TIME_OUT_WIFI;
                parametters=true;

                nvs_open("wifi",NVS_READWRITE, &my_handle);         // Open the nvs in read/write mode
                
                nvs_get_str(my_handle, "SSID", NULL, &required_size);  // Get the required size, and value of the SSID from NVS
                char *wifi_ssid = malloc(required_size);
                nvs_get_str(my_handle, "SSID", wifi_ssid, &required_size);
                
                nvs_get_str(my_handle, "PSWD", NULL, &required_size);  // Get the required size, and value of the PSWD from NVS
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
        parametters=false;
        time_out = TIME_OUT_WIFI;
        stop_webserver(server);
        
        ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler));
        ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler));
        
        /* Start ESP32 in Station mode*/ 
        sta_object = wifi_init_sta();
        
        if(wifi_ok){    // If WiFi connection is ready, then start mqqt client and prepare topics
            loop = true;
        }
        else{           // Else, go to AP again
            loop=false;
        }

        while(loop){
            if(wifi_ok){
                gpio_set_level(BLUE_LED, 1);
                vTaskDelay(1000/portTICK_PERIOD_MS);
                gpio_set_level(BLUE_LED, 0);
                vTaskDelay(1000/portTICK_PERIOD_MS);
            }
            /* If something is wrong with WiFi, externals handlers set wifi_ok to FALSE, and then wait
            undefinitely until some new WiFi event occurs.
            If WiFi connection is OK, then continue normally. 
            Else, restart the ESP and put in AP mode again. */
            else{
                if(wifi_ok==false){
                    
                    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                        pdTRUE,
                                        pdFALSE,
                                        portMAX_DELAY);
                    if(bits & WIFI_CONNECTED_BIT){
                        wifi_ok = true;
                    }
                    else if(bits & WIFI_FAIL_BIT){
                        loop=false;
                    }
                }
            }
        }  
        esp_wifi_stop();
        esp_netif_destroy_default_wifi(sta_object);
        esp_netif_destroy_default_wifi(ap_object);
    }
}
