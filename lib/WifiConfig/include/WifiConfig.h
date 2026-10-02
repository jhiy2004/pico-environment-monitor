#ifndef WIFI_CONFIG_H
#define WIFI_CONFIG_H

typedef struct wifi_config {
    char ssid[100];
    char password[100];
    char ip[16];
} wifi_config;

void GetWifiConfig(wifi_config* config);

#endif