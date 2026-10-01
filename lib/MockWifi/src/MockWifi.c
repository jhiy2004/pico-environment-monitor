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

static SOCKET server_fd = INVALID_SOCKET;

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

void handleRequestFile(char* filename) {
    char buffer[BUFFER_SIZE]
    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        printf("File isn't opened");
        return false;
    }

    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char* ext = strstr(filename, ".");
    char* contentType;
    if (strcmp(ext == ".html") == 0) {
        contentType = "text/html";
    } else if(strcmp(ext == ".js") == 0) {
        contentType = "text/javascript";
    } else if(strcmp(ext == ".css") == 0) {
        contentType = "text/css";
    } else {
        return false;
    }
    
    char header[512];
    int header_length = snprintf(
        header,
        sizeof(header),

        "HTTP/1.1 200 OK\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "\r\n"

        contentType,
        length
    );

    int sent = send(
        new_socket,
        header,
        header_length,
        0
    );

    if (sent == SOCKET_ERROR) {
        printf(
            "send failed: %d\n",
            WSAGetLastError()
        );

        closesocket(new_socket);

        return false;
    }

    while(1) {
        int count = fread(buffer, sizeof(char), BUFFER_SIZE, file);
        if (count == 0) break;
        
        sent = send(new_socket, buffer, count, 0);
    }
}
void handleRequestData();
void handleNotFound();

void dispatcher(char* request) {
    int count;
    char* sep1 = strstr(request, " ");
    char* sep2 = strstr(request + 1, " ");

    char method[10];
    char resource[50];

    count = sep1 - start;
    memcpy(method, request, count);
    method[count] = '\0';
    
    count = sep2 - (sep1 + 2)
    memcpy(resource, request, count);
    method[count] = '\0';

    if (strcmp(method, "GET") == 0) {
        if (strcmp(resource, "/") == 0 || strcmp(resource, "/index.html") == 0) handleRequestFile("index.html");
        else if(strcmp(resource, "/index.js") == 0) handleRequestFile("index.js");
        else if(strcmp(resource, "/style.css") == 0) handleRequestFile("style.css");
        else if(strcmp(resource, "/sensors") == 0) handleRequestData();
        else {

        }
    }
}


bool WebPoll() {
    if (server_fd == INVALID_SOCKET) {
        printf("Server is not initialized\n");
        return false;
    }

    SOCKET new_socket;
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

    if (valread == SOCKET_ERROR) {

        printf(
            "recv failed: %d\n",
            WSAGetLastError()
        );

        closesocket(new_socket);

        return false;
    }

    if (valread == 0) {
        printf("Client disconnected\n");

        closesocket(new_socket);

        return false;
    }

    buffer[valread] = '\0';

    printf(
        "Received:\n%s\n",
        buffer
    );

    // Send HTTP response
    char response[4096];

    int response_length = snprintf(
        response,
        sizeof(response),

        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: %zu\r\n"
        "Connection: close\r\n"
        "\r\n"
        "%s",

        strlen(page),
        page
    );

    if (response_length < 0 ||
        response_length >= sizeof(response)) {

        printf("Response too large\n");

        closesocket(new_socket);

        return false;
    }

    int sent = send(
        new_socket,
        response,
        response_length,
        0
    );

    if (sent == SOCKET_ERROR) {

        printf(
            "send failed: %d\n",
            WSAGetLastError()
        );

        closesocket(new_socket);

        return false;
    }

    printf(
        "Sent %d bytes\n",
        sent
    );

    closesocket(new_socket);

    return true;
}