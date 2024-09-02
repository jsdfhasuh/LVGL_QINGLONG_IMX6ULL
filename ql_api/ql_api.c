#include "cJSON.h"
#include "http.h"
#include <stdio.h>
#include <stdlib.h>
#include "debug.h"
#include <string.h>
#include "ql_api.h"


char * global_url;
char * global_token;

return_data ql_login(char *url,const char *client_id,const char *client_sercret)
{
    char * path = (char *)calloc(2048,sizeof(char));
    cJSON * temp_json;
    return_data return_data_ql_login;
    char * token;
    int error_num;
    #if DEBUG
    //'http://192.168.1.165:5700/open/auth/token?client_id=Vt_tztUiE4oO&client_secret=iUQNhsZt7f___a9emb67c4_3'
    printf("url :%s\n",url);
    printf("client_id :%s\n",client_id);
    printf("client_sercret :%s\n",client_sercret);
    #endif
    error_num = snprintf(path,1024,"/open/auth/token?client_id=%s&client_secret=%s",client_id,client_sercret);
    if (error_num < 0)
    {
        printf("path error\n");
        return (return_data){.data = NULL, .length = 0};
    }
    # if DEBUG
    printf("path :%s\n",path);
    # endif
    http_data* data = http_main(url, path, 5700, "GET", 1,"Connection: keep-alive\n\r","");
    printf("response :%s\n",data->response);
    temp_json = cJSON_GetObjectItem(data->response_json, "data");
    if (temp_json == NULL)
    {
        printf("data is null\n");
        return (return_data){.data = NULL, .length = 0};
    }
    free(path);
    token = cJSON_GetObjectItem(temp_json, "token")->valuestring;
    return_data_ql_login = (return_data){.data = token, .length = strlen(token)};
    global_token = token;
    global_url = url;
    return return_data_ql_login;
}


http_data* get_env(char *url,const char *token,const char *name)
{
    http_data * data;
    char * headers;
    int error_num;
    char *path = "/open/envs";
    if (url == NULL && token == NULL)
    {
        url = global_url;
        token = global_token;
    }
    #if DEBUG
    printf("url :%s\n",url);
    printf("token :%s\n",token);
    printf("name :%s\n",name);
    #endif
    headers = (char *)calloc(2048,sizeof(char));
    error_num = snprintf(headers,2048,"Authorization: Bearer %s\r\naccept: application/json\r\nsearchValue: %s\r\nConnection: keep-alive\n\r",token,name);
    if (error_num < 0)
    {
        printf("headers error\n");
        data = NULL;
    }
    #if DEBUG
    printf("headers :\n%s\n",headers);
    #endif
    data = http_main(url, path, 5700, "GET", 1,headers,"");
    printf("body:\n%s\n",data->body);
    return data;
}

http_data* get_crons(char *url,const char *token)
{
    char * headers;
    int error_num;
    char *path = "/open/crons";
    if (url == NULL && token == NULL)
    {
        url = global_url;
        token = global_token;
    }
    #if DEBUG
    printf("url :%s\n",url);
    printf("token :%s\n",token);
    #endif
    headers = (char *)calloc(2048,sizeof(char));
    error_num = snprintf(headers,2048,"Authorization: Bearer %s\r\naccept: application/json\r\nConnection: keep-alive\n\r",token);
    if (error_num < 0)
    {
        printf("headers error\n");
        http_data * data = NULL;
    }
    #if DEBUG
    printf("headers :\n%s\n",headers);
    #endif
    http_data * data = http_main(url, path, 5700, "GET", 1,headers,"");
    free(headers);
    return data;
}


void run_corn(char *url,const char *token,const int id)
{
    char * headers;
    char * body;
    int error_num;
    http_data * data;
    char *path = "/open/crons/run";
    if (url == NULL && token == NULL)
    {
        url = global_url;
        token = global_token;
    }
    #if DEBUG
    printf("url :%s\n",url);
    printf("token :%s\n",token);
    printf("id :%d\n",id);
    #endif
    body = (char *)calloc(2048,sizeof(char));
    error_num = snprintf(body,2048,"[%d]",id);
    headers = (char *)calloc(2048,sizeof(char));
    error_num = snprintf(headers,2048,        
                                                "User-Agent: c\r\n"
                                                        "accept: application/json\r\n"
                                                        "Accept-Encoding: gzip, deflate, br, zstdn\r\n"
                                                        "Connection: keep-alive\r\n"
                                                        "Authorization: Bearer %s\r\n"
                                                        "Content-Length: %zu\r\n"
                                                        "Content-Type: application/json\r\n"
                                                        ,token,(strlen(body)));
    if (error_num < 0)
    {
        printf("headers error\n");
    }
    #if DEBUG
    printf("headers :\n%s\n",headers);
    printf("body :\n%s\n",body);
    #endif
    data = http_main(url, path, 5700, "PUT", 1,headers,body);
    free(headers);
}
