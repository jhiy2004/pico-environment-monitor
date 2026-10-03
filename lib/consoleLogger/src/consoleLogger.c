#include "ConsoleLogger.h"
#include "Logger.h"

#include <stdio.h>
#include <stdlib.h>

void LogInit() {
    char header[100];
    buildLogHeader(header);
        
    printf("Init Console Logger\n");
    printf("%s\n", header);    
}

void LogInfo(log_info* info) {
    char line[100];

    buildLogMessage(line, info);
    printf(line);
}