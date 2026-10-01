#include "SDLogger.h"
#include "logger.h"

#include <stdio.h>
#include <string.h>

void logInit() {
    printf("Initialize SD Logger\n");

    FILE* file = fopen(LOG_FILENAME, "r");
    if (file == NULL) {
        char header[100];
        buildLogHeader(header);
        
        file = fopen(LOG_FILENAME, "w");
        
        fputs(header, file);
        fputs("\n", file);

        fclose(file);
    }
}

void logInfo(log_info* info) {
    char line[100];

    buildLogMessage(line, info);
    printf(line);

    FILE* file = fopen(LOG_FILENAME, "r");
    if (file == NULL) {
        printf("The logger must be initialized with logInit()\n");
        return;
    }
    fclose(file);

    file = fopen(LOG_FILENAME, "a");

    fputs(line, file);
    fputs("\n", file);

    fclose(file);
}