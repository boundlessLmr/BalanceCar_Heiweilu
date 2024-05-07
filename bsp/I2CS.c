#include "I2CS.h"
void I2CS_Init(void)
{
		    GPIO_InitTypeDef gpio_initstruct;
    __HAL_RCC_GPIOC_CLK_ENABLE();                          /* IO��PBʱ��ʹ�� */
	
    gpio_initstruct.Pin = GPIO_PIN_14|GPIO_PIN_15;                      /* SCK,SDA���� */
    gpio_initstruct.Mode = GPIO_MODE_OUTPUT_PP;            /* ������� */
    gpio_initstruct.Pull = GPIO_PULLUP;                    /* ���� */
    gpio_initstruct.Speed = GPIO_SPEED_FREQ_HIGH;          /* ���� */
    HAL_GPIO_Init(GPIOC, &gpio_initstruct);                /* ��ʼ��SCK,SDA���� */

}
//��ʼ�ź�
void I2C_Start(void)
{
	OLED_SDIN_Set();
	OLED_SCLK_Set();
	OLED_SDIN_Clr();
	OLED_SCLK_Clr();
}

//�����ź�
void I2C_Stop(void)
{
	OLED_SCLK_Set();
	OLED_SDIN_Clr();
	OLED_SDIN_Set();
}

//�ȴ��ź���Ӧ
void I2C_WaitAck(void) //�������źŵĵ�ƽ
{
	OLED_SCLK_Set();
	OLED_SCLK_Clr();
}

//д��һ���ֽ�
void Send_Byte(uint8_t dat)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		OLED_SCLK_Clr();//��ʱ���ź�����Ϊ�͵�ƽ
		if(dat&0x80)//��dat��8λ�����λ����д��
		{
			OLED_SDIN_Set();
    }
		else
		{
			OLED_SDIN_Clr();
    }
		OLED_SCLK_Set();//��ʱ���ź�����Ϊ�ߵ�ƽ
		OLED_SCLK_Clr();//��ʱ���ź�����Ϊ�͵�ƽ
		dat<<=1;
  }
}
