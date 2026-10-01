#include "Wifi.h"
#include "config.h"

#include "pico/stdlib.h"

#include <stdio.h>
#include <string.h>

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
        getBlock(buf, len);
        if (strstr(buf, target)) return i;
    }

    return -1;
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

bool WebPoll() {
    const int len = 1024;

    uint8_t buf[len]; 
    char temp[256];
    char id[10];

    char page[1024];
    sprintf(page,
        "<html>"
            "<head>"
                "<title>Application</title>"
            "</head>"
            "<body>"
                "<h1>Sensor</h1>"
                "<h2>Temperature: %.2f</h2>"
                "<h2>Humidity: %.2f</h2>"
            "</body>"
        "</html>",
        38.6f,
        83.3f
    );


    // Pattern receive by ESP-01 when a client send something, in this case is expected that the client
    // send a http request message
    // +IPD,0,n:xxxxxxxxxx
    // If the ESP-01 didn't send a response containing `+IPD`, then no client reached the socket.
    if (getBlocks(buf, len, 1, "+IPD") < 0) return false;

    // This code is used to extract the connection id
    char *b = strstr(buf, "+IPD");
    b += 5;
    strncpy(temp, b, sizeof(temp));
    char *e = strstr(temp, ",");
    int d = e - temp;
    memset(id, '\0', sizeof(id));
    strncpy(id, temp, d);

    // Send web page to the client
    uint8_t command[128];
    int count = snprintf(command, 128, "AT+CIPSEND=%s,%d\r\n", id, strlen(page));
    uart_write_blocking(ESP01_UART, command, count);
    if (getBlocks(buf, len, 10, ">") < 0) return false;

    uart_write_blocking(ESP01_UART, page, strlen(page));
    if (getBlocks(buf, len, 10, "OK") < 0) return false;

    // Close TCP connection with the client
    count = snprintf(command, 128, "AT+CIPCLOSE=%s\r\n", id);
    uart_write_blocking(ESP01_UART, command, count);

    if (getBlocks(buf, len, 10, "OK") < 0) return false;


    return true;
}