#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include "cJSON.h"
#include "http.h"
#include "ql_api.h"

#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "lvgl_func.h"

#define DISP_BUF_SIZE (1024 * 600)


int main(void)
{
    /*LittlevGL init*/
    lv_init();
    int element;
    char *body;
    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_draw_buf_t disp_buf;
    lv_disp_draw_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.draw_buf   = &disp_buf;
    disp_drv.flush_cb   = fbdev_flush;
    disp_drv.hor_res    = 1024;
    disp_drv.ver_res    = 600;
    lv_disp_drv_register(&disp_drv);

	/* Linux input device init */
    evdev_init();
	
    /* Initialize and register a display input driver */
    lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);      /*Basic initialization*/

    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = evdev_read;
    lv_indev_t * my_indev = lv_indev_drv_register(&indev_drv); 
    return_data temp_data;
    http_data *data;
    char url[] = "192.168.100.1";
    char client_id[] = "8v4_ZqAd--zy";
    char client_sercret[] = "5_zaHgMj9BzHc39RxsMnP3_1";
    temp_data = ql_login(url,client_id,client_sercret);
    data = get_crons(url,temp_data.data);
    //run_corn(url, temp_data.data, 73);
    body = data->body;
    printf("body is \n%s\n",body);
    cJSON *json = cJSON_GetObjectItem(data->response_json, "data");
    json = cJSON_GetObjectItem(json, "data");

    /*Create a Demo*/
    lvgl_func(json);

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_task_handler();
        usleep(5000);
    }

    return 0;
}


/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
