

#include <stdio.h>
#include "../lvgl/lvgl.h"
#include "include/lvgl_func.h"
#include "cJSON.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "lv_event.h"
#include "lv_label.h"
#include "lv_obj_style.h"
#include "lv_table.h"
#include "lvgl_func.h"
#include "tool.h"

lv_obj_t * base_screen;
lv_obj_t * label_screen;

struct my_lvgl_data data = {1, 2, 3};

void change_data(void)
{
    data.x++;
    data.y++;
    data.z++;
}


// 回调函数，当表格中的单元格被点击时触发
static void table_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    uint16_t col =0;
    uint16_t row = 0;
    if (code == LV_EVENT_PRESSING) {
        //lv_scr_load(label_screen);
        lv_table_get_selected_cell(obj, &row, &col);
        printf("Cell clicked: row %d, col %d\n", row, col);
    }
    else if (code == LV_EVENT_RELEASED)
    {
        lv_scr_load(base_screen);
    }
}

void lvgl_func(cJSON *json)
{
    int height;
    height = cJSON_GetArraySize(json);
    printf("height = %d\n", height);
    LV_FONT_DECLARE(SourceHanSan);                         // 声明外部字库

    char *type[] = {"name","status","last_execution_time"};
    #if 1
    base_screen = lv_obj_create(NULL);
    lv_scr_load(base_screen);
    label_screen = lv_label_create(NULL);
    lv_obj_set_style_pad_all(lv_scr_act(), 0, 0);
    lv_obj_set_style_outline_width(lv_scr_act(), 0, 0);
    lv_obj_set_size(base_screen, LV_PCT(100), LV_PCT(100));
    lv_refr_now(NULL);
    lv_coord_t base_screen_width = lv_obj_get_width(base_screen);
    printf("base_screen_width = %d\n", base_screen_width);
    lv_obj_align(base_screen, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(base_screen, LV_FLEX_FLOW_COLUMN); // 设置弹性布局为列方向
    lv_obj_set_style_pad_all(base_screen, 0, 0);
    lv_obj_set_style_outline_width(base_screen, 0, 0);




    // 创建表头容器
    lv_obj_t * header_container = lv_obj_create(base_screen);
    lv_obj_set_size(header_container, base_screen_width, LV_PCT(10));
    lv_obj_align(header_container, LV_ALIGN_TOP_MID, 0, 0);
    // 设置表头容器的 padding 和 margin 全部为 0
    lv_obj_set_style_pad_all(header_container, 0, 0);
    lv_obj_set_style_outline_width(header_container, 0, 0);
    // 隐藏表头容器的滚动条
    lv_obj_set_scrollbar_mode(header_container, LV_SCROLLBAR_MODE_OFF);
    

    // 在表头容器中创建一行的表格，包含4个单元格
    lv_obj_t * header_table = lv_table_create(header_container);
    lv_obj_set_scrollbar_mode(header_table, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_pad_all(header_table, 0, 0);
    lv_obj_set_style_outline_width(header_table, 0, 0);
    lv_table_set_col_cnt(header_table, 4); // 设置列数
    lv_table_set_row_cnt(header_table, 1); // 设置行数
    lv_obj_set_style_text_font(header_table, &SourceHanSan, 0);
    lv_refr_now(NULL);
    lv_table_set_cell_value(header_table, 0, 0, "名称");
    lv_table_set_cell_value(header_table, 0, 1, "运行状态");
    lv_table_set_cell_value(header_table, 0, 2, "最后运行时间");
    lv_table_set_cell_value(header_table, 0, 3, "下次运行时间");


    // 设置每个单元格的宽度均分
    lv_table_set_col_width(header_table, 0, base_screen_width / 4.1);
    lv_table_set_col_width(header_table, 1, base_screen_width / 4.1);
    lv_table_set_col_width(header_table, 2, base_screen_width / 4.1);
    lv_table_set_col_width(header_table, 3, base_screen_width / 4.1);


    lv_obj_set_size(header_table, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_align(header_table, LV_ALIGN_CENTER, 0, 0);

    // 创建表格容器
    lv_obj_t * table_container = lv_obj_create(base_screen);
    lv_obj_set_size(table_container, LV_PCT(100), LV_PCT(90));
    lv_obj_align(table_container, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_scrollbar_mode(table_container, LV_SCROLLBAR_MODE_AUTO);
    // 设置表头容器的 padding 和 margin 全部为 0
    lv_obj_set_style_pad_all(table_container, 0, 0);
    lv_obj_set_style_outline_width(table_container, 0, 0);

    // 在表格容器中创建表格
    lv_obj_t * table = lv_table_create(table_container);
    lv_table_set_col_cnt(table, 4); // 设置列数
    lv_table_set_row_cnt(table, height); // 设置行数
    // 设置每个单元格的宽度均分
    lv_table_set_col_width(table, 0, base_screen_width / 4.1);
    lv_table_set_col_width(table, 1, base_screen_width / 4.1);
    lv_table_set_col_width(table, 2, base_screen_width / 4.1);
    lv_table_set_col_width(table, 3, base_screen_width / 4.1);
    lv_obj_set_style_text_font(table, &SourceHanSan, 0);
    char *name;
    char *status;
    int last_execution_time;
    char *last_execution_time_str = (char *)malloc(100);
    
    // 填充表格内容
    for (int row = 0; row < height; row++) {
        cJSON *item = cJSON_GetArrayItem(json, row);
        printf("item = %s\n", cJSON_Print(item));
        name = cJSON_GetObjectItem(item,type[0])->valuestring;
        printf("name = %s\n", name);
        status = cJSON_GetObjectItem(item,type[1])->valueint==0?"运行中":"已停止";
        printf("status = %s\n", status);
        last_execution_time = cJSON_GetObjectItem(item,type[2])->valueint;
        //printf("last_execution_time = %d\n", last_execution_time);
        convert_timestamp(last_execution_time, last_execution_time_str);
        printf("last_execution_time = %s\n", last_execution_time_str);
        
        lv_table_set_cell_value(table, row, 0, name);
        lv_table_set_cell_value(table, row, 1, status);
        lv_table_set_cell_value(table,row,2,last_execution_time_str);
    }

    // 设置表格大小和对齐方式
    lv_obj_set_size(table, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_align(table, LV_ALIGN_TOP_MID, 0, 0);
    //lv_obj_add_event_cb(table, table_event_cb, LV_EVENT_ALL, NULL);

    //sleep(5);
    //lv_scr_load(label_screen);

    #endif
}

