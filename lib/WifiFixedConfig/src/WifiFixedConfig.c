#include "WifiConfig.h"

#include <stdio.h>
#include <string.h>

void GetWifiConfig(wifi_config* config) {
    strcpy(config->ssid, "Rosa");
    strcpy(config->password, "rosa2508");
    memset(config->ip, 0, 16);
}