#ifndef __I2CS_H
#define __I2CS_H
#include "main.h"

void I2CS_Init(void);

void Send_Byte(uint8_t dat);

void I2C_Start(void);//结束信号

void I2C_Stop(void);//等待信号响应

void I2C_WaitAck(void);//测数据信号的电平

#define OLED_SCLK_Clr() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_RESET)
#define OLED_SCLK_Set() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_14,GPIO_PIN_SET)//SCL

#define OLED_SDIN_Clr() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,GPIO_PIN_RESET)//SDA
#define OLED_SDIN_Set() HAL_GPIO_WritePin(GPIOC,GPIO_PIN_15,GPIO_PIN_SET)

#endif

