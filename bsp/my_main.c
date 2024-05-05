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

uint8_t Usart3_RX_Buf;  //串口接收数据缓存buf
u8 Flag_forward = 0, Flag_retreat = 0, Flag_left = 0, Flag_right = 0;//蓝牙遥控标志
unsigned char Usart_TX_Buf[40];


//----子函数声明-----
void UART_Proc(void);
//void	go();
//void	stop();
//void	backward();
int Vertical(float Angle,float Gyro_Y);			 				//直立环
int Velocity(int Encoder_left,int Encoder_right);				//速度环
int Turn(int Encoder_Left,int Encoder_Right,float gyro);	//转向环
u8 Stop(signed int angle);    		//倒下保护
void Limit(int *motoA,int *motoB);  //电机速度限幅
void Set_Pwm(int Moto1,int Moto2);	//控制PWM最终输出
	



u8 test111 = 0;

void key_proc(void)
{
	
	
/*短按*/	
	if(bkey[2].short_flag)
	{
		printf("short2 ok\n\r");
		bkey[2].short_flag = 0;
	}
	if(bkey[1].short_flag)
	{	
		printf("short1 ok\n\r");		
		bkey[1].short_flag = 0;
	}		
/*长按*/	
	if(bkey[2].long_flag)
	{
		
		printf("long2 ok\n\r");			
		bkey[2].long_flag = 0;
	}
	if(bkey[1].long_flag)
	{
		printf("long1 ok\n\r");				
		bkey[1].long_flag = 0;		
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

	HAL_UARTEx_ReceiveToIdle_IT(&huart3, (uint8_t *)Usart3_RX_Buf,50);//使能串口3接收中断	 
	 

	//------MPU6050测试-----
//   printf("Mpu6050 Project Start\r\n");

  //----iic读取MUP器件ID----
  uint8_t recv = 0x00;
  HAL_I2C_Mem_Read(&hi2c1, (0x68 << 1), 0x75, I2C_MEMADD_SIZE_8BIT, &recv, 1, 0xfff);//句柄，MUP地址,器件ID寄存器
  
//  if (recv == 0x68)
//  {
//    printf("mpu6050 ID Read: OK at 0x68\r\n");	  
//  }
//  else
//  {
//    printf("Err mpu id:0x%x\r\n", recv);  
//  }  	 

  //-----DMP初始化----
  while(mpu_dmp_init())//成功返回0，否则返回1
  {
		uint8_t res;
		res = mpu_dmp_init();
		HAL_Delay(300);
		printf("res=%d\r\n",res);
	   printf("-----------------------333------------\r\n");

  }

 printf("---------------MPU初始化成功！！---------------\r\n");
 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0); //打开PC13 LED
 Start_Flag = 1; //标志系统初始化成功



}

void loop(void)
{
		UART_Proc();
	key_proc();
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
//		test111 =8;
	  mpu_dmp_get_data(&Pitch,&Roll,&Yaw);	    // 读取角度
    MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);  // 读取角速度
    MPU_Get_Accelerometer(&aacx,&aacy,&aacz); // 读取加速度

//	  Encoder_Left  =   Read_Encoder(3);		//读取编码器测量的电机转速
//		Encoder_Right =   Read_Encoder(4);		

  // 2.将数据压入闭环控制中，计算出控制输出量
//	  Velocity_out = Velocity(Encoder_Left,Encoder_Right);// 速度环输出误差  
	  Vertical_out = Vertical(Roll,gyrox+62);// 直立环输出PWM
//		
	  // -------------蓝牙控制--------------
//	  if(1==Flag_left||1==Flag_right)    
//			Turn_Pwm =Turn(Encoder_Left,Encoder_Right,gyroz+33);//出现左右转标志才进入转向环闭环控制
//	  else   
//			Turn_Pwm=-0.5*(gyroz+33); //保持走直线
//	  
//	  
		//------------最终输出----------------
//	  PWM_out= Vertical_out+Velocity_out;
		PWM_out= Vertical_out;
	  // 3.把控制输出量加载到电机上，完成最终控制
//		Moto1 = PWM_out+Turn_Pwm; // 左电机
//      Moto2 = PWM_out-Turn_Pwm; // 右电机  
			Moto1 = PWM_out; // 左电机
      Moto2 = PWM_out; // 右电机  
      Limit(&Moto1,&Moto1);     // PWM限幅 
//		  Set_Pwm(Moto1,Moto2);        // 加载到电机上 
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
	float Bias, Vertical_Kp= -550,Vertical_Kd= -02;	//直立环Kp,Kd
	Bias = Angle-0.25;//直立偏差
	PWM_out = Vertical_Kp*Bias+Vertical_Kd*(Gyro_Y-0);	
	return PWM_out;	
} 


/********************************************************************* 
速度环PI控制器：Kp*Ek+Ki*Ek_S(Ek_S：偏差的积分)
入口：左右编码器测到的数值
出口：                   
**********************************************************************/
int Velocity(int Encoder_left,int Encoder_right)	
{
	// 定义成静态变量，保存在静态存储器，使得变量不丢掉
	static float Velocity,Encoder_Err,Encoder,Encoder_last,Movement=0; //速度，误差，编码器
	static float Encoder_Integral;					  //编码器数值积分
		
	
	float kp=10,ki=0.05;	
	
	//--------------蓝牙控制----------------
	if(1==Flag_forward) 		    Movement = -200;
	else if(1==Flag_retreat)    Movement = 200;
	else							    Movement = 0;
	
	
	// 1.计算速度偏差 	
	Encoder_Err = ((Encoder_Left+Encoder_Right)-0);	
 
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
int Turn(int Encoder_Left,int Encoder_Right,float gyro)
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
	
	 int dead_zone = 3000 ;		//L298N控制死区 0 - 3000
//	 Moto_Flag=Stop(Roll);	//获取是否倒下的标志
	if(Start_Flag == 1)		//一级判断系统是否正常初始化
	{
		if(1)	//二级判断//Moto_Flag==0
		{
			if(Moto1>0)  AIN1 = 1,AIN2 = 0;		
			else			 AIN1 = 0,AIN2 = 1;				
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,abs(Moto1));		 
			
			if(Moto2>0)  BIN1 = 1,BIN2 = 0;		
			else		    BIN1 = 0 ,BIN2 = 1;
    		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,abs(Moto1)); //0-7200
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

/**************************************************************************
函数功能：蓝牙遥控设置USART1接收中断回调
引脚：		 TX_PA9,RX_PA10
**************************************************************************/
//串口中断回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) 
{
//	switch(Usart3_RX_Buf)
//    {
//        case 0:
//        Flag_forward = 0, Flag_retreat = 0, Flag_left = 0, Flag_right = 0;
//        break;
//        case 1:
//        Flag_forward = 1, Flag_retreat = 0, Flag_left = 0, Flag_right = 0;
//        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//        break;
//        case 2:
//        Flag_forward = 0, Flag_retreat = 1, Flag_left = 0, Flag_right = 0;
//        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//        break;
//        case 3:
//        Flag_forward = 0, Flag_retreat = 0, Flag_left = 1, Flag_right = 0;
//        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//        break;
//        case 4:
//        Flag_forward = 0, Flag_retreat = 0, Flag_left = 0, Flag_right = 1;
//        HAL_GPIO_TogglePin(GPIOC, GPIO_PIN_13);
//        break;
//        default:
//        Flag_forward = 0, Flag_retreat = 0, Flag_left = 0, Flag_right = 0;
//        HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13,1);
//        break;
//    }
	HAL_UARTEx_ReceiveToIdle_IT(&huart3, (uint8_t *)Usart3_RX_Buf,50);
}













/**************************************************************************
函数功能：通过串口测试数据
**************************************************************************/
void UART_Proc()
{
  if((uwTick - uwTick_UART) < 1000) return;//return;结束函数
	uwTick_UART = uwTick;
	
//	//	printf("%d\r\n",PWM_out);
//	printf("-------------------------------------------\r\n");	
//  if ((mpu_dmp_get_data(&Pitch, &Roll, &Yaw) == 0) && (MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz) == 0)) //DMP读取欧拉角，digital motion processor数字运动处理器
//   {
//		//俯仰角，横滚角，偏航角
//		 printf("pitch=%.3f   roll=%.3f   yaw=%.3f \r\n", Pitch, Roll, Yaw);   //串口打印欧拉角
//  	 printf("gyrox=%d   gyroy=%d  gyroz=%d \r\n", gyrox, gyroy, gyroz);	// 角速度
//		 printf("-------------------------------------------\r\n");
//			
//	 }
//	 
//	 printf("left:%d\r\n", Encoder_Left);	
//	 printf("right:%d\r\n", Encoder_Right);	
//	 printf("Vertical_out=%d",Vertical_out);
printf("%d\r\n",test111);
}
