#ifndef __QL_API_H__
#define __QL_API_H__
#include "http.h"
#include "cJSON.h"

return_data ql_login(const char *url,const char *client_id,const char *client_sercret);
http_data* get_env(const char *url,const char *token,const char *name);
http_data* get_crons(char *url,const char *token);
void run_corn(char *url,const char *token,const int id);
#endif