// motor.h
#ifndef MOTOR_H
#define MOTOR_H

#include "stm32f10x.h"
#include "stm32f10x_conf.h"

// 初始化电机
void Motor_Init(void);

// 蓝牙指令控制
void motor_control(char cmd);

// 设置速度 (0-100)
void Motor_SetSpeed(uint8_t speed);

// 停止所有电机
void Motor_StopAll(void);

// 方向控制（四轮驱动）
void Motor_Forward(uint8_t speed);
void Motor_Backward(uint8_t speed);
void Motor_Left(uint8_t speed);
void Motor_Right(uint8_t speed);

// 新增：前后轮独立控制
void Motor_SetFrontSpeed(uint8_t speed);
void Motor_SetRearSpeed(uint8_t speed);
void Motor_AllStop(void);

#endif
