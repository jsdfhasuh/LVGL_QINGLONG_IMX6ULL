#ifndef __TOOL__
#define __TOOL__
struct user_data{
    char * url;
    char * token;
};
void convert_timestamp(int timestamp_str, char* output_str);
int get_current_timestamp();
#endif