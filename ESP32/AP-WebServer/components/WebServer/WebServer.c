#include "WebServer.h"

extern bool parametters;

// Homepage handler
esp_err_t homepage_get_handler(httpd_req_t *req)
{
    esp_err_t error;
    const char *response=(const char*)req->user_ctx;
    error=httpd_resp_send(req, response,strlen(response));
    if(error != ESP_OK){
        printf("Error printing the web page in server mode, error code: %d \n",error);
    }
    printf("Homepage loaded successfully!\n");
    return ESP_OK;
}
const httpd_uri_t homepage = {
    .uri       = "/",
    .method    = HTTP_GET,
    .handler   = homepage_get_handler,
    
    .user_ctx  = "<!DOCTYPE html>\
<html>\
<body>\
	<form action=\"/data\" method=\"GET\">\
      <h1>ESP32 Wi-Fi configuration </h1>\
        <strong>SSID:</strong> <input type=\"text\" name=\"SSID\"><br>\
        <strong>PSWD:</strong> <input type=\"text\" name=\"PSWD\"><br><br> \
        <input type=\"submit\" value=\"Enviar datos\" onclick=\"alert('Datos cargados!')\">\
  </form>\
</body>\
</html>"
};

// Data handler
 esp_err_t data_get_handler(httpd_req_t *req)
{
    char *query, *buff;
    bool ssid=false;
    size_t query_len, buff_len;

    buff_len = 40;
    buff = malloc(++buff_len);
    
    query_len = httpd_req_get_url_query_len(req);
    query = malloc(++query_len);

    httpd_req_get_url_query_str(req, query, query_len);
    query = urlDecode(query);
    
    /* Preparing the NVS */
    nvs_handle_t my_handle;
    nvs_open("wifi",NVS_READWRITE, &my_handle);

    /* Parametters configuration */
    // SSID:
    httpd_query_key_value(query, "SSID", buff, buff_len);
    if(buff[0]=='\0'){
        printf("SSID read: default.\n");
    }
    else{
        nvs_set_str(my_handle,"SSID",buff);
        printf("SSID read: %s\n",buff);
        ssid=true;
    }
    // Password:
    httpd_query_key_value(query, "PSWD", buff, buff_len);
    if(buff[0]=='\0'){
        if(ssid){
            nvs_set_str(my_handle,"PSWD",buff);
            printf("PSWD read: %s\n",buff);
            ssid = false;
        }
        else{
            printf("PSWD read: default.\n");
        }
    }
    else{
        nvs_set_str(my_handle,"PSWD",buff);
        printf("PSWD read: %s\n",buff);
    }
    
    nvs_commit(my_handle);
    nvs_close(my_handle);
    free(query);
    free(buff);
    parametters=true;
    return ESP_OK;
}
const httpd_uri_t data = {
    .uri       = "/data",
    .method    = HTTP_GET,
    .handler   = data_get_handler,
    
    .user_ctx  = NULL
};

// Start WebServer function
httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    printf("Starting server on port: '%d' \n", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        printf("Registering URI handlers...\n");
        httpd_register_uri_handler(server, &homepage);
        httpd_register_uri_handler(server, &data);
        printf("Handlers registered! \n");

        return server;
    }

    printf("Error starting server!\n");
    return NULL;
}

// Stop WebServer function
void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    printf("Stopping the WebServer...\n");
    httpd_stop(server);
}

char *urlDecode(const char *str) {

  char *dStr = (char *) malloc(strlen(str) + 1);
  char eStr[] = "00"; /* for a hex code */

  strcpy(dStr, str);

    for(int i=0;i<strlen(dStr);++i) {

      if(dStr[i] == '%') {
        if(dStr[i+1] == 0)
          return dStr;

        if(isxdigit((int)dStr[i+1]) && isxdigit((int)dStr[i+2])) {

          /* combine the next to numbers into one */
          eStr[0] = dStr[i+1];
          eStr[1] = dStr[i+2];

          /* convert it to decimal */
          long int x = strtol(eStr, NULL, 16);

          /* remove the hex */
          memmove(&dStr[i+1], &dStr[i+3], strlen(&dStr[i+3])+1);

          dStr[i] = x;
        }
      }
      else if(dStr[i] == '+') { dStr[i] = ' '; }
    }
  return dStr;
}