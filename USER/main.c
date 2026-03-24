// main.c
#include "main.h"
#include "motor.h"
#include <string.h>

// 接收缓冲区
uint8_t rx_data;

// 函数声明
void SystemClock_Config(void);
void MX_GPIO_Init(void);
void MX_USART1_UART_Init(void);
void MX_TIM3_Init(void);
void MX_TIM4_Init(void);

// UART 中断处理函数
void USART1_IRQHandler(void) {
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET) {
        rx_data = USART_ReceiveData(USART1);
        motor_control(rx_data);
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

int main(void) {
    // 系统初始化
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_TIM3_Init();
    MX_TIM4_Init();

    // 初始化电机模块
    Motor_Init();

    // 启动前轮 PWM（TIM3 通道 3 和通道 4）
    TIM_Cmd(TIM3, ENABLE);
    
    // 启动后轮 PWM（TIM4 通道 3 和通道 4）
    TIM_Cmd(TIM4, ENABLE);

    // 启动 UART 接收中断
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    NVIC_EnableIRQ(USART1_IRQn);

    while (1) {
        // 主循环
    }
}

// ==================== 系统配置函数实现 ====================

void SystemClock_Config(void) {
    // 使用 HSE 作为系统时钟源，配置系统时钟为 72MHz
    // 这里使用默认配置，具体根据实际晶振频率调整
    SystemCoreClockUpdate();
}

void MX_GPIO_Init(void) {
    // GPIO 初始化已经在 Motor_Init() 中完成
    // 这里可以添加其他 GPIO 的初始化
}

void MX_USART1_UART_Init(void) {
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // 使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 配置 USART1 引脚 (PA9-TX, PA10-RX)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;  // TX
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10;  // RX
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // 配置 USART1
    USART_InitStructure.USART_BaudRate = 9600;  // 蓝牙模块波特率
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART1, &USART_InitStructure);
    
    // 配置 USART1 中断
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStructure);
    
    // 使能 USART1
    USART_Cmd(USART1, ENABLE);
}

void MX_TIM3_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // 配置 TIM3 PWM 引脚 (PB0-CH3, PB1-CH4)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 配置定时器时基
    TIM_TimeBaseInitStruct.TIM_Prescaler = 71;  // 预分频 72MHz/72 = 1MHz
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 999;    // 周期 1000，PWM 频率 1kHz
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseInitStruct);
    
    // 配置 PWM 通道 3
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;  // 初始占空比为 0
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM3, &TIM_OCInitStruct);
    TIM_OC3PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
    // 配置 PWM 通道 4
    TIM_OC4Init(TIM3, &TIM_OCInitStruct);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
    
    TIM_ARRPreloadConfig(TIM3, ENABLE);
}

void MX_TIM4_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    GPIO_InitTypeDef GPIO_InitStruct;
    
    // 使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    
    // 配置 TIM4 PWM 引脚 (PB8-CH3, PB9-CH4)
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    // 配置定时器时基
    TIM_TimeBaseInitStruct.TIM_Prescaler = 71;  // 预分频 72MHz/72 = 1MHz
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 999;    // 周期 1000，PWM 频率 1kHz
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseInitStruct);
    
    // 配置 PWM 通道 3
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_Pulse = 0;  // 初始占空比为 0
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OC3Init(TIM4, &TIM_OCInitStruct);
    TIM_OC3PreloadConfig(TIM4, TIM_OCPreload_Enable);
    
    // 配置 PWM 通道 4
    TIM_OC4Init(TIM4, &TIM_OCInitStruct);
    TIM_OC4PreloadConfig(TIM4, TIM_OCPreload_Enable);
    
    TIM_ARRPreloadConfig(TIM4, ENABLE);
}
