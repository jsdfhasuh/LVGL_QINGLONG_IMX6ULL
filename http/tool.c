#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cJSON.h"
#include "http.h"

return_data create_custom_headers(const char *json_string, char *custom_headers,int begin_size) 
{
    // 解析JSON字符串
    int buffer_size = begin_size;
    int written  = 0;
    char *current_position_header = custom_headers;
    char temp[1024];
    size_t custom_headers_size = 1024;
    size_t remaining_size = buffer_size;
    printf("json_string: %s\n", json_string);
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        return (return_data){.data = NULL, .length = 0};
    }

    // 遍历JSON对象中的所有键
    cJSON *current_element = NULL;
    cJSON_ArrayForEach(current_element, json) 
    {
        if (cJSON_IsString(current_element)) 
        {
            // 将键值对添加到自定义请求头中
            written = snprintf(temp,remaining_size, "%s: %s\r\n", current_element->string, current_element->valuestring);
            if (written >= remaining_size)
            {
                int new_size = custom_headers_size + 1024;
                int differ = current_position_header - custom_headers;
                custom_headers_size = custom_headers_size + 1024;
                custom_headers = realloc(custom_headers,new_size);
                remaining_size += 1024;
                current_position_header = custom_headers + differ;
                strncat(current_position_header,temp,written);
                current_position_header = current_position_header + written;
                remaining_size = remaining_size - written;
            }
            else
            {
                strncat(current_position_header,temp,written);
                current_position_header = current_position_header + written;
                remaining_size = remaining_size - written;
            }
        }
    }
    // 释放JSON对象
    cJSON_Delete(json);
    return (return_data){.data = custom_headers, .length = strlen(custom_headers)};
}

return_data create_json_body(const char *json_string, char *body,int begin_size)
{
        // 解析JSON字符串
    int buffer_size = begin_size;
    int written  = 0;
    char *current_position_body = body;
    char temp[1024];
    size_t body_size = 1024;
    size_t remaining_size = buffer_size;
    printf("json_string: %s\n", json_string);
    cJSON *json = cJSON_Parse(json_string);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        return (return_data){.data = NULL, .length = 0};
    }

    // 遍历JSON对象中的所有键
    cJSON *current_element = NULL;
    cJSON_ArrayForEach(current_element, json) 
    {
        if (cJSON_IsString(current_element)) 
        {
            // 将键值对添加到自定义请求头中
            written = snprintf(temp,remaining_size, "%s: %s\r\n", current_element->string, current_element->valuestring);
            if (written >= remaining_size)
            {
                // 扩大body的大小
                int new_size = body_size + 1024;
                int differ = current_position_body - body;
                body_size = body_size + 1024;
                body = realloc(body,new_size);
                remaining_size += 1024;
                current_position_body = body + differ;
                strncat(current_position_body,temp,written);
                current_position_body = current_position_body + written;
                remaining_size = remaining_size - written;
            }
            else
            {
                strncat(current_position_body,temp,written);
                current_position_body = current_position_body + written;
                remaining_size = remaining_size - written;
            }
        }
    }
    // 释放JSON对象
    cJSON_Delete(json);
    return (return_data){.data = body, .length = strlen(body)};
}