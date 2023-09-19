#include <stdio.h>
#include "WiFi.h"

#define WIFI_SSID       "UNLP - GyG"
#define WIFI_PASS       "defiunlp"
#define WIFI_CHANNEL    1
#define MAX_STA_CONN    1

esp_netif_t * wifi_init_softap(void){
    // It is recommended firstable initialize the Wi-Fi module with the default configuration
    esp_netif_t *wifi_object= esp_netif_create_default_wifi_ap();   // Creates default WIFI AP
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();            // Load default WiFi structure
    esp_wifi_init(&cfg);                                            // Initialize WiFi

    wifi_config_t wifi_config = {                   // WiFi structure configuration
        .ap = {
            .ssid = WIFI_SSID,
            .ssid_len = strlen(WIFI_SSID),
            .channel = WIFI_CHANNEL,
            .password = WIFI_PASS,
            .max_connection = MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };

    esp_wifi_set_mode(WIFI_MODE_AP);               // Set WiFi mode to Acces Point  
    esp_wifi_set_config(WIFI_IF_AP, &wifi_config); // Set WiFi structure configuration
    esp_wifi_start();                              // Start WiFi

    return wifi_object;
}

esp_netif_t* wifi_init_sta(void){

    // It is recommended firstable initialize the Wi-Fi module with the default configuration
    esp_netif_t *wifi_object = esp_netif_create_default_wifi_sta(); // Creates default WiFi STA
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();            // Load default WiFi structure
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));                           // Initialize WiFi
    
    nvs_handle_t my_handle;
    nvs_open("wifi",NVS_READWRITE, &my_handle);             // Open the nvs in read/write mode
    size_t required_size;
    
    nvs_get_str(my_handle, "SSID", NULL, &required_size);   // Get the required size, and value of the SSID from NVS
    char *wifi_ssid = malloc(required_size);
    nvs_get_str(my_handle, "SSID", wifi_ssid, &required_size);
    
    nvs_get_str(my_handle, "PSWD", NULL, &required_size);   // Get the required size, and value of the PSWD from NVS
    char *wifi_pswd = malloc(required_size);
    nvs_get_str(my_handle, "PSWD", wifi_pswd, &required_size);

    wifi_config_t wifi_config = {                           // WiFi structure configuration
        .sta = 
        {            
            .pmf_cfg =
            {
                .capable = true,
                .required = false
            },
        },
    };
    if(*wifi_pswd =='\0'){
        wifi_config.sta.threshold.authmode = WIFI_AUTH_OPEN;
        strcpy((char *)wifi_config.sta.password, "");
    }
    else{
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        strcpy((char *)wifi_config.sta.password, wifi_pswd);
    }
    strcpy((char *)wifi_config.sta.ssid, wifi_ssid);
    
    esp_wifi_set_mode(WIFI_MODE_STA);                   // Set WiFi mode to Station  
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);     // Set WiFi structure configuration  
    esp_wifi_start();                                   // Start WiFi
    
    nvs_close(my_handle);
    free(wifi_pswd);
    free(wifi_ssid);
    return wifi_object;
}
