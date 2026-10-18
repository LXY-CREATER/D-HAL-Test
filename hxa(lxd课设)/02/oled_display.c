// oled_display.c
#include "oled_display.h"
#include "oled.h"  // �����µ�OLED����ͷ�ļ�
#include "all_includes.h"

#include "global_vars.h"
extern I2C_HandleTypeDef hi2c1;

// ��������״̬
static OLEDStatus_t g_oled = {0};

// ȫ�ֱ������壨��app_globals.c����
static SystemStatus_t* get_system_ptr(void) {
    static SystemStatus_t sys = {0};
    return &sys;
}

static RehabilitationParams_t* get_rehab_ptr(void) {
    static RehabilitationParams_t rehab = {0};
    return &rehab;
}

void OLED_Display_Init(I2C_HandleTypeDef* i2c)
{
    (void)i2c;
    
    // ʹ���µ�OLED������ʼ��
    OLED_Init();
    
    // ����
    OLED_Clear();
    
    // ��ʾ������Ϣ
    OLED_Show_String(0, 0, "OLED Init OK", 6);
    OLED_Refresh();
    
    // ���ó�ʼ״̬
    g_oled.current_page = PAGE_MAIN_STATUS;
    g_oled.display_mode = DISPLAY_NORMAL;
    g_oled.last_update = 0;
    g_oled.need_refresh = 1;
    g_oled.is_initialized = 1;
    g_oled.brightness = 128;
    
    // �ӳ�ȷ����ʾ
    HAL_Delay(100);
}

void OLED_Show_Main_Status(void)
{
    OLED_Clear();
    
    // ��һ�У�����
    OLED_Show_String(0, 0, "Rehab Arm v1.0", 6);
    
    // �ڶ��У�ϵͳ״̬
    char status[16];
    SystemStatus_t* sys = get_system_ptr();
    switch (sys->mode) {
        case MODE_IDLE:
            sprintf(status, "Mode: IDLE");
            break;
        case MODE_MANUAL:
            sprintf(status, "Mode: MANUAL");
            break;
        case MODE_REHABILITATION:
            sprintf(status, "Mode: REHAB");
            break;
        case MODE_CALIBRATION:
            sprintf(status, "Mode: CALIB");
            break;
    }
    OLED_Show_String(0, 1, status, 6);
    
    // �����У�����״̬
    sprintf(status, "BT: %s", Bluetooth_Is_Connected() ? "OK" : "NC");
    OLED_Show_String(0, 2, status, 6);
    
    // �����У����״̬
    sprintf(status, "Motor: %d%%", Motor_Get_Speed());
    OLED_Show_String(0, 3, status, 6);
    
    // �����У�������λ��
    sprintf(status, "Pos: %ld", Encoder_Get_Count());
    OLED_Show_String(0, 4, status, 6);
    
    // �����У�����ģʽ״̬
    RehabilitationParams_t* rehab = get_rehab_ptr();
    if (rehab->is_active) {
        sprintf(status, "Rehab: Lv%d", rehab->resistance_level);
    } else {
        sprintf(status, "Rehab: OFF");
    }
    OLED_Show_String(0, 5, status, 6);
    
    // �����У���ʾ
    OLED_Show_String(0, 6, "F/B/S/R/E", 6);
    
    OLED_Refresh();
}
