

#include <stdio.h>
#include "../lvgl/lvgl.h"
#include "include/lvgl_func.h"
#include "cJSON.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "http.h"
#include "lv_dropdown.h"
#include "lv_event.h"
#include "lv_label.h"
#include "lv_obj_pos.h"
#include "lv_obj_style.h"
#include "lv_table.h"
#include "lvgl_func.h"
#include "tool.h"
#include "ql_api.h"

lv_obj_t * base_screen;
lv_obj_t * control_screen;
lv_obj_t * data_table;
lv_obj_t * dd;
lv_obj_t * log_txt_table;


struct key_data {
    int col;
    int row;
    int status; // 0: 未完成 1: 完成 2：未开始
};
struct key_data first_key_data = {65535,65535,2};
struct key_data second_key_data = {65535,65535,2};
cJSON *my_all_json;   //所以后端传递数据的整理
http_data *log_http_data;

static void refresh_key_data()
{
    first_key_data.col = 65535;
    first_key_data.row = 65535;
    first_key_data.status = 2;
    second_key_data.col = 65535;
    second_key_data.row = 65535;
    second_key_data.status = 2;
}

static int get_taskid(int row)
{
    char option_index_text[10];
    int task_id;
    sprintf(option_index_text, "%d",row);
    cJSON *target_json = cJSON_GetObjectItem(my_all_json,option_index_text);
    task_id = cJSON_GetObjectItem(target_json,"task_id")->valueint;
    return task_id;
}

// 获取按钮的标签文本
static char * get_button_label_text(lv_obj_t * btn) {
    // 获取按钮的第一个子对象（假设是标签）
    lv_obj_t * label = lv_obj_get_child(btn, NULL);
    if (label != NULL && lv_obj_check_type(label, &lv_label_class)) {
        // 获取标签的文本
        return lv_label_get_text(label);
    }
    return NULL;
}


// 回调函数，当表格中的单元格被点击时触发
static void table_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    uint16_t col =0;
    uint16_t row = 0;
    int task_id;
    if (code == LV_EVENT_PRESSING) {
        //lv_scr_load(control_screen);
        lv_table_get_selected_cell(obj, &row, &col);
        printf("Cell LV_EVENT_PRESSING: row %d, col %d\n", row, col);
        if (first_key_data.col == 65535)
        {
            first_key_data.col = col;
            first_key_data.row = row;
            first_key_data.status = 0;
        }
        else
        {
            second_key_data.col = col;
            second_key_data.row = row;
            second_key_data.status = 0;
        }
    }
    else if (code == LV_EVENT_RELEASED)
    {
        printf("key %d %d finish", first_key_data.row, first_key_data.col);
        if (first_key_data.status == 0)
        {
            first_key_data.status = 1;
            //printf("完成第一个键");
        }
        else if (second_key_data.status == 0)
        {
            second_key_data.status = 1;
            //printf("完成第二个键");
        
        }
        if (first_key_data.status == 1 && second_key_data.status == 1)
        {
            printf("开始判断双击情况\n");
            if ((first_key_data.col == second_key_data.col) && (first_key_data.row == second_key_data.row))
            {
                printf("双击\n");
                task_id = get_taskid(first_key_data.row);
                printf("task_id = %d\n", task_id);
                // 获得日志的http_data
                log_http_data = get_log(NULL, NULL, task_id);
                if (log_http_data == NULL || log_http_data->response_json == NULL) 
                {
                    printf("response_json is NULL\n");
                }
                else{
                    printf("get log");
                    char *text = cJSON_GetObjectItem(log_http_data->response_json, "data")->valuestring;
                    printf("text = \n%s\n", text);
                    lv_textarea_set_text(log_txt_table, text);
                    printf("finish log");
                    //printf("text = \n%s\n", text);
                }
                lv_dropdown_set_selected(dd, first_key_data.row);
                lv_scr_load(control_screen);
                refresh_key_data();
            }
            else
            {
                printf("不是双击\n");
                refresh_key_data();
            }
        }
        //lv_scr_load(base_screen);
    }
}

static void button_event_cb(lv_event_t * e) {
    char *text;
    int select_option_index;
    char option_index_text[10];
    cJSON *target_json;
    int task_id;
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if (code == LV_EVENT_CLICKED) 
    {
        printf("Button clicked\n");
        text = get_button_label_text(obj);
        select_option_index = lv_dropdown_get_selected(dd);
        sprintf(option_index_text, "%d",select_option_index);
        #if DEBUG
        printf("select_option_index = %d\n", select_option_index);
        #endif
        if (text != NULL) 
        {
            //printf("Button text: %s\n", text);
            target_json = cJSON_GetObjectItem(my_all_json,option_index_text);
            task_id = cJSON_GetObjectItem(target_json,"task_id")->valueint;
            #if DEBUG
            printf("task_id = %d\n", task_id);
            printf("Button text: %s\n", text);
            #endif
            if (strcmp(text, "运行") == 0) {
                //free(log_http_data->response);
                printf("运行%d\n",task_id);
                run_corn(NULL, NULL, task_id);
                lv_scr_load(base_screen);
            }
            else if (strcmp(text, "停止") == 0) 
            {
                //free(log_http_data->response);
                printf("停止%d\n",task_id);
                lv_scr_load(base_screen);
            }
            else if (strcmp(text, "退出") == 0) 
            {
                //free(log_http_data->response);
                printf("退出%d\n");
                lv_scr_load(base_screen);
            }
        }
    }
}

void fill_table(cJSON *json)
{
    if (cJSON_GetArraySize(my_all_json) != 0)
    {
        cJSON_Delete(my_all_json);
        my_all_json = cJSON_CreateObject();
    }
    char *json_str;
    //printf("json:\n %s\n", json_str);
    char *type[] = {"name","status","last_execution_time"};
    char *name;
    char *status;
    int id;
    char id_str[12];
    char option_text[12];
    char option_index_text[12];
    int last_execution_time;
    char *last_execution_time_str = (char *)malloc(100);
    int height = lv_table_get_row_cnt(data_table);
    lv_dropdown_clear_options(dd);
    # if 1
    printf("height = %d\n", height);
    #endif
        for (int row = 0; row < height; row++) {
        cJSON *item = cJSON_GetArrayItem(json, row);
        //printf("item = %s\n", cJSON_Print(item));
        name = cJSON_GetObjectItem(item,type[0])->valuestring;
        //printf("name = %s\n", name);
        status = cJSON_GetObjectItem(item,type[1])->valueint==0?"运行中":"已停止";
        //printf("status = %s\n", status);
        last_execution_time = cJSON_GetObjectItem(item,type[2])->valueint;
        id = cJSON_GetObjectItem(item,"id")->valueint;
        //printf("last_execution_time = %d\n", last_execution_time);
        convert_timestamp(last_execution_time, last_execution_time_str);
        //printf("last_execution_time = %s\n", last_execution_time_str);
        lv_table_set_cell_value(data_table, row, 0, name);
        lv_table_set_cell_value(data_table, row, 1, status);
        lv_table_set_cell_value(data_table, row,2,last_execution_time_str);
        sprintf(option_text, "%s_%d", name,id);
        lv_dropdown_add_option(dd, option_text, LV_DROPDOWN_POS_LAST);
        cJSON *nested_json = cJSON_CreateObject();
        cJSON_AddStringToObject(nested_json, "name", name);
        cJSON_AddStringToObject(nested_json, "status", status);
        cJSON_AddStringToObject(nested_json, "last_execution_time", last_execution_time_str);
        cJSON_AddNumberToObject(nested_json, "task_id", id);
        cJSON_AddStringToObject(nested_json, "option_text;", option_text);
        sprintf(id_str, "%d", id);
        sprintf(option_index_text, "%d",row);
        cJSON_AddItemToObject(my_all_json, option_index_text, nested_json);
    }
    json_str = cJSON_Print(my_all_json);
    printf("json_str:\n %s\n", json_str);
    //http_data *data = get_log(NULL, NULL, 21);
    //printf("body is\n%s\n",data->body);
    //free(json_str);
}


void lvgl_func(cJSON *json)
{
    int height;
    // 初始化 my_all_json
    my_all_json = cJSON_CreateObject();

    height = cJSON_GetArraySize(json);
    #if DEBUG
    printf("user_data url = %s\n", my_user_data.url);
    printf("height = %d\n", height);
    #endif
    LV_FONT_DECLARE(SourceHanSan);                         // 声明外部字库

    
    #if 1
    base_screen = lv_obj_create(NULL);
    lv_scr_load(base_screen);
    
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
    lv_table_set_cell_value(header_table, 0, 3, "任务ID");


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
    data_table = lv_table_create(table_container);
    lv_table_set_col_cnt(data_table, 4); // 设置列数
    lv_table_set_row_cnt(data_table, height); // 设置行数
    // 设置每个单元格的宽度均分
    lv_table_set_col_width(data_table, 0, base_screen_width / 4.1);
    lv_table_set_col_width(data_table, 1, base_screen_width / 4.1);
    lv_table_set_col_width(data_table, 2, base_screen_width / 4.1);
    lv_table_set_col_width(data_table, 3, base_screen_width / 4.1);
    lv_obj_set_style_text_font(data_table, &SourceHanSan, 0);
    

    // 设置表格大小和对齐方式
    lv_obj_set_size(data_table, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_align(data_table, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_add_event_cb(data_table, table_event_cb, LV_EVENT_ALL, NULL);

    //sleep(5);
    //lv_scr_load(control_screen);
    
    // 创建定时器
    //lv_timer_t * timer = lv_timer_create(refresh_data, 500,  my_user_data_ptr);

    #endif

    // 编写control_screen
    control_screen = lv_label_create(NULL);
    lv_obj_set_size(control_screen, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_text_font(control_screen, &SourceHanSan, 0);
    lv_refr_now(NULL);
    //lv_scr_load(control_screen);
    lv_obj_align(control_screen, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(control_screen, LV_FLEX_FLOW_COLUMN); // 设置弹性布局为列方向
    lv_obj_set_style_pad_all(control_screen, 0, 0);
    lv_obj_set_style_outline_width(control_screen, 0, 0);

    //创建control_screen 的表头
    lv_obj_t * control_header_container = lv_obj_create(control_screen);
    
    lv_obj_set_size(control_header_container, base_screen_width, LV_PCT(10));
    lv_obj_align(control_header_container, LV_ALIGN_TOP_MID, 0, 0);
    // 设置表头容器的 padding 和 margin 全部为 0
    lv_obj_set_style_pad_all(control_header_container, 0, 0);
    lv_obj_set_style_outline_width(control_header_container, 0, 0);
    // 隐藏表头容器的滚动条
    lv_obj_set_scrollbar_mode(control_header_container, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(control_header_container, LV_FLEX_FLOW_ROW); // 设置弹性布局为列方向
    // 在表头容器创建一个按钮和下拉框
    lv_obj_t * start_btn = lv_btn_create(control_header_container);
    lv_obj_t * start_label = lv_label_create(start_btn);
    lv_obj_add_event_cb(start_btn, button_event_cb, LV_EVENT_ALL, NULL);
    lv_label_set_text(start_label, "运行"); // 在按钮上添加文字
    lv_obj_t * stop_btn = lv_btn_create(control_header_container);
    lv_obj_t * stop_label = lv_label_create(stop_btn);
    lv_obj_add_event_cb(stop_btn, button_event_cb, LV_EVENT_ALL, NULL);
    lv_label_set_text(stop_label, "停止");
    lv_obj_t * change_btn = lv_btn_create(control_header_container);
    lv_obj_align(change_btn, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_t * change_label = lv_label_create(change_btn);
    lv_obj_add_event_cb(change_btn, button_event_cb, LV_EVENT_ALL, NULL);
    lv_label_set_text(change_label, "退出");
    
    
    dd = lv_dropdown_create(control_header_container);
    lv_obj_set_width(dd, base_screen_width*0.3);
    log_txt_table = lv_textarea_create(control_screen);

    lv_obj_set_size(log_txt_table, base_screen_width, LV_PCT(80));
    lv_textarea_set_text(log_txt_table, "hello"); // 设置初始文本
    lv_textarea_set_cursor_click_pos(log_txt_table, false); // 禁用光标点击位置
    lv_textarea_set_accepted_chars(log_txt_table, ""); // 禁用所有字符输入


    // 填充数据
    fill_table(json);
    
}

