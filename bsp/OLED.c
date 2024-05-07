#include "OLED.h"
#include "main.h"
#include "OLED_Font.h"
#include <stdlib.h>
#include "bmp.h"
#include "stdio.h"

uint8_t OLED_GRAM[144][8];

void OLED_WR_Byte(uint8_t dat,uint8_t mode)
{
	I2C_Start();
	Send_Byte(0x78);
	I2C_WaitAck();
	if(mode){Send_Byte(0x40);}
  else{Send_Byte(0x00);}
	I2C_WaitAck();
	Send_Byte(dat);
	I2C_WaitAck();
	I2C_Stop();
}

//void OLED_WR_Byte(uint8_t dat,uint8_t mode)
//{
//	if (mode == OLED_CMD)
//	{
//		Write_IIC_Command(dat);
//	}
//	else  
//	{
//		Write_IIC_Data(dat);
//	}
//}

void OLED_DisPlay_On(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//电荷泵使能
	OLED_WR_Byte(0x14,OLED_CMD);//开启电荷泵
	OLED_WR_Byte(0xAF,OLED_CMD);//点亮屏幕
}

//关闭OLED显示 
void OLED_DisPlay_Off(void)
{
	OLED_WR_Byte(0x8D,OLED_CMD);//电荷泵使能
	OLED_WR_Byte(0x10,OLED_CMD);//关闭电荷泵
	OLED_WR_Byte(0xAF,OLED_CMD);//关闭屏幕
}

//更新显存到OLED	

void OLED_Refresh_XY(char x, char y, char x1, char y1)
{
	uint8_t i,n;
	for(i=y;i<y1;i++)
	{
		
	   for(n=x;n<x1;n++)
			{
				OLED_WR_BP(n,i);
				OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA);
			}
  }
}
void OLED_Refresh(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)
	{
	   OLED_WR_Byte(0xb0+i,OLED_CMD); //设置行起始地址
	   OLED_WR_Byte(0x00,OLED_CMD);   //设置低列起始地址
	   OLED_WR_Byte(0x10,OLED_CMD);   //设置高列起始地址
	   for(n=0;n<128;n++)
		 OLED_WR_Byte(OLED_GRAM[n][i],OLED_DATA);
  }
}

//清屏函数
void OLED_Clear(void)
{
	uint8_t i,n;
	for(i=0;i<8;i++)
	{
	   for(n=0;n<128;n++)
			{
			 OLED_GRAM[n][i]=0;//清除所有数据
			}
  }
	OLED_Refresh();//更新显示
}

//画点 
//x:0~127
//y:0~63
void OLED_DrawPoint(uint8_t x,uint8_t y)
{
	uint8_t i,m,n;
	i=y/8;
	m=y%8;
	n=1<<m;
	OLED_GRAM[x][i]|=n;
}

//清除一个点
//x:0~127
//y:0~63
void OLED_ClearPoint(uint8_t x,uint8_t y)
{
	uint8_t i,m,n;
	i=y/8;
	m=y%8;
	n=1<<m;
	OLED_GRAM[x][i]=~OLED_GRAM[x][i];
	OLED_GRAM[x][i]|=n;
	OLED_GRAM[x][i]=~OLED_GRAM[x][i];
}

//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//size:选择字体 12/16/24



//char reverse_bit(char num)//八位二进制数倒序
//{
//	int i;
//	int bit;
//	char new_num = 0;
//	for (i = 0; i < 8; i++)
//	{
//		bit = num & 1;            //取出最后一位
//		new_num <<= 1;            //新数左移
//		new_num =new_num | bit;   //把刚取出的一位加到新数
//		num >>= 1;                //原数右移，准备取第二位
//	}
//	return new_num;
//}

void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t size1)
{
	uint8_t i,m,temp,size2,chr1;
	uint8_t y0=y;
	size2=(size1/8+((size1%8)?1:0))*(size1/2);  //得到字体一个字符对应点阵集所占的字节数
	chr1=chr-' ';  //计算偏移后的值
	for(i=0;i<size2;i++)
	{
		if(size1==12)
        {temp=asc2_1206[chr1][i];} //调用1206字体
		else if(size1==16)
        {temp=asc2_1608[chr1][i];} //调用1608字体
		else if(size1==24)
        {temp=asc2_2412[chr1][i];} //调用2412字体
		else return;
				for(m=0;m<8;m++)           //写入数据
				{
					if(temp&0x80)OLED_DrawPoint(x,y);
					else OLED_ClearPoint(x,y);
					temp<<=1;
					y++;
					if((y-y0)==size1)
					{
						y=y0;
						x++;
						break;
          }
				}
  }
}

//显示字符串
//x,y:起点坐标  
//size1:字体大小 
//*chr:字符串起始地址 
void OLED_ShowString(uint8_t x,uint8_t y,uint8_t *chr,uint8_t size1)
{
	while((*chr>=' ')&&(*chr<='~'))//判断是不是非法字符!
	{
		OLED_ShowChar(x,y,*chr,size1);
		x+=size1/2;
		if(x>128-size1)  //换行
		{
			x=0;
			y+=2;
    }
		chr++;
  }
}

uint32_t OLED_Pow(uint8_t m,uint8_t n)
{
	uint32_t result=1;
	while(n--)
	{
	  result*=m;
	}
	return result;
}


////显示2个数字
////x,y :起点坐标	 
////len :数字的位数
////size:字体大小
void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size1)
{
	uint8_t t,temp;
	for(t=0;t<len;t++)
	{
		temp=(num/OLED_Pow(10,len-t-1))%10;
			if(temp==0)
			{
				OLED_ShowChar(x+(size1/2)*t,y,'0',size1);
      }
			else 
			{
			  OLED_ShowChar(x+(size1/2)*t,y,temp+'0',size1);
			}
  }
}



void OLED_ShowSignedNum(uint8_t x, uint8_t y, int32_t Number, uint8_t Length,uint8_t size1)
{
	uint8_t i;
	uint32_t Number1;
	if (Number >= 0)
	{
		OLED_ShowChar(x, y, '+',size1);
		Number1 = Number;
	}
	else
	{
		OLED_ShowChar(x, y, '-',size1);
		Number1 = -Number;
	}
	for (i = 0; i < Length; i++)							
	{		
		OLED_ShowChar(x+size1/2*(i+1), y , Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0',size1);
	}
}


void OLED_ShowHexNum(uint8_t x, uint8_t y, uint32_t Number, uint8_t Length,uint8_t size1)
{
	uint8_t i, SingleNumber;
	for (i = 0; i < Length; i++)							
	{
		SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
		if (SingleNumber < 10)
		{
			OLED_ShowChar(x+size1/2*(i+1), y , SingleNumber + '0',size1);
		}
		else
		{
			OLED_ShowChar(x+size1/2*(i+1), y , SingleNumber - 10 + 'A',size1);   
		}
	}
}


void OLED_ShowBinNum(uint8_t x, uint8_t y, uint32_t Number, uint8_t Length,uint8_t size1)
{
	uint8_t i;
	for (i = 0; i < Length; i++)							
	{
		OLED_ShowChar(x+size1/2*(i+1), y, Number / OLED_Pow(2, Length - i - 1) % 2 + '0',size1);
	}
}

//显示汉字
//x,y:起点坐标
//num:汉字对应的序号
void OLED_ShowChinese(uint8_t x,uint8_t y,uint8_t num,uint8_t size1)
{
	uint8_t i,m,n=0,temp,chr1;
	uint8_t x0=x,y0=y;
	uint8_t size3=size1/8;//i++return Original value
	while(size3--)
	{
		chr1=num*size1/8+n;
		n++;
			for(i=0;i<size1;i++)
			{
				if(size1==16)
						{temp=Hzk1[chr1][i];}//调用16*16字体
				else if(size1==24)
						{temp=Hzk2[chr1][i];}//调用24*24字体
				else if(size1==32)       
						{temp=Hzk3[chr1][i];}//调用32*32字体
				else if(size1==64)
						{temp=Hzk4[chr1][i];}//调用64*64字体
				else return;
							
						for(m=0;m<8;m++)
							{
								if(temp&0x01)OLED_DrawPoint(x,y);
								else OLED_ClearPoint(x,y);
								temp>>=1;
								y++;
							}
							x++;
							if((x-x0)==size1)
							{x=x0;y0=y0+8;}
							y=y0;
			 }
	}
}

//num 显示汉字的个数
//space 每一遍显示的间隔


void OLED_ScrollDisplay(uint8_t num,uint8_t space)
{
	uint8_t i,n,t=0,m=0,r;
	while(1)
	{
		if(m==0)
		{
	    OLED_ShowChinese(128,24,t,16); //写入一个汉字保存在OLED_GRAM[][]数组中
			t++;
		}
		if(t==num)
			{
				for(r=0;r<16*space;r++)      //显示间隔
				 {
					for(i=0;i<144;i++)
						{
							for(n=0;n<8;n++)
							{
								OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
							}
						}
           OLED_Refresh();
				 }
        t=0;
      }
		m++;
		if(m==16){m=0;}
		for(i=0;i<144;i++)   //实现左移
		{
			for(n=0;n<8;n++)
			{
				OLED_GRAM[i-1][n]=OLED_GRAM[i][n];
			}
		}
		OLED_Refresh();
	}
}

//配置写入数据的起始位置
void OLED_WR_BP(uint8_t x,uint8_t y)
{
	OLED_WR_Byte(0xb0+y,OLED_CMD);//设置行起始地址
	OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	OLED_WR_Byte((x&0x0f)|0x00,OLED_CMD);
}

//x0,y0：起点坐标
//x1,y1：终点坐标
//BMP[]：要写入的图片数组
void OLED_ShowPicture(uint8_t x0,uint8_t y0,uint8_t x1,uint8_t y1,uint8_t BMP[])
{
	uint32_t j=0;
	uint8_t x=0,y=0;
	for(y=y0;y<=y1;y++)
	 {
		 OLED_WR_BP(x0,y);
		 for(x=x0;x<=x1;x++)
		 {
			 OLED_WR_Byte(BMP[j],OLED_DATA);
			 j++;
     }
	 }
}

void OLED_Ground()
{
	static unsigned int pos = 0;
	unsigned char speed = 5;
	unsigned int ground_length = sizeof(ground_bmp);
	uint8_t x=0,y=0;


		 OLED_WR_BP(0,7);
		 for(x=0;x<128;x++)
		 {
			 OLED_WR_Byte(ground_bmp[(x+pos)%ground_length],OLED_DATA);
     }
			pos = pos + speed;
		 if(pos>ground_length) pos = 0;

}

void OLED_Cloud()
{
	static int pos = 128;
	static char height = 0;
	char speed = 3;
	int x;
	int start_x = 0;
	int length = sizeof(Cloud_bmp);
	unsigned char byte;
	
	if(pos+length<=-speed)
	{
		pos = 128;
		height = rand()%3;
	}
	
	if(pos<0)
	{
		start_x = -pos;
		OLED_WR_BP(0,1+height);
	}
	else
	{
		OLED_WR_BP(pos,1+height);
	}
	
	for(x=start_x;x < length + speed;x++)
	{
		if(pos + x > 127)	break;
		if (x < length) byte = Cloud_bmp[x];
		else byte = 0x0;
		
		OLED_WR_Byte(byte,OLED_DATA);
		
	}
	
	pos = pos - speed;
}

void oled_drawbmp_block_clear(int bx, int by, int clear_size)
{
	 unsigned int i;
	OLED_WR_BP(bx, by);
	
	for (i = 0; i < clear_size; i++)
	{
		if (bx + i>128) break;
		OLED_WR_Byte (0x0, OLED_DATA);
	}
	
}

void OLED_Dino()
{
	static unsigned char dino_dir = 0;
	unsigned int j = 0;
	unsigned char x, y;
	unsigned char byte;
	
	dino_dir++;
	dino_dir = dino_dir%2;
	for (y = 0; y < 2; y++)
	{
		OLED_WR_BP(16,6+y);
		for(x = 0; x < 16; x++)
		{
			j = y*16 + x;
			byte = Dino_bmp[dino_dir][j];
			
			OLED_WR_Byte(byte, OLED_DATA);
		}
	}
	
}

int OLED_Cactus(unsigned char ver, unsigned char reset)
{
	static int pos = 128;
	char speed = 5;
	int start_x = 0;
	int length = 0;
	
	unsigned int i = 0, j = 0;
	unsigned char x, y;
	unsigned char byte;
	
	
	if (reset == 1)
	{
		pos = 128;
		oled_drawbmp_block_clear(0, 6, speed);
		return 128;
	}
	
	if (ver == 0) length = 8;
	else if (ver == 1) length = 16;
	else if (ver == 2 || ver == 3) length = 24;
	
	
	for(y=0;y<=1;y++)
	{
		if(pos<0)
		{
			start_x = -pos;
			OLED_WR_BP(0,y+6);
		}
		else
		{
			OLED_WR_BP(pos,y+6);
		}
	
	
		for(x=start_x;x < length + speed;x++)
		{
			if(pos + x > 127)	break;

			j = y*length + x;
			if (ver == 0) byte = C1_bmp[j];
			else if (ver == 1) byte = C2_bmp[j];
			else if (ver == 2) byte = C3_bmp[j];			
			else byte = C4_bmp[j];
			
			OLED_WR_Byte(byte,OLED_DATA);
			
		}
	}
	oled_drawbmp_block_clear(pos + length, 6, speed);
	pos = pos - speed;
	return pos + speed;

}



int OLED_Jump(char reset)
{
	char speed_arr[] = {1,1,3,3,4,4,5,6,7};
	static char speed_idx = sizeof(speed_arr)-1;
	static int height = 0;
	static char dir = 0;
	
	unsigned int i=0, j=0;
	unsigned char x, y;
	char offset = 0;
	unsigned byte;
	
	if(reset == 1)
	{
		height = 0;
		dir = 0;
		speed_idx = sizeof(speed_arr)-1;
		return 0;
	}
	if (dir==0)
	{
		height += speed_arr[speed_idx];
		speed_idx --;
		if (speed_idx<0) speed_idx = 0;
		
	}
	if (dir==1)
	{
		height -= speed_arr[speed_idx];
		speed_idx ++;
		if (speed_idx>sizeof(speed_arr)-1)	speed_idx = sizeof(speed_arr)-1;
	}
	if(height >= 31)
	{
		dir = 1;
		height = 31;
	}
	if(height <= 0)
	{
		dir = 0;
		height = 0;
	}
	if(height <= 7) offset = 0;
	else if(height <=15) offset = 1;
	else if(height <=23) offset = 2;
	else if(height <=31) offset = 3;
	else offset = 4;
	
	for(y=0; y<3; y++)
	{
		OLED_WR_BP(16, 5 - offset + y);
		
		for(x = 0; x < 16; x++)
		{
			j = y*16 + x;
			byte = JUMP_bmp[height%8][j];
			
			OLED_WR_Byte (byte,OLED_DATA);
		}
	}
	if (dir == 0) oled_drawbmp_block_clear(16, 8- offset, 16);
	if (dir == 1) oled_drawbmp_block_clear(16, 4- offset, 16);
	return height;
	
	
}

void OLED_ShowEnd(void)
{
	{
	uint8_t x=0,y=0;

		for(y=0; y<8; y++)
		{
			OLED_WR_BP(0,y);
			for(x=0;x<128;x++)
			{
				OLED_WR_Byte(GO_bmp[x+128*y],OLED_DATA);
			}
		}
	
	}
}

void OLED_ShowStart(void)
{
	{
	uint8_t x=0,y=0;

		for(y=0; y<8; y++)
		{
			OLED_WR_BP(0,y);
			for(x=0;x<128;x++)
			{
				OLED_WR_Byte(START_bmp[x+128*y],OLED_DATA);
			}
		}
	
	}
}
void OLED_ShowFNum(uint8_t x,uint8_t y,float Fnum,uint8_t size1)
{
	uint8_t Data[]= " ";                             //创建目标数组，用来存放转换后的字符数据 
  sprintf(Data,"%.3f",Fnum);                       //保留小数点后3位小数，打印到Data数组中
	OLED_ShowString(x,y,Data,size1);
}
                        
void OLED_Init(void)
{

	I2CS_Init();
	
	HAL_Delay(1000);

	
	OLED_WR_Byte(0xAE,OLED_CMD);//--turn off oled panel
	OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
	OLED_WR_Byte(0x81,OLED_CMD);//--set contrast control register
	OLED_WR_Byte(0xCF,OLED_CMD);// Set SEG Output Current Brightness
	OLED_WR_Byte(0xA1,OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	OLED_WR_Byte(0xC8,OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	OLED_WR_Byte(0xA6,OLED_CMD);//--set normal display
	OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3f,OLED_CMD);//--1/64 duty
	OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
	OLED_WR_Byte(0x00,OLED_CMD);//-not offset
	OLED_WR_Byte(0xd5,OLED_CMD);//--set display clock divide ratio/oscillator frequency
	OLED_WR_Byte(0x80,OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
	OLED_WR_Byte(0xD9,OLED_CMD);//--set pre-charge period
	OLED_WR_Byte(0xF1,OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
	OLED_WR_Byte(0xDA,OLED_CMD);//--set com pins hardware configuration
	OLED_WR_Byte(0x12,OLED_CMD);
	OLED_WR_Byte(0xDB,OLED_CMD);//--set vcomh
	OLED_WR_Byte(0x40,OLED_CMD);//Set VCOM Deselect Level
	OLED_WR_Byte(0x20,OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
	OLED_WR_Byte(0x02,OLED_CMD);//
	OLED_WR_Byte(0x8D,OLED_CMD);//--set Charge Pump enable/disable
	OLED_WR_Byte(0x14,OLED_CMD);//--set(0x10) disable
	OLED_WR_Byte(0xA4,OLED_CMD);// Disable Entire Display On (0xa4/0xa5)
	OLED_WR_Byte(0xA6,OLED_CMD);// Disable Inverse Display On (0xa6/a7) 
	OLED_WR_Byte(0xAF,OLED_CMD);
	OLED_Clear();

}

