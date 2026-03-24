// motor.c
#include "motor.h"
#include "main.h"

// ==================== 前轮电机引脚定义 (TB6612 模块一) ====================
// 左前电机 (Motor A)
#define AIN1_PIN GPIO_Pin_12
#define AIN2_PIN GPIO_Pin_13

// 右前电机 (Motor B)
#define BIN1_PIN GPIO_Pin_14
#define BIN2_PIN GPIO_Pin_15

// 前轮 PWM 引脚
#define PWMA_PIN GPIO_Pin_0
#define PWMB_PIN GPIO_Pin_1

// ==================== 后轮电机引脚定义 (TB6612 模块二) ====================
// 左后电机 (Motor C)
#define AIN1_PIN_2 GPIO_Pin_4
#define AIN2_PIN_2 GPIO_Pin_5

// 右后电机 (Motor D)
#define BIN1_PIN_2 GPIO_Pin_1
#define BIN2_PIN_2 GPIO_Pin_0

// 后轮 PWM 引脚
#define PWMA_PIN_2 GPIO_Pin_8
#define PWMB_PIN_2 GPIO_Pin_9

// ==================== 端口定义 ====================
// 前轮端口
#define MOTOR_AIN1_PORT GPIOB
#define MOTOR_AIN2_PORT GPIOB
#define MOTOR_BIN1_PORT GPIOB
#define MOTOR_BIN2_PORT GPIOB
#define MOTOR_PWMA_PORT GPIOB
#define MOTOR_PWMB_PORT GPIOB

// 后轮端口
#define MOTOR_AIN1_2_PORT GPIOA
#define MOTOR_AIN2_2_PORT GPIOA
#define MOTOR_BIN1_2_PORT GPIOC
#define MOTOR_BIN2_2_PORT GPIOC
#define MOTOR_PWMA_2_PORT GPIOB
#define MOTOR_PWMB_2_PORT GPIOB

// 待机控制引脚 (两个模块共用 STBY)
#define STBY_PIN GPIO_Pin_3
#define MOTOR_STBY_PORT GPIOB

// 当前速度
static uint8_t current_speed = 100;

// 初始化电机 GPIO
void Motor_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // 使能所有 GPIO 端口时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
    
    // 初始化前轮电机控制引脚
    GPIO_InitStruct.GPIO_Pin = AIN1_PIN | AIN2_PIN | BIN1_PIN | BIN2_PIN | PWMA_PIN | PWMB_PIN | STBY_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 初始化后轮电机控制引脚
    GPIO_InitStruct.GPIO_Pin = AIN1_PIN_2 | AIN2_PIN_2;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = BIN1_PIN_2 | BIN2_PIN_2;
    GPIO_Init(GPIOC, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = PWMA_PIN_2 | PWMB_PIN_2;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 置高 STBY 引脚，使能 TB6612 模块
    GPIO_SetBits(MOTOR_STBY_PORT, STBY_PIN);
    
    Motor_StopAll();
}

// 设置前轮电机方向
static void set_front_motor_direction(uint8_t left_dir, uint8_t right_dir) {
    // 左前电机
    if (left_dir == 1) {
        GPIO_SetBits(MOTOR_AIN1_PORT, AIN1_PIN);
        GPIO_ResetBits(MOTOR_AIN2_PORT, AIN2_PIN);
    } else if (left_dir == 2) {
        GPIO_ResetBits(MOTOR_AIN1_PORT, AIN1_PIN);
        GPIO_SetBits(MOTOR_AIN2_PORT, AIN2_PIN);
    } else {
        GPIO_ResetBits(MOTOR_AIN1_PORT, AIN1_PIN);
        GPIO_ResetBits(MOTOR_AIN2_PORT, AIN2_PIN);
    }

    // 右前电机
    if (right_dir == 1) {
        GPIO_SetBits(MOTOR_BIN1_PORT, BIN1_PIN);
        GPIO_ResetBits(MOTOR_BIN2_PORT, BIN2_PIN);
    } else if (right_dir == 2) {
        GPIO_ResetBits(MOTOR_BIN1_PORT, BIN1_PIN);
        GPIO_SetBits(MOTOR_BIN2_PORT, BIN2_PIN);
    } else {
        GPIO_ResetBits(MOTOR_BIN1_PORT, BIN1_PIN);
        GPIO_ResetBits(MOTOR_BIN2_PORT, BIN2_PIN);
    }
}

// 设置后轮电机方向
static void set_rear_motor_direction(uint8_t left_dir, uint8_t right_dir) {
    // 左后电机
    if (left_dir == 1) {
        GPIO_SetBits(MOTOR_AIN1_2_PORT, AIN1_PIN_2);
        GPIO_ResetBits(MOTOR_AIN2_2_PORT, AIN2_PIN_2);
    } else if (left_dir == 2) {
        GPIO_ResetBits(MOTOR_AIN1_2_PORT, AIN1_PIN_2);
        GPIO_SetBits(MOTOR_AIN2_2_PORT, AIN2_PIN_2);
    } else {
        GPIO_ResetBits(MOTOR_AIN1_2_PORT, AIN1_PIN_2);
        GPIO_ResetBits(MOTOR_AIN2_2_PORT, AIN2_PIN_2);
    }

    // 右后电机
    if (right_dir == 1) {
        GPIO_SetBits(MOTOR_BIN1_2_PORT, BIN1_PIN_2);
        GPIO_ResetBits(MOTOR_BIN2_2_PORT, BIN2_PIN_2);
    } else if (right_dir == 2) {
        GPIO_ResetBits(MOTOR_BIN1_2_PORT, BIN1_PIN_2);
        GPIO_SetBits(MOTOR_BIN2_2_PORT, BIN2_PIN_2);
    } else {
        GPIO_ResetBits(MOTOR_BIN1_2_PORT, BIN1_PIN_2);
        GPIO_ResetBits(MOTOR_BIN2_2_PORT, BIN2_PIN_2);
    }
}

// 设置前轮 PWM 速度
void Motor_SetFrontSpeed(uint8_t speed) {
    if (speed > 100) speed = 100;
    
    uint32_t pulse = (uint32_t)(speed * 1000 / 100);
    TIM_SetCompare3(TIM3, pulse);
    TIM_SetCompare4(TIM3, pulse);
}

// 设置后轮 PWM 速度
void Motor_SetRearSpeed(uint8_t speed) {
    if (speed > 100) speed = 100;
    
    uint32_t pulse = (uint32_t)(speed * 1000 / 100);
    TIM_SetCompare3(TIM4, pulse);
    TIM_SetCompare4(TIM4, pulse);
}

// 设置速度 (0-100) - 同时控制前后轮
void Motor_SetSpeed(uint8_t speed) {
    if (speed > 100) speed = 100;
    current_speed = speed;
    
    Motor_SetFrontSpeed(speed);
    Motor_SetRearSpeed(speed);
}

// 停止所有电机
void Motor_StopAll(void) {
    Motor_SetSpeed(0);
    set_front_motor_direction(0, 0);
    set_rear_motor_direction(0, 0);
}

// 蓝牙指令解析
void motor_control(char cmd) {
    switch(cmd) {
        case 'F':
            set_front_motor_direction(1, 1);
            set_rear_motor_direction(1, 1);
            Motor_SetSpeed(current_speed);
            break;
        case 'B':
            set_front_motor_direction(2, 2);
            set_rear_motor_direction(2, 2);
            Motor_SetSpeed(current_speed);
            break;
        case 'L':
            set_front_motor_direction(0, 1);
            set_rear_motor_direction(0, 1);
            Motor_SetSpeed(current_speed);
            break;
        case 'R':
            set_front_motor_direction(1, 0);
            set_rear_motor_direction(1, 0);
            Motor_SetSpeed(current_speed);
            break;
        case 'S':
            Motor_StopAll();
            break;
        case 'U': // 加速
            if (current_speed < 100) current_speed += 10;
            Motor_SetSpeed(current_speed);
            break;
        case 'D': // 减速
            if (current_speed > 0) current_speed -= 10;
            Motor_SetSpeed(current_speed);
            break;
        default:
            break;
    }
}
