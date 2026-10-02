#include "Wifi.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include <winsock2.h>
#include <ws2tcpip.h>

#pragma comment(lib, "Ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024
#define SD_PATH "C:\\Users\\jhiy2\\OneDrive\\Desktop\\Projeto Raspberry\\project\\sd-content\\"

static SOCKET server_fd = INVALID_SOCKET;
static SOCKET new_socket = INVALID_SOCKET;


static struct sockaddr_in address;
static int addrlen = sizeof(address);

bool InitWifi(wifi_config* config)
{
    printf(
        "Init Wifi\n"
        "SSID: %s\n"
        "Password: %s\n",
        config->ssid,
        config->password
    );

    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return false;
    }

    // Create socket
    server_fd = socket(
        AF_INET,
        SOCK_STREAM,
        0
    );

    if (server_fd == INVALID_SOCKET) {
        printf(
            "socket failed: %d\n",
            WSAGetLastError()
        );

        WSACleanup();
        return false;
    }

    // Allow address reuse
    int opt = 1;

    if (setsockopt(
            server_fd,
            SOL_SOCKET,
            SO_REUSEADDR,
            (const char *)&opt,
            sizeof(opt)
        ) == SOCKET_ERROR) {

        printf(
            "setsockopt failed: %d\n",
            WSAGetLastError()
        );

        closesocket(server_fd);
        WSACleanup();

        return false;
    }

    // Configure address
    memset(&address, 0, sizeof(address));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind
    if (bind(
            server_fd,
            (struct sockaddr *)&address,
            sizeof(address)
        ) == SOCKET_ERROR) {

        printf(
            "bind failed: %d\n",
            WSAGetLastError()
        );

        closesocket(server_fd);
        WSACleanup();

        return false;
    }

    // Start listening
    if (listen(server_fd, 3) == SOCKET_ERROR) {

        printf(
            "listen failed: %d\n",
            WSAGetLastError()
        );

        closesocket(server_fd);
        WSACleanup();

        return false;
    }

    printf(
        "Server listening on port %d...\n",
        PORT
    );

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

int sendAll(const char* buffer, int length){
    int total = 0;

    while (total < length) {
        int sent = send(new_socket, buffer + total, length - total, 0);

        if (sent == SOCKET_ERROR)
            return -1;

        if (sent == 0)
            return -1;

        total += sent;
    }

    return total;
}

void handleRequestFile(char* filename) {
    char buffer[BUFFER_SIZE];
    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        printf("File isn't opened");
        return;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char header[512];
    int header_length = getFileHttpHeader(filename, size, header, 512);

    printf("Size: %d\n", size);

    int sent = sendAll(header, header_length);

    if (sent == SOCKET_ERROR) {
        printf(
            "send failed: %d\n",
            WSAGetLastError()
        );

        closesocket(new_socket);

        fclose(file);
        return;
    }

    printf("Sent header\n");

    int count = 0;
    int block_size = 0;
    while(1) {
        block_size = getFileBlock(file, buffer, BUFFER_SIZE, count++);
        printf("Block size: %d\n", block_size);

        if (block_size <= 0) break;

        printf("Sending block %d\n", count);
        sent = sendAll(buffer, block_size);
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

    int sent = sendAll(header, header_length);

    if (sent == SOCKET_ERROR) {
        printf(
            "send failed: %d\n",
            WSAGetLastError()
        );

        closesocket(new_socket);

        return;
    }

    sendAll(content, content_length);
}

void handleNotFound() {
    char content[512];
    int content_length = snprintf(
        content,
        512,
        "<html>"
            "<head>"
                "<title>Application</title>"
            "</head>"
            "<body>"
                "<h1>Not found</h1>"
            "</body>"
        "</html>"
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

    int sent = sendAll(header, header_length);

    if (sent == SOCKET_ERROR) {
        printf(
            "send failed: %d\n",
            WSAGetLastError()
        );

        closesocket(new_socket);

        return;
    }

    sendAll(content, content_length);
};

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


bool WebPoll(temp_humidity_reading* reading) {
    if (server_fd == INVALID_SOCKET) {
        printf("Server is not initialized\n");
        return false;
    }

    // Wait for client
    new_socket = accept(
        server_fd,
        (struct sockaddr *)&address,
        &addrlen
    );

    if (new_socket == INVALID_SOCKET) {

        printf(
            "accept failed: %d\n",
            WSAGetLastError()
        );

        return false;
    }

    printf("Client connected\n");

    // Receive HTTP request
    char buffer[BUFFER_SIZE];

    int valread = recv(
        new_socket,
        buffer,
        sizeof(buffer) - 1,
        0
    );

    if (valread <= 0) {
        closesocket(new_socket);
        new_socket = INVALID_SOCKET;
        return false;
    }

    buffer[valread] = '\0';

    printf("***********************************\n");
    printf("%s", buffer);
    printf("***********************************\n");

    dispatcher(buffer, reading);

    closesocket(new_socket);
    new_socket = INVALID_SOCKET;

    closesocket(new_socket);
    new_socket = INVALID_SOCKET;
    return true;
}