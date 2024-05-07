#ifndef __OLED_H
#define __OLED_H

#include "main.h"
#include "I2CS.h"

#define OLED_CMD  0	//写命令
#define OLED_DATA 1	//写数据


void OLED_WR_Byte(uint8_t dat,uint8_t mode);

void OLED_DisPlay_On(void);
void OLED_DisPlay_Off(void);

void OLED_Refresh_XY(char x, char y, char x1, char y1);
	
void OLED_Refresh(void);

void OLED_Clear(void);

void OLED_Init(void);

void OLED_DrawPoint(uint8_t x,uint8_t y);//画点 
//x:0~127
//y:0~63




void OLED_ClearPoint(uint8_t x,uint8_t y);//清除一个点
//x:0~127
//y:0~63


void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1);




void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1);//显示字符串
//x,y:起点坐标  
//size1:字体大小 
//*chr:字符串起始地址 

uint32_t OLED_Pow(uint8_t m,uint8_t n);


////显示2个数字
////x,y :起点坐标	 
////len :数字的位数
////size:字体大小
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1);

void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length,uint8_t size1);


void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length,uint8_t size1);


void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length,uint8_t size1);


//显示汉字
//x,y:起点坐标
//num:汉字对应的序号
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1);


//num 显示汉字的个数
//space 每一遍显示的间隔

void OLED_ScrollDisplay(uint8_t num,uint8_t space);
	
//x0,y0：起点坐标
//x1,y1：终点坐标
//BMP[]：要写入的图片数组
void OLED_ShowPicture(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t BMP[]);	
	
void OLED_ShowFNum(uint8_t x,uint8_t y,float Fnum,uint8_t size1);

void OLED_Ground(void);

void OLED_Cloud(void);

void OLED_Dino(void);

int OLED_Cactus(unsigned char ver, unsigned char reset);

int OLED_Jump(char reset);

void OLED_WR_BP(uint8_t x,uint8_t y);

void OLED_ShowEnd(void);

void OLED_ShowStart(void);
	
#endif

