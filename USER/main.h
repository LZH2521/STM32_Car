// main.h
#ifndef MAIN_H
#define MAIN_H

#include "stm32f10x.h"
#include "stm32f10x_conf.h"
#include <stdio.h>

// 系统时钟配置
void SystemClock_Config(void);

// 全局变量声明
extern uint8_t rx_data;

#endif
