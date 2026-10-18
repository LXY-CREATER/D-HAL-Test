/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : motor_controller_v2.h
  * @brief          : Advanced Motor Controller with AT8870 Driver
  * @details        : This module provides robust motor control with:
  *                   - Proper PWM and GPIO management
  *                   - Emergency stop functionality
  *                   - Position and speed control
  *                   - Safety limits protection
  ******************************************************************************
  * @attention
  *
  * Hardware Configuration:
  * - MCU: STM32F103C8T6
  * - Motor Driver: AT8870 (Dual H-Bridge)
  * - PWM Timer: TIM3 (Channels 1 & 2)
  * - Control Pins: PB4 (IN1), PB5 (IN2)
  *
  * Motor Control Logic for AT8870:
  * - IN1=0, IN2=0: Motor STOP (coast to stop)
  * - IN1=PWM, IN2=0: Motor FORWARD
  * - IN1=0, IN2=PWM: Motor BACKWARD
  * - IN1=1, IN2=1: Motor BRAKE (active braking)
  *
  * Important Notes:
  * - Always stop PWM before changing direction
  * - Ensure both pins are LOW when stopping
  * - Use proper delay between direction changes
  *
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MOTOR_CONTROLLER_V2_H
#define __MOTOR_CONTROLLER_V2_H

#include "main.h"
#include "tim.h"
#include "gpio.h"

/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
/* USER CODE END Includes */

/* USER CODE BEGIN Private defines */

// ============================================================================
// HARDWARE CONSTANTS
// ============================================================================

// Motor Driver Pins (AT8870)
#define MOTOR_IN1_PIN         GPIO_PIN_4
#define MOTOR_IN1_PORT        GPIOB
#define MOTOR_IN2_PIN         GPIO_PIN_5
#define MOTOR_IN2_PORT        GPIOB

// PWM Configuration
#define MOTOR_PWM_TIMER       &htim3
#define MOTOR_PWM_CH1         TIM_CHANNEL_1
#define MOTOR_PWM_CH2         TIM_CHANNEL_2
#define MOTOR_PWM_PERIOD      1000    // 10kHz PWM with 72MHz APB1
#define MOTOR_PWM_PRESCALER   7       // 72MHz / 7 = 10.28MHz

// PWM Duty Cycle Limits
#define MOTOR_PWM_MIN_DUTY    10      // Minimum PWM to start motor
#define MOTOR_PWM_MAX_DUTY    990     // Maximum PWM (99%)
#define MOTOR_PWM_FULL_DUTY   MOTOR_PWM_PERIOD

// Travel Limits (Mechanical)
#define TRAVEL_RANGE_MM       200.0f  // Safe travel range
#define PHYSICAL_TRAVEL_MM    205.0f  // Physical limit
#define SAFETY_MARGIN_MM      5.0f    // Safety margin from ends

// Encoder and Mechanical Parameters
#define PULLEY_TEETH          24      // HTPA24S3M060-B-P8
#define PULLEY_PITCH_MM       3.0f    // 3M timing belt
#define PULLEY_CIRCUMFERENCE  (PULLEY_TEETH * PULLEY_PITCH_MM)  // 72mm
#define BELT_TOTAL_LENGTH     660.0f  // Total belt length

// Motor Control Timing
#define DIRECTION_CHANGE_DELAY_MS  10    // Delay between direction changes
#define BRAKE_DURATION_MS          50    // Active brake duration
#define EMERGENCY_BRAKE_DURATION   100   // Emergency brake duration
#define POSITION_UPDATE_RATE_MS    10    // Position update rate

// ============================================================================
// CONTROL CONSTANTS
// ============================================================================

#define SPEED_MIN_PERCENT     10      // Minimum speed (10%)
#define SPEED_MAX_PERCENT     100     // Maximum speed (100%)
#define SPEED_DEFAULT_PERCENT 50      // Default speed

#define ACCELERATION_RATE     5       // % per update cycle
#define DECELERATION_RATE     10      // % per update cycle (faster for safety)

/* USER CODE END Private defines */

/* USER CODE BEGIN Private typedefs */

// ============================================================================
// ENUMERATIONS
// ============================================================================

/**
 * @brief Motor Direction States
 */
typedef enum {
    MOTOR_DIR_STOP = 0,
    MOTOR_DIR_FORWARD,
    MOTOR_DIR_BACKWARD,
    MOTOR_DIR_BRAKE,
    MOTOR_DIR_ERROR
} MotorDirection_t;

/**
 * @brief Motor Control Mode
 */
typedef enum {
    MOTOR_MODE_IDLE = 0,
    MOTOR_MODE_SPEED,
    MOTOR_MODE_POSITION,
    MOTOR_MODE_TORQUE,
    MOTOR_MODE_IMPEDANCE,
    MOTOR_MODE_CALIBRATION,
    MOTOR_MODE_EMERGENCY_STOP
} MotorControlMode_t;

/**
 * @brief Motor Error States
 */
typedef enum {
    MOTOR_ERROR_NONE = 0,
    MOTOR_ERROR_OVER_TRAVEL_MIN,
    MOTOR_ERROR_OVER_TRAVEL_MAX,
    MOTOR_ERROR_SPEED_LIMIT,
    MOTOR_ERROR_TIMEOUT,
    MOTOR_ERROR_PWM_FAILURE,
    MOTOR_ERROR_ENCODER_FAILURE,
    MOTOR_ERROR_OVERCURRENT
} MotorError_t;

// ============================================================================
// DATA STRUCTURES
// ============================================================================

/**
 * @brief Motor Configuration Structure
 */
typedef struct {
    // PWM Settings
    TIM_HandleTypeDef* pwm_timer;
    uint32_t pwm_ch1;
    uint32_t pwm_ch2;
    uint16_t pwm_period;
    uint16_t pwm_min_duty;
    uint16_t pwm_max_duty;
    
    // GPIO Pins
    GPIO_TypeDef* in1_port;
    uint16_t in1_pin;
    GPIO_TypeDef* in2_port;
    uint16_t in2_pin;
    
    // Mechanical Limits
    float min_position_mm;
    float max_position_mm;
    float home_position_mm;
    
    // Control Parameters
    bool soft_limit_enabled;
    bool torque_limit_enabled;
    float max_speed_mm_s;
    float max_accel_mm_s2;
} MotorConfig_t;

/**
 * @brief Motor State Structure
 */
typedef struct {
    // Basic State
    MotorDirection_t direction;
    MotorControlMode_t mode;
    MotorError_t error;
    
    // Speed Control
    uint8_t target_speed_percent;
    uint8_t current_speed_percent;
    uint16_t target_pwm;
    uint16_t current_pwm;
    
    // Position Control
    float target_position_mm;
    float current_position_mm;
    float position_offset_mm;
    
    // Status Flags
    bool is_moving;
    bool is_calibrated;
    bool is_homed;
    bool emergency_stop_active;
    
    // Timing
    uint32_t last_update_time;
    uint32_t direction_change_time;
    uint32_t stop_timeout;
    
    // Error Tracking
    uint32_t error_count;
    uint32_t last_error_time;
} MotorState_t;

/**
 * @brief Impedance Control Parameters (for rehabilitation)
 */
typedef struct {
    // Stiffness (resistance to position change)
    float stiffness;          // N/mm or similar unit
    float stiffness_min;
    float stiffness_max;
    
    // Damping (resistance to velocity)
    float damping;           // Ns/mm
    float damping_min;
    float damping_max;
    
    // Target Position (center point)
    float center_position_mm;
    
    // Current State
    float current_force;     // Calculated force
    float current_torque;    // Calculated torque
    
    // Limits
    float max_force;
    float max_torque;
} ImpedanceControl_t;

/* USER CODE END Private typedefs */

/* USER CODE BEGIN Private variables */

// Global Motor Configuration
extern MotorConfig_t g_motor_config;

// Global Motor State
extern MotorState_t g_motor_state;

// Global Impedance Control Parameters
extern ImpedanceControl_t g_impedance;

/* USER CODE END Private variables */

/* USER CODE BEGIN Private function prototypes */

// ============================================================================
// INITIALIZATION FUNCTIONS
// ============================================================================

/**
 * @brief Initialize motor controller with default parameters
 * @retval None
 */
void MotorController_Init(void);

/**
 * @brief Initialize motor controller with custom configuration
 * @param config: Motor configuration structure
 * @retval None
 */
void MotorController_Init_Config(const MotorConfig_t* config);

/**
 * @brief Initialize impedance control for rehabilitation mode
 * @param stiffness: Initial stiffness value
 * @param damping: Initial damping value
 * @retval None
 */
void MotorController_Init_Impedance(float stiffness, float damping);

// ============================================================================
// BASIC MOTOR CONTROL
// ============================================================================

/**
 * @brief Set motor direction with proper sequencing
 * @param dir: Target direction
 * @retval None
 */
void Motor_SetDirection(MotorDirection_t dir);

/**
 * @brief Set motor speed by percentage
 * @param percent: Speed percentage (0-100)
 * @retval None
 */
void Motor_SetSpeed(uint8_t percent);

/**
 * @brief Move motor forward at specified speed
 * @param speed_percent: Speed percentage (10-100)
 * @retval None
 */
void Motor_Forward(uint8_t speed_percent);

/**
 * @brief Move motor backward at specified speed
 * @param speed_percent: Speed percentage (10-100)
 * @retval None
 */
void Motor_Backward(uint8_t speed_percent);

/**
 * @brief Stop motor (coast to stop)
 * @retval None
 */
void Motor_Stop(void);

/**
 * @brief Active brake motor
 * @param duration_ms: Brake duration in milliseconds
 * @retval None
 */
void Motor_Brake(uint16_t duration_ms);

/**
 * @brief Emergency stop (hard stop)
 * @retval None
 */
void Motor_EmergencyStop(void);

/**
 * @brief Smooth stop with deceleration
 * @param duration_ms: Stop duration in milliseconds
 * @retval None
 */
void Motor_SmoothStop(uint16_t duration_ms);

// ============================================================================
// POSITION CONTROL
// ============================================================================

/**
 * @brief Move motor to absolute position
 * @param target_mm: Target position in millimeters
 * @param speed_percent: Maximum speed percentage
 * @retval bool: True if position is reachable, false otherwise
 */
bool Motor_MoveToPosition(float target_mm, uint8_t speed_percent);

/**
 * @brief Move motor relative to current position
 * @param distance_mm: Distance to move (positive = forward, negative = backward)
 * @param speed_percent: Maximum speed percentage
 * @retval bool: True if move is possible, false otherwise
 */
bool Motor_MoveRelative(float distance_mm, uint8_t speed_percent);

/**
 * @brief Set current position as reference (zero)
 * @retval None
 */
void Motor_SetHome(void);

/**
 * @brief Set position offset
 * @param offset_mm: Position offset in millimeters
 * @retval None
 */
void Motor_SetPositionOffset(float offset_mm);

/**
 * @brief Check if motor has reached target position
 * @retval bool: True if at target position
 */
bool Motor_IsAtPosition(void);

// ============================================================================
// IMPEDANCE CONTROL (REHABILITATION MODE)
// ============================================================================

/**
 * @brief Enable impedance control mode
 * @retval None
 */
void Motor_EnableImpedanceControl(void);

/**
 * @brief Disable impedance control mode
 * @retval None
 */
void Motor_DisableImpedanceControl(void);

/**
 * @brief Update impedance control based on current position and velocity
 * @param current_position: Current position in mm
 * @param current_velocity: Current velocity in mm/s
 * @retval None
 */
void Motor_UpdateImpedanceControl(float current_position, float current_velocity);

/**
 * @brief Set impedance control parameters
 * @param stiffness: Stiffness value (0-100)
 * @param damping: Damping value (0-100)
 * @retval None
 */
void Motor_SetImpedanceParameters(uint8_t stiffness, uint8_t damping);

/**
 * @brief Set center position for impedance control
 * @param position_mm: Center position in millimeters
 * @retval None
 */
void Motor_SetImpedanceCenter(float position_mm);

// ============================================================================
// STATUS AND MONITORING
// ============================================================================

/**
 * @brief Get current motor direction
 * @retval MotorDirection_t: Current direction
 */
MotorDirection_t Motor_GetDirection(void);

/**
 * @brief Get current speed percentage
 * @retval uint8_t: Current speed percentage
 */
uint8_t Motor_GetSpeed(void);

/**
 * @brief Get current position in millimeters
 * @retval float: Current position
 */
float Motor_GetPosition(void);

/**
 * @brief Get current motor mode
 * @retval MotorControlMode_t: Current control mode
 */
MotorControlMode_t Motor_GetMode(void);

/**
 * @brief Get current motor error
 * @retval MotorError_t: Current error state
 */
MotorError_t Motor_GetError(void);

/**
 * @brief Check if motor is moving
 * @retval bool: True if moving
 */
bool Motor_IsMoving(void);

/**
 * @brief Check if emergency stop is active
 * @retval bool: True if emergency stop is active
 */
bool Motor_IsEmergencyStopActive(void);

// ============================================================================
// SAFETY AND LIMITS
// ============================================================================

/**
 * @brief Set soft limits for position
 * @param min_mm: Minimum position in millimeters
 * @param max_mm: Maximum position in millimeters
 * @retval bool: True if limits are valid
 */
bool Motor_SetSoftLimits(float min_mm, float max_mm);

/**
 * @brief Enable or disable soft limits
 * @param enable: True to enable, false to disable
 * @retval None
 */
void Motor_SetSoftLimitEnable(bool enable);

/**
 * @brief Check if position is within safe limits
 * @param position_mm: Position to check
 * @retval bool: True if position is safe
 */
bool Motor_IsPositionSafe(float position_mm);

/**
 * @brief Clear motor error
 * @retval None
 */
void Motor_ClearError(void);

/**
 * @brief Reset motor controller to initial state
 * @retval None
 */
void Motor_Reset(void);

// ============================================================================
// INTERNAL FUNCTIONS (Called from main loop)
// ============================================================================

/**
 * @brief Main motor control update function
 * @param current_position_mm: Current position from encoder
 * @param current_velocity_mm_s: Current velocity from encoder
 * @retval None
 */
void Motor_Update(float current_position_mm, float current_velocity_mm_s);

/**
 * @brief Update motor position
 * @param position_mm: New position in millimeters
 * @retval None
 */
void Motor_UpdatePosition(float position_mm);

/* USER CODE END Private function prototypes */

#endif /* __MOTOR_CONTROLLER_V2_H */
