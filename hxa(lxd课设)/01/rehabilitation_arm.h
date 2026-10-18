#ifndef __REHABILITATION_ARM_H
#define __REHABILITATION_ARM_H

#include "main.h"
#include "config.h"

typedef struct {
    uint8_t refresh_request;  // 刷新请求标志
    uint8_t page_index;       // 当前显示页
    uint32_t last_refresh;    // 上次刷新时间
    uint8_t pending_update;   // 有待更新
} DisplayManager_t;

// 函数声明
void System_Init_Rehabilitation(void);
void Main_Task_Rehabilitation(void);
void Display_Update_Safe(void);

#endif /* __REHABILITATION_ARM_H */
