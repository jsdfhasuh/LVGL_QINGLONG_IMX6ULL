#ifndef LVGL_FUNC_H
#define LVGL_FUNC_H
#include "cJSON.h"
#include "http.h"




void lvgl_func(cJSON *json);
void change_data(void);
void fill_table(cJSON *json);
#endif // LVGL_FUNC_H