#include "Wifi.h"
#include "config.h"

#include "pico/stdlib.h"

#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 4096
#define SD_PATH "/sd/"

static char id[10];

static int getBlock(uint8_t buf[], int len) {
    int count = 0;
    while(count < len - 1) {
        if (uart_is_readable_within_us(ESP01_UART, 10000)) {
            buf[count++] = uart_getc(ESP01_UART);
        } else {
            break;
        }
    }
    buf[count] = 0;
    return count;
}

static int getBlocks(uint8_t buf[], int len, int num, char target[]) {
    for (int i=0; i < num; i++) {
        if (uart_is_readable_within_us(ESP01_UART, 1000 * 1000));
        int blkSize = getBlock(buf, len);
        printf("blkSize: %d\n", blkSize);
        if (strstr(buf, target)) return i;
    }

    return -1;
}

static int sendAll(const char* buffer, int length)
{
    char response[BUFFER_SIZE];
    char command[128];

    int count = snprintf(
        command,
        sizeof(command),
        "AT+CIPSEND=%s,%d\r\n",
        id,
        length
    );

    if (count < 0 || count >= sizeof(command))
        return -1;

    uart_write_blocking(
        ESP01_UART,
        command,
        count
    );

    if (getBlocks(response, sizeof(response), 10, ">") < 0)
        return -1;

    uart_write_blocking(
        ESP01_UART,
        buffer,
        length
    );

    return getBlocks(
        response,
        sizeof(response),
        10,
        "OK"
    );
}

static bool reset() {
    uint8_t SendData[] = "AT+RST\r\n";
    uart_write_blocking(ESP01_UART, SendData, 8);

    uint8_t buf[512];
    if (getBlock(buf, 512) > 0) {
        return true;
    }

    return false;
}

bool InitWifi(wifi_config* config) {
    uint8_t buf[512];
    uint8_t command[128];

    uart_init(ESP01_UART, 115200);
    gpio_set_function(ESP01_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(ESP01_UART_RX, GPIO_FUNC_UART);
    uart_set_format(ESP01_UART, 8, 1, UART_PARITY_NONE);

    sleep_ms(100);

    uart_write_blocking(ESP01_UART, "AT\r\n", 4);
    
    int count = getBlock(buf, 512);
    if (count <= 0) {
        return false;
    }

    printf("Read something: %s\n", buf);
    if (strstr(buf, "OK\r\n") == NULL) {
        return false;
    }

    count = snprintf(command, 128, "AT+CWMODE_CUR=%d\r\n", 1);
    uart_write_blocking(ESP01_UART, command, count);

    count = getBlock(buf, 512);
    if (count <= 0) {
        return false;
    }

    printf("Read something: %s\n", buf);
    if (strstr(buf, "OK\r\n") == NULL) {
        return false;
    }

    count = snprintf(command, 128, "AT+CWJAP_CUR=\"%s\",\"%s\"\r\n", config->ssid, config->password);
    uart_write_blocking(ESP01_UART, command, count);
    count = getBlocks(buf, 512, 20, "OK");


    // Query ip received from router
    count = snprintf(command, 128, "AT+CIFSR\r\n");
    uart_write_blocking(ESP01_UART, command, count);
    count = getBlock(buf, 512);

    if (strstr(buf, "OK") == NULL) {
        return false;
    }

    char* start = strstr(buf, "STAIP,");
    start += 7;
    char* end = strstr(start, "\"");
    count = end - start;

    memcpy(config->ip, start, count);
    config->ip[count] = '\0';

    // Configure ESP01 as a web server

    // Change to multiple connection mode
    uart_write_blocking(ESP01_UART, "AT+CIPMUX=1\r\n", 13);
    if (getBlocks(buf, 512, 10, "OK") < 0) return false;

    // Start a listening socket at port 80
    uart_write_blocking(ESP01_UART, "AT+CIPSERVER=1,80\r\n", 19);
    if (getBlocks(buf, 512, 10, "OK") < 0) return false;

    return true;
}

int getFileHttpHeader(char* filename, long size, char* header, int header_length) {
    char* ext = strstr(filename, ".");
    char* contentType;
    if (strcmp(ext, ".html") == 0) {
        contentType = "text/html";
    } else if(strcmp(ext, ".js") == 0) {
        contentType = "text/javascript";
    } else if(strcmp(ext, ".css") == 0) {
        contentType = "text/css";
    } else {
        return -1;
    }

    return snprintf(
        header,
        header_length,

        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %ld\r\n"
        "Connection: close\r\n"
        "\r\n",

        contentType,
        size
    );
}

int getFileBlock(FILE *file, char* block, int length, int idx) {
    if (file == NULL || block == NULL || length <= 0 || idx < 0) {
        return -1;
    }

    long offset = (long)length * idx;

    if (fseek(file, offset, SEEK_SET) != 0) {
        return -1;
    }

    return (int)fread(block, 1, length, file);
}

void handleRequestFile(char* filename) {
    char buffer[BUFFER_SIZE];
    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        printf("File isn't opened");

        if (strstr(filename, "index.html") != NULL) {
            char page[BUFFER_SIZE];
            int count = snprintf(
                page,
                BUFFER_SIZE,
                "<html>"
                    "<head>"
                        "<title>Application</title>"
                    "</head>"
                    "<body>"
                        "<span>Temperature: </span>"
                        "<span id=\"temperature\"></span>"

                        "<span>Humidity: </span>"
                        "<span id=\"humidity\"></span>"
                        "<script src=\"index.js\"></script>"
                    "</body>"
                "</html>"
            );

            char header[512];
            int header_length = getFileHttpHeader(filename, count, header, 512);

            sendAll(header, header_length);
            sendAll(page, count);

            printf("Sent index.html\n");
        } else if(strstr(filename, "index.js") != NULL) {
            char page[BUFFER_SIZE];
            int count = snprintf(
                page,
                BUFFER_SIZE,
                "async function updateSensors() {"
                    "try {"
                        "const response = await fetch(\"/sensors\");"
                        "const data = await response.json();"

                        "document.getElementById(\"temperature\").textContent ="
                            "data.temperature.toFixed(2);"

                        "document.getElementById(\"humidity\").textContent ="
                            "data.humidity.toFixed(2);"

                    "} catch (error) {"
                        "console.log(\"Failed to fetch sensors data:\", error);"
                    "}"
                "}"

                "setInterval(updateSensors, 10000);"
                "updateSensors();"
            );

            char header[512];
            int header_length = getFileHttpHeader(filename, count, header, 512);

            sendAll(header, header_length);
            sendAll(page, count);

            printf("Sent index.js\n");
        }
        return;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char header[512];
    int header_length = getFileHttpHeader(filename, size, header, 512);

    printf("Size: %d\n", size);

    sendAll(header, header_length);

    int count = 0;
    int block_size = 0;
    while(1) {
        block_size = getFileBlock(file, buffer, BUFFER_SIZE, count++);
        printf("Block size: %d\n", block_size);

        if (block_size <= 0) break;

        printf("Sending block %d\n", count);
        sendAll(buffer, block_size);
    }
    fclose(file);
}

void handleRequestData(temp_humidity_reading* reading) {
    char content[512];
    int content_length = snprintf(
        content,
        512,
        "{"
            "\"temperature\": %.2f,"
            "\"humidity\": %.2f"
        "}",
        reading->temp_celsius,
        reading->humidity
    );
    
    char header[512];
    int header_length = snprintf(
        header,
        512,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n" ,
        content_length
    );

    sendAll(header, header_length);
    sendAll(content, content_length);
}

void handleNotFound() {
    const char *body = "{\"error\":\"Not Found\"}";
    int content_length = strlen(body);

    char header[512];

    int header_length = snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        content_length
    );

    sendAll(header, header_length);
    sendAll(body, content_length);
}

void dispatcher(char* request, temp_humidity_reading* reading) {
    printf("************************************* Dispatcher *************************************\n");
    int count;
    char* sep1 = strstr(request, " ");
    char* sep2 = strstr(sep1 + 1, " ");

    char method[10];
    char resource[50];

    count = sep1 - request;
    memcpy(method, request, count);
    method[count] = '\0';
    
    count = sep2 - (sep1 + 1);
    memcpy(resource, (sep1+1), count);
    resource[count] = '\0';

    printf("Method: %s\n", method);
    printf("Resource: %s\n", resource);

    if (strcmp(method, "GET") == 0) {
        if (strcmp(resource, "/") == 0 || strcmp(resource, "/index.html") == 0) handleRequestFile(SD_PATH "index.html");
        else if(strcmp(resource, "/index.js") == 0) handleRequestFile(SD_PATH "index.js");
        else if(strcmp(resource, "/style.css") == 0) handleRequestFile(SD_PATH "style.css");
        else if(strcmp(resource, "/sensors") == 0) handleRequestData(reading);
        else handleNotFound();
    }
}

bool WebPoll(temp_humidity_reading *reading) {
    char buf[BUFFER_SIZE]; 
    char temp[256];
    char command[128];

    // Pattern receive by ESP-01 when a client send something, in this case is expected that the client
    // send a http request message
    // +IPD,0,n:xxxxxxxxxx
    // If the ESP-01 didn't send a response containing `+IPD`, then no client reached the socket.
    
    bool hit = false;

    while(1) {
        if (getBlocks(buf, BUFFER_SIZE, 1, "+IPD") < 0) return hit;

        hit = true;

        // This code is used to extract the connection id
        char *b = strstr(buf, "+IPD");
        b += 5;
        strncpy(temp, b, sizeof(temp));
        char *e = strstr(temp, ",");
        int d = e - temp;
        memset(id, '\0', sizeof(id));
        strncpy(id, temp, d);

        char *request = strstr(buf, "+IPD");
        request = strstr(request, ":");
        if (request == NULL) return hit;

        request += 1;

        printf("*******************\n");
        printf(buf);
        printf("*******************\n");
        dispatcher(request, reading);
    
        // Close TCP connection with the client
        int count = snprintf(command, 128, "AT+CIPCLOSE=%s\r\n", id);
        uart_write_blocking(ESP01_UART, command, count);

        if (getBlocks(buf, BUFFER_SIZE, 10, "OK") < 0) return hit;
    }    
}