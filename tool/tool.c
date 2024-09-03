#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "tool.h"


int get_current_timestamp() {
    // 获取当前时间
    time_t current_time = time(NULL);
    
    // 将当前时间（time_t类型）转换为时间戳（int类型）
    return (int)current_time;
}

void convert_timestamp(int timestamp, char* output_str) {
    // Convert the int timestamp to a time_t variable
    //printf("timestamp = %d\n", timestamp);
    time_t raw_time = (time_t)timestamp;

    // Convert the time_t variable to a struct tm for local time
    struct tm *time_info = localtime(&raw_time);

    // Format the time into a string
    strftime(output_str, 100, "%Y-%m-%d %H:%M:%S", time_info);
}