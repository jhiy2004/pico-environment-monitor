#ifndef WIFI_H
#define WIFI_H

#include "WifiConfig.h"

#include <stdbool.h>

bool InitWifi(wifi_config* config);
bool WebPoll();

#endif