#include "traffic_light.h"

void TrafficLight_Init(void)
{
    HAL_GPIO_WritePin(LED1_R_GPIO_Port, LED1_R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED1_Y_GPIO_Port, LED1_Y_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED1_G_GPIO_Port, LED1_G_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_R_GPIO_Port, LED2_R_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_Y_GPIO_Port, LED2_Y_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(LED2_G_GPIO_Port, LED2_G_Pin, GPIO_PIN_RESET);
}

void TrafficLight_SetColor(uint8_t laneId, LedColor_t color)
{
    GPIO_TypeDef *rPort, *yPort, *gPort;
    uint16_t rPin, yPin, gPin;
    
    if (laneId == TRAFFIC_LIGHT_LANE1) {
        rPort = LED1_R_GPIO_Port; rPin = LED1_R_Pin;
        yPort = LED1_Y_GPIO_Port; yPin = LED1_Y_Pin;
        gPort = LED1_G_GPIO_Port; gPin = LED1_G_Pin;
    } else {
        rPort = LED2_R_GPIO_Port; rPin = LED2_R_Pin;
        yPort = LED2_Y_GPIO_Port; yPin = LED2_Y_Pin;
        gPort = LED2_G_GPIO_Port; gPin = LED2_G_Pin;
    }
    
    HAL_GPIO_WritePin(rPort, rPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(yPort, yPin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(gPort, gPin, GPIO_PIN_RESET);
    
    switch (color) {
        case LED_COLOR_RED:
            HAL_GPIO_WritePin(rPort, rPin, GPIO_PIN_SET);
            break;
        case LED_COLOR_YELLOW:
            HAL_GPIO_WritePin(yPort, yPin, GPIO_PIN_SET);
            break;
        case LED_COLOR_GREEN:
            HAL_GPIO_WritePin(gPort, gPin, GPIO_PIN_SET);
            break;
        default:
            break;
    }
}

void TrafficLight_SetAll(LedColor_t color1, LedColor_t color2)
{
    TrafficLight_SetColor(TRAFFIC_LIGHT_LANE1, color1);
    TrafficLight_SetColor(TRAFFIC_LIGHT_LANE2, color2);
}

void TrafficLight_Off(uint8_t laneId)
{
    TrafficLight_SetColor(laneId, LED_COLOR_OFF);
}

void TrafficLight_AllOff(void)
{
    TrafficLight_Off(TRAFFIC_LIGHT_LANE1);
    TrafficLight_Off(TRAFFIC_LIGHT_LANE2);
}
