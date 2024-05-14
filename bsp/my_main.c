#include "my_main.h"

//-----全局变量---
extern struct Bkeys bkey[];

__IO uint32_t uwTick_UART = 0;
int Vertical_out,Velocity_out,Turn_out; // 直立环&速度环&转向环的输出变量
int Encoder_Left,Encoder_Right,Balance_PWM;
int Moto1,Moto2;
int PWM_out,Turn_Pwm; //闭环输出
		
u8 AIN1,AIN2;//控制电机正反转
u8 BIN1,BIN2;
u8 Moto_Flag=0;//电机控制标志
u8 Start_Flag=0;//系统初始化完成标志

float Pitch,Roll,Yaw;	        // Pitch：俯仰角，Roll：横滚角，Yaw：偏航角
short gyrox,gyroy,gyroz;        // 角速度
short aacx,aacy,aacz;           // 加速度
float Med_Angle=0;//机械中值，能使得小车真正平衡住的角度 



/*UART 相关*/
char Usart3_RX_Buf[50];  //串口接收数据缓存buf
unsigned char Usart3_TX_Buf[50];
char temp[50]; 



u8 Flag_forward = 0, Flag_retreat = 0, Flag_left = 0, Flag_right = 0;//蓝牙遥控标志


//----调参选择-----// 
#define TIAO_VERp          0
#define TIAO_VERd          1
#define TIAO_VELp          2

uint8_t Tiao_Who = TIAO_VELp;
float test_num_VERp = 10;
float test_num_VERd = 1;
float test_num_VELp = 0.1;


//----直立环-----// 
float Bias, Vertical_Kp= 300,Vertical_Kd= 1.38;
float Med = 0;

//直立环Kp,Kd均为正值 (480  2.3) * 0.6 =  288    1.38  
//直立环Kp,Kd均为正值 (480  2.2) * 0.6 =  288    1.32  !
//直立环Kp,Kd均为正值 (600  2.4) * 0.6 =  360    1.44  X
//直立环Kp,Kd均为正值 (650  2.5) * 0.6 =  390    1.5   X
//float Bias, Vertical_Kp= 0,Vertical_Kd= 0;	


//----速度环-----
 float Velocity,Encoder_Err,Encoder,Encoder_last,Movement=0; //速度，误差，编码器
 float Encoder_Integral;	 //编码器数值积分
 #define kp_val 5.4
 float kp=kp_val,ki=kp_val/200;
 
 
//----循迹-----
uint8_t Black_miss_flag = 0;
 
 
//----子函数声明-----
void UART_Proc(void);
//void	go();
//void	stop();
//void	backward();
int Vertical(float Angle,float Gyro_Y);			 				//直立环
int GetVelocity(int Encoder_left,int Encoder_right);				//速度环
int Turn(float gyro);	//转向环
u8 Stop(signed int angle);    		//倒下保护
void Limit(int *motoA,int *motoB);  //电机速度限幅
void Set_Pwm(int Moto1,int Moto2);	//控制PWM最终输出
void jizhi(void);
void forward(void);
void right(void);
void left(void);
void back(void);
void BigRight(void);
void BigLeft(void);
void XunJi(void);

u8 test111 = 0;
uint8_t page = 0;

void key_proc(void)
{
/*短按*/	
	if(bkey[2].short_flag)
	{
		switch(Tiao_Who)
		{
			case TIAO_VERp:Vertical_Kp-= test_num_VERp;break;	
			case TIAO_VERd:Vertical_Kd-= test_num_VERd;break;
			case TIAO_VELp:kp					-= test_num_VELp;ki = kp/200;break;
		}
		
		bkey[2].short_flag = 0;
	}
	if(bkey[1].short_flag)
	{	
		switch(Tiao_Who)
		{
			case TIAO_VERp:Vertical_Kp+= test_num_VERp;break;	
			case TIAO_VERd:Vertical_Kd+= test_num_VERd;break;
			case TIAO_VELp:kp					+= test_num_VELp;ki = kp/200;break;
		}
		
		bkey[1].short_flag = 0;
	}		
/*长按*/	
	if(bkey[2].long_flag)
	{
//		Set_Pwm(0,0);
//		Start_Flag = !Start_Flag;
//		if(Start_Flag)
//		{
//			OLED_ShowString(103,00,"on",12);     
//		}
//		else
//		{
//			OLED_ShowString(103,00,"of",12);    		
//		}
Tiao_Who++;if(Tiao_Who== 3)Tiao_Who = 0;	
OLED_Clear();		
		
		bkey[2].long_flag = 0;
	}
	if(bkey[1].long_flag)
	{
		switch(Tiao_Who)
		{
			case TIAO_VERp:if(test_num_VERp == 100)test_num_VERp = 10;else test_num_VERp = 100;break;
			case TIAO_VERd:if(test_num_VERd == 1) test_num_VERd = 0.1;else test_num_VERd = 1;break;
			case TIAO_VELp:if(test_num_VELp == 0.01)test_num_VELp = 0.1;else test_num_VELp = 0.01;break;	
		}		
		
		bkey[1].long_flag = 0;		
	}	
	
}
/**************************************************************************
函数功能：通过串口测试数据
**************************************************************************/
void UART_Proc()
{
  if((uwTick - uwTick_UART) < 1000) return;//return;结束函数
	uwTick_UART = uwTick;
//	
//	sprintf("kP:%.2f,kD:%.2f,flag:%d,num:%.2f\r\n",Vertical_Kp,Vertical_Kd,Start_Flag,test_num);
//	HAL_UART_Transmit(&huart3,(uint8_t *)Usart3_TX_Buf,strlen(Usart3_TX_Buf),50);
//	sprintf("-------------------------------------------\r\n");	
//  HAL_UART_Transmit(&huart3,(uint8_t *)Usart3_TX_Buf,strlen(Usart3_TX_Buf),50);
//  if ((mpu_dmp_get_data(&Pitch, &Roll, &Yaw) == 0) && (MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz) == 0)) //DMP读取欧拉角，digital motion processor数字运动处理器
//   {
//		//俯仰角，横滚角，偏航角
//		 sprintf("pitch=%.3f   roll=%.3f   yaw=%.3f \r\n", Pitch, Roll, Yaw);   //串口打印欧拉角
//     HAL_UART_Transmit(&huart3,(uint8_t *)Usart3_TX_Buf,strlen(Usart3_TX_Buf),50);
//  	 sprintf("gyrox=%d   gyroy=%d  gyroz=%d \r\n", gyrox, gyroy, gyroz);	// 角速度
//		 HAL_UART_Transmit(&huart3,(uint8_t *)Usart3_TX_Buf,strlen(Usart3_TX_Buf),50);
//		 sprintf("-------------------------------------------\r\n");
//     HAL_UART_Transmit(&huart3,(uint8_t *)Usart3_TX_Buf,strlen(Usart3_TX_Buf),50);
//			
//	 }

//	 sprintf("Vertical_out=%d",Vertical_out);
//	 HAL_UART_Transmit(&huart3,(uint8_t *)Usart3_TX_Buf,strlen(Usart3_TX_Buf),50);
//	sprintf("%d,1\r\n",PWM_out);
//	HAL_UART_Transmit(&huart3,(uint8_t *)Usart3_TX_Buf,strlen(Usart3_TX_Buf),50);
//	sprintf("%d\n",test111);
//	HAL_UART_Transmit(&huart3,(uint8_t *)Usart3_TX_Buf,strlen(Usart3_TX_Buf),50);
//	sprintf(Usart3_TX_Buf,"tx ok");
//	HAL_UART_Transmit(&huart3,(uint8_t *)Usart3_TX_Buf,strlen(Usart3_TX_Buf),50);
}

void oled_proc()
{//行x:0~127    列y:0~63
	if(page == 0)
	{
		switch(Tiao_Who)
		{

			case TIAO_VERp:
											//============= 第一行 调直立环=======================//	
												OLED_ShowString(00,00,"P:",12);                   
												OLED_ShowFNum(24,00,Vertical_Kp,12);
											//=============  第二行=======================//	
												OLED_ShowString(00,12,"D:",12);                   
												OLED_ShowFNum(24,12,Vertical_Kd,12);
											//=============  第三行=======================//	
												OLED_ShowString(00,24,"ro:",12);
												OLED_ShowFNum(36,24,Roll,12);
											//=============  第四行=======================//	
												OLED_ShowString(00,36,"gx:",12);
												OLED_ShowFNum(36,36,gyrox,12);	
											//=============  第5行=======================//	
												OLED_ShowString(00,48,"vp:",12);
												OLED_ShowFNum(36,48,Vertical_out,12);			
											
												OLED_Refresh();	break;					
			
			case TIAO_VERd:
											//============= 第一行 调直立环=======================//	
												OLED_ShowString(00,00,"P:",12);                   
												OLED_ShowFNum(24,00,Vertical_Kp,12);
											//=============  第二行=======================//	
												OLED_ShowString(00,12,"D:",12);                   
												OLED_ShowFNum(24,12,Vertical_Kd,12);
											//=============  第三行=======================//	
												OLED_ShowString(00,24,"ro:",12);
												OLED_ShowFNum(36,24,Roll,12);
											//=============  第四行=======================//	
												OLED_ShowString(00,36,"gx:",12);
												OLED_ShowFNum(36,36,gyrox,12);	
											//=============  第5行=======================//	
												OLED_ShowString(00,48,"vp:",12);
												OLED_ShowFNum(36,48,Vertical_out,12);			
											
												OLED_Refresh();	break;					
						
			case TIAO_VELp:
											//============= 第一行 调速度环=======================//	
												OLED_ShowString(00,00,"P:",12);                   
												OLED_ShowFNum(24,00,kp,12);
											//=============  第二行=======================//	
												OLED_ShowString(00,12,"i:",12);                   
												OLED_ShowFNum(24,12,ki,12);
											//=============  第三行=======================//	
												OLED_ShowString(00,24,"le:",12);
												OLED_ShowSignedNum(24,24,Encoder_Left,4,12);
												
									//			OLED_ShowString(00,24,"er:",12);
									//		  OLED_ShowSignedNum(24,24,Encoder_Err,4,12);			
											//=============  第四行=======================//	
//												OLED_ShowString(00,36,"ri:",12);
//												OLED_ShowSignedNum(24,36,Encoder_Right,4,12);
												OLED_ShowString(00,36,"tt:",12);
											  OLED_ShowSignedNum(24,36,test111,4,12);			
											//=============  第5行=======================//		
												OLED_ShowFNum(00,48,Roll,12);	  OLED_ShowFNum(60,48,PWM_out,12);	 
											// 没有第6行
									
												OLED_Refresh();break;									
		}
	}
	else if(page == 1)
	{
	
	}
}

void set_up(void)
{
  //启动TIM初始化

	
   HAL_TIM_Base_Start_IT(&htim2);//每10ms触发一次中断 
  
   HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);//PWM
	 HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);	
  
	 HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_1);//编码器
	 HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_2);
	 HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_1);
	 HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_2);	

	HAL_UARTEx_ReceiveToIdle_IT(&huart3, (uint8_t *)Usart3_RX_Buf,50);
	
		OLED_Init();	
  	OLED_Clear();	
	
	OLED_ShowString(00,00,"mpu_dmp_init...",12);  
	OLED_Refresh();	
  //-----DMP初始化----
  while(mpu_dmp_init())//成功返回0，否则返回1
  {
		uint8_t res;
		res = mpu_dmp_init();
		HAL_Delay(300);
		OLED_ShowNum(00,12,res,1,12);
		OLED_Refresh();	
  }
  	OLED_Clear();	
		OLED_ShowString(103,00,"on",12);  
 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET); //打开PC13 LED
 Start_Flag = 1; //标志系统初始化成功

//	Set_Pwm(1200,1200);
//jizhi();
//forward();
//BigRight();
	
//left();
//BigRight();
//BigLeft();		  	
}

void loop(void)
{
//forward();
	UART_Proc();
	key_proc();
	oled_proc();
//	XunJi();
}

/**************************************************************************
函数功能：10ms定时器中断，主控制函数
**************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

  if(htim->Instance == TIM2)//TIM2设置为10ms中断
  {
		// 1.采集编码器数据&MPU6050角度信息
   // 电机是相对安装，刚好相差180度，为了编码器输出极性一致，就需要对其中一个取反  	
	  mpu_dmp_get_data(&Pitch,&Roll,&Yaw);	    // 读取角度
    MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);  // 读取角速度
    MPU_Get_Accelerometer(&aacx,&aacy,&aacz); // 读取加速度

	  Encoder_Left  =   Read_Encoder(3);		//读取编码器测量的电机转速
		Encoder_Right =   Read_Encoder(4);		

  // 2.将数据压入闭环控制中，计算出控制输出量
	  Velocity_out = GetVelocity(Encoder_Left,Encoder_Right);// 速度环输出误差  
	  Vertical_out = Vertical(Roll,gyrox);// 直立环输出PWM
//		
	  // -------------蓝牙控制--------------
//	  if(1==Flag_left||1==Flag_right)    
//			Turn_Pwm =Turn(Encoder_Left,Encoder_Right,gyroz+33);//出现左右转标志才进入转向环闭环控制
//	  else   
//			Turn_Pwm=-0.5*(gyroz+33); //保持走直线
//	  
//	  
		//------------最终输出----------------
	  PWM_out= Vertical_out+Velocity_out;
	  // 3.把控制输出量加载到电机上，完成最终控制
		Moto1 = PWM_out+Turn_Pwm; // 左电机
      Moto2 = PWM_out-Turn_Pwm; // 右电机  
      Limit(&Moto1,&Moto1);     // PWM限幅 
		  Set_Pwm(Moto1,Moto2);        // 加载到电机上 
  }
	key_serv_long();
}

/*************************************************************************** 
直立环PD控制器：Kp*Ek+Kd*Ek_D
入口：Med:机械中值(期望角度)，Angle:真实角度，gyro_Y:真实角速度
出口：PMW数值
******************************************************************************/
int Vertical(float Angle,float Gyro_Y) 
{

	Bias = Angle-Med;//直立偏差 //angle = -0.23就平衡了
	PWM_out = Vertical_Kp*Bias+Vertical_Kd*(Gyro_Y-0);	
	return PWM_out;	
} 


/********************************************************************* 
速度环PI控制器：Kp*Ek+Ki*Ek_S(Ek_S：偏差的积分)
入口：左右编码器测到的数值
出口：                   
**********************************************************************/
int GetVelocity(int Encoder_left,int Encoder_right)	
{
	
	//--------------蓝牙控制----------------
	if(1==Flag_forward) 		    Movement = -2000;
	else if(1==Flag_retreat)    Movement = 2000;
	else							    Movement = 0;
	
	
	// 1.计算速度偏差 	
	Encoder_Err = ((Encoder_Left+Encoder_Right)-0);	//速度目标值//Encoder_Left+Encoder_Right 最大是 160  7200 /（160 * 50%） = 90
 
	// 2.对速度偏差进行--低通滤波--
  // low_out = (1-a)*Ek+a*low_out_last	
	Encoder = Encoder_Err*0.3 + Encoder_last*0.7;// 使得波形更加平滑
	Encoder_last = Encoder; 							// 防止速度过大影响直立环的正常工作

	// 3.对速度偏差积分出位移,遥控的速度通过积分融入速度控制器，减缓速度突变对直立控制的影响	
  Encoder_Integral += Encoder-Movement;	


	
	

	// 4.积分限幅	
	if(Encoder_Integral>3000)  	Encoder_Integral=3000;   
	if(Encoder_Integral<-3000)	   Encoder_Integral=-3000;           	

	if(Moto_Flag == 1||Start_Flag ==0) 			Encoder_Integral=0;     		//===电机关闭后或者复位清除积分
    //5.速度环控制输出	
  Velocity=Encoder*kp+Encoder_Integral*ki;

	
	
	return Velocity;



}


/*********************************************************************
转向环：系数*Z轴角速度+系数*遥控数据
入口：左右电机编码器测得数值，Z轴角速度
**********************************************************************/
int Turn(float gyro)
{
   float Turn_Target,Turn_PWM,Bias,Encoder_temp,Turn_Convert=70,Turn_Count; 
   float Turn_Amplitude=100,Turn_Kp=10,Turn_Kd=0;  
	
	// --- 蓝牙控制 ---
	if(1==Flag_left) 			Turn_Target+=Turn_Convert;//左转标志
	else if(1==Flag_right)  Turn_Target-=Turn_Convert; //右转标志
	 else 						 Turn_Target=0;
	
	 if(Turn_Target>Turn_Amplitude)  Turn_Target=Turn_Amplitude; //转速限幅    
    if(Turn_Target<-Turn_Amplitude)  Turn_Target=-Turn_Amplitude;

	 if(Flag_forward==1||Flag_retreat==1) Turn_Kd=7;		          
    else 										  Turn_Kd=0;//前进或者后退标志
	


	
	 //=============turing PD controller==================//	
		Turn_PWM= -Turn_Target*Turn_Kp+gyro*Turn_Kd;
 
  return Turn_PWM;
  

}


/*************************************************************************** 
函数功能：控制电机
******************************************************************************/
void Contrl(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, BIN1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, BIN2);
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, AIN1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, AIN2);	
		

}




/**************************************************************************
函数功能：电机异常关闭函数
入口参数：角度
返回  值：1：关闭，0：不关闭
**************************************************************************/	 		
u8 Stop(signed int angle)
{
	    u8 temp=0;
			if(angle<-50||angle>50)
			{	                                //===倾角大于40度关闭电机
				temp=1;                   		   //===Flag_Stop置1关闭电机
				Moto1 = 0;
				Moto2 = 0;
      }
	
		return temp;
}



/**************************************************************************
函数功能：电机转动控制函数
入口参数：闭环控制最终输出值
**************************************************************************/	
void Set_Pwm(int Moto1,int Moto2)
{
	
	 Moto_Flag=Stop(Roll);	//获取是否倒下的标志
	if(Start_Flag == 1)		//一级判断系统是否正常初始化
	{
		if(Moto_Flag==0)	//二级判断//
		{
			if(Moto1>0)  AIN1 = 1,AIN2 = 0;
			else			 AIN1 = 0,AIN2 = 1;				
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,abs(Moto1));		 
			
			if(Moto2>0)  BIN1 = 0,BIN2 = 1;		
			else		    BIN1 = 1 ,BIN2 = 0;
    		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,abs(Moto2)); //0-7200
		}
			
		else//倒下就关闭电机
		{
		   __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,0);	//4500-6000
		   __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,0);	//4500-6000
		}
		 Contrl();
	}
}


/**************************************************************************
函数功能：限制电机速度
入口参数：闭环控制最终输出值
**************************************************************************/	
void Limit(int *motoA,int *motoB)
{
	if(*motoA>7000)*motoA=7000;//最大7200
	if(*motoA<-7000)*motoA=-7000;

	if(*motoB>7000)*motoB=7000;
	if(*motoB<-7000)*motoB=-7000;
} 


void jizhi(void)
{
	Movement = 0;
	Turn_Pwm = 0;
}
void forward(void)
{
	Movement = 2000;
		Turn_Pwm = 0;
}
void back(void)
{
	Movement = -500;
		Turn_Pwm = 0;
}
void right(void)
{
	Movement = 300;
	Turn_Pwm = 300;
}
void left(void)
{
	Movement =300;
	Turn_Pwm = -300;
}
void BigRight(void)
{
	Movement = 500;
	Turn_Pwm = 500;
}
void BigLeft(void)
{
	Movement = 500;
	Turn_Pwm = -500;
}

/**************************************************************************
函数功能：循迹
入口参数：无
**************************************************************************/	
void XunJi(void)//xun = 1 检测到白线， xun = 0 检测到黑线
{
	Read_8PIN();
	if(!xun[0] && !xun[1] && !xun[2] && !xun[3] && !xun[4] && !xun[5] && !xun[6] && !xun[7])//检测到全是黑线
	{
		jizhi();//静止
		
		Black_miss_flag = 1;
	}
	if(xun[0] || xun[1] || xun[2] || xun[3] || xun[4] || xun[5] || xun[6] || xun[7])//检测到有白线
	{
		Black_miss_flag = 0;
	}
	if((!xun[3] || !xun[4] ) && !Black_miss_flag)
	{
		forward();//gogogo
	}
	if((!xun[1] && !xun[2]) && !Black_miss_flag)
	{
		left();
	}
	if(!xun[0] && !Black_miss_flag)
	{
		BigLeft();
	}
	if((!xun[5]&&!xun[6]) && !Black_miss_flag)
	{
		right();
	}
	if(!xun[7] && !Black_miss_flag)
	{
		BigRight();
	}	
}


/**************************************************************************
函数功能：蓝牙遥控设置USART3接收中断回调
引脚：		 
**************************************************************************/
//串口空闲中断回调函数
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)	
{
	if(strcmp(Usart3_RX_Buf,"1")==0)
	{
			jizhi();
	}
	else if(strcmp(Usart3_RX_Buf,"2")==0)
	{
			Flag_forward = 1,Flag_retreat = 0;
	}
	else if(strcmp(Usart3_RX_Buf,"3")==0)
	{
		right();
	}	
	else if(strcmp(Usart3_RX_Buf,"4")==0)
	{
			left();
	}	
	else if(strcmp(Usart3_RX_Buf,"5")==0)
	{
		  Flag_retreat = 1, Flag_forward = 0;
	}		
	else if(strcmp(Usart3_RX_Buf,"6")==0)
	{
			BigRight();
	}	
	else if(strcmp(Usart3_RX_Buf,"7")==0)
	{
		  BigLeft();
	}			
		HAL_UARTEx_ReceiveToIdle_IT(&huart3, (uint8_t *)Usart3_RX_Buf,50);				
}
