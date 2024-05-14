#include "Grayscale.h"

uint8_t xun[8];

void Read_8PIN(void)
{
	xun[0] = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_2);
	xun[1] = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_1);	
	xun[2] = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0);
	xun[3] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5);		
	xun[4] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_4);
	xun[5] = HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_3);		
	xun[6] = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_15);
	xun[7] = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_12);		
}
