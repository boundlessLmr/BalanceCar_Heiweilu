#include "key.h"

struct Bkeys bkey[3]={0,0,0,0};

uint8_t key_read()
{
	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_4)==0)      return 1;
	else if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_5)==1) return 2;
	else return 0;
}


void key_serv()
{
	uint8_t key_sta=key_read();
	if(key_sta!=0)
	{
		bkey[key_sta].age++;
		if(bkey[key_sta].age>1)bkey[key_sta].short_flag=1;
	}
	else
	{
		for(int i=0;i<3;i++) bkey[i].age=0;
	}
}
/*长按快速增加功能*/
//void key_serv_long()
//{
//	uint8_t key_sta=key_read();
//	if(key_sta!=0)
//	{
//		bkey[key_sta].age++;
//		if(bkey[key_sta].age>1) bkey[key_sta].press=1;
//	}
//	else
//	{
//		for(int i=0;i<3;i++) 
//		{
//			if(bkey[i].press==1&&bkey[i].long_flag==0)
//				bkey[i].short_flag=1;
//			bkey[i].age=0;
//			bkey[i].press=0;
//			bkey[i].long_flag=0;
//		}
//	}
//	if(bkey[key_sta].age>199) bkey[key_sta].long_flag=1;
//}
/*长按后抬起执行*/
void key_serv_long()
{
	uint8_t key_sta=key_read();
	if(key_sta!=0)
	{
		bkey[key_sta].age++;
		if(bkey[key_sta].age>1) bkey[key_sta].press=1;
	}
	else
	{
		for(int i=0;i<3;i++) 
		{
			if(bkey[i].age>69) bkey[i].long_flag=1;
			if(bkey[i].press==1&&bkey[i].long_flag==0)
				bkey[i].short_flag=1;
			bkey[i].age=0;
			bkey[i].press=0;
		}
	}
}
//下列按键经过测试有问题
//void key_serv_double()
//{
//	uint8_t key_sta=key_read();
//	if(key_sta!=0)
//	{
//		bkey[key_sta].age++;
//		if(bkey[key_sta].age>1) bkey[key_sta].press=1;
//	}
//	else
//	{
//		for(int i=0;i<3;i++) 
//		{
//			if(bkey[i].double_ageEN==1&&bkey[i].press==1)
//			{
//				bkey[i].double_flag=1;
//				bkey[i].double_ageEN=0;
//				bkey[i].press=0;
//			}
//			if(bkey[i].press&&bkey[i].long_flag==0)
//			{
//				bkey[i].double_ageEN=1;
//			}
//			if(bkey[i].double_ageEN==1)bkey[i].double_age++;
//			if(bkey[i].double_ageEN==1&&bkey[i].double_age>20)
//			{
//				bkey[i].short_flag=1;
//				bkey[i].double_ageEN=0;
//				bkey[i].double_age=0;
//			}
//			bkey[i].age=0;
//			bkey[i].press=0;
//			bkey[i].long_flag=0;
//		}
//	}
//	if(bkey[key_sta].age>69) bkey[key_sta].long_flag=1;
//}
