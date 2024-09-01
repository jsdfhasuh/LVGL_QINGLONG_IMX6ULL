#ifndef LVGL_FUNC_H
#define LVGL_FUNC_H
#include "cJSON.h"
#include "http.h"


struct my_lvgl_data {
    int x;
    int y;
    int z;
};

void lvgl_func(cJSON *json);
void change_data(void);
#endif // LVGL_FUNC_H