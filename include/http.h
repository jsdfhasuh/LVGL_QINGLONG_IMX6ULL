#ifndef __HTTP__
#define __HTTP__
#include "cJSON.h"

struct http_all_data {
    char *host;
    char *path;
    int port;
    int sockfd;
    char *response;
    cJSON *response_json;
    char *body;
    int fisnish_status;
};

struct return_data {
    char *data;
    int length;
};
#define http_data struct http_all_data
#define return_data struct return_data
http_data* http_main(char *host, char *path, int port, char *action, int get,const char *custom_headers,const char *body);
return_data create_custom_headers(const char *json_string, char *custom_headers,int begin_size);
return_data create_json_body(const char *json_string, char *body,int begin_size);
#endif