#include "logger.h"

void buildLogHeader(char* header) {
    sprintf(header, "Year,Umidity,Temperature");
}

void buildLogMessage(char* line, log_info* info) {
    sprintf(line, "%04d-%02d-%02dT%02d:%02d,%d,%d", info->year, info->month, info->day, info->hour, info->minute, info->temperature, info->umidity);
}