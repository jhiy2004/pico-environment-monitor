#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdio.h>

typedef struct log_info {
    int8_t temperature;
    int8_t umidity;
    int8_t hour;
    int8_t minute;
    int8_t day;
    int8_t month;
    int16_t year;
} log_info;

void buildLogHeader(char* header);
void buildLogMessage(char* line, log_info* info);

void LogInit();
void LogInfo(log_info* info);

#endif