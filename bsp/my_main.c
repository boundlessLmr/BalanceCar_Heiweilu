#include "my_main.h"



//-----ȫ�ֱ���---
extern struct Bkeys bkey[];

__IO uint32_t uwTick_UART = 0;
int Vertical_out,Velocity_out,Turn_out; // ֱ����&�ٶȻ�&ת�򻷵��������
int Encoder_Left,Encoder_Right,Balance_PWM;
int Moto1,Moto2;
int PWM_out,Turn_Pwm; //�ջ����
		
u8 AIN1,AIN2;//���Ƶ������ת
u8 BIN1,BIN2;
u8 Moto_Flag=0;//������Ʊ�־
u8 Start_Flag=0;//ϵͳ��ʼ����ɱ�־

float Pitch,Roll,Yaw;	        // Pitch�������ǣ�Roll������ǣ�Yaw��ƫ����
short gyrox,gyroy,gyroz;        // ���ٶ�
short aacx,aacy,aacz;           // ���ٶ�
float Med_Angle=0;//��е��ֵ����ʹ��С������ƽ��ס�ĽǶ� 

uint8_t Usart3_RX_Buf;  //���ڽ������ݻ���buf
u8 Flag_forward = 0, Flag_retreat = 0, Flag_left = 0, Flag_right = 0;//����ң�ر�־
unsigned char Usart_TX_Buf[40];


//----����ѡ��-----// 
#define TIAO_VERp          0
#define TIAO_VERd          1
#define TIAO_VELp          2

uint8_t Tiao_Who = TIAO_VELp;
float test_num_VERp = 10;
float test_num_VERd = 1;
float test_num_VELp = 0.1;



//----ֱ����-----// 
float Bias, Vertical_Kp= 288,Vertical_Kd= 1.38;
float Med = - 5.5;

//ֱ����Kp,Kd��Ϊ��ֵ (480  2.3) * 0.6 =  288    1.38  
//ֱ����Kp,Kd��Ϊ��ֵ (480  2.2) * 0.6 =  288    1.32  !
//ֱ����Kp,Kd��Ϊ��ֵ (600  2.4) * 0.6 =  360    1.44  X
//ֱ����Kp,Kd��Ϊ��ֵ (650  2.5) * 0.6 =  390    1.5   X
//float Bias, Vertical_Kp= 0,Vertical_Kd= 0;	


//----�ٶȻ�-----
 float Velocity,Encoder_Err,Encoder,Encoder_last,Movement=0; //�ٶȣ���������
 float Encoder_Integral;					  //��������ֵ����
 float kp=5,ki=0.025;
 
//----�Ӻ�������-----
void UART_Proc(void);
//void	go();
//void	stop();
//void	backward();
int Vertical(float Angle,float Gyro_Y);			 				//ֱ����
int GetVelocity(int Encoder_left,int Encoder_right);				//�ٶȻ�
int Turn(int Encoder_Left,int Encoder_Right,float gyro);	//ת��
u8 Stop(signed int angle);    		//���±���
void Limit(int *motoA,int *motoB);  //����ٶ��޷�
void Set_Pwm(int Moto1,int Moto2);	//����PWM�������



u8 test111 = 0;
uint8_t page = 0;

void key_proc(void)
{
/*�̰�*/	
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
/*����*/	
	if(bkey[2].long_flag)
	{
//		page++;
		Set_Pwm(0,0);
		Start_Flag = !Start_Flag;
		if(Start_Flag)
		{
			OLED_ShowString(103,00,"on",12);     
		}
		else
		{
			OLED_ShowString(103,00,"of",12);    		
		}
//		if(page == 2)page = 0;
//    OLED_Clear();		
		
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
�������ܣ�ͨ�����ڲ�������
**************************************************************************/
void UART_Proc()
{
//  if((uwTick - uwTick_UART) < 1000) return;//return;��������
//	uwTick_UART = uwTick;
//	
//	printf("kP:%.2f,kD:%.2f,flag:%d,num:%.2f\r\n",Vertical_Kp,Vertical_Kd,Start_Flag,test_num);
//	
//	printf("-------------------------------------------\r\n");	
//  if ((mpu_dmp_get_data(&Pitch, &Roll, &Yaw) == 0) && (MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz) == 0)) //DMP��ȡŷ���ǣ�digital motion processor�����˶�������
//   {
//		//�����ǣ�����ǣ�ƫ����
//		 printf("pitch=%.3f   roll=%.3f   yaw=%.3f \r\n", Pitch, Roll, Yaw);   //���ڴ�ӡŷ����
//  	 printf("gyrox=%d   gyroy=%d  gyroz=%d \r\n", gyrox, gyroy, gyroz);	// ���ٶ�
//		 printf("-------------------------------------------\r\n");
//			
//	 }

//	 printf("Vertical_out=%d",Vertical_out);
	printf("%d,1\r\n",PWM_out);
}

void oled_proc()
{//��x:0~127    ��y:0~63
	if(page == 0)
	{
		switch(Tiao_Who)
		{
			case TIAO_VERp:
											//============= ��һ�� ��ֱ����=======================//	
												OLED_ShowString(00,00,"P:",12);                   
												OLED_ShowFNum(24,00,Vertical_Kp,12);
											//=============  �ڶ���=======================//	
												OLED_ShowString(00,12,"D:",12);                   
												OLED_ShowFNum(24,12,Vertical_Kd,12);
											//=============  ������=======================//	
												OLED_ShowString(00,24,"ro:",12);
												OLED_ShowFNum(36,24,Roll,12);
											//=============  ������=======================//	
												OLED_ShowString(00,36,"gx:",12);
												OLED_ShowFNum(36,36,gyrox,12);	
											//=============  ��5��=======================//	
												OLED_ShowString(00,48,"vp:",12);
												OLED_ShowFNum(36,48,Vertical_out,12);			
											
												OLED_Refresh();	break;					
			
			case TIAO_VERd:
											//============= ��һ�� ��ֱ����=======================//	
												OLED_ShowString(00,00,"P:",12);                   
												OLED_ShowFNum(24,00,Vertical_Kp,12);
											//=============  �ڶ���=======================//	
												OLED_ShowString(00,12,"D:",12);                   
												OLED_ShowFNum(24,12,Vertical_Kd,12);
											//=============  ������=======================//	
												OLED_ShowString(00,24,"ro:",12);
												OLED_ShowFNum(36,24,Roll,12);
											//=============  ������=======================//	
												OLED_ShowString(00,36,"gx:",12);
												OLED_ShowFNum(36,36,gyrox,12);	
											//=============  ��5��=======================//	
												OLED_ShowString(00,48,"vp:",12);
												OLED_ShowFNum(36,48,Vertical_out,12);			
											
												OLED_Refresh();	break;					
						
			case TIAO_VELp:
											//============= ��һ�� ���ٶȻ�=======================//	
												OLED_ShowString(00,00,"P:",12);                   
												OLED_ShowFNum(24,00,kp,12);
											//=============  �ڶ���=======================//	
												OLED_ShowString(00,12,"i:",12);                   
												OLED_ShowFNum(24,12,ki,12);
											//=============  ������=======================//	
												OLED_ShowString(00,24,"le:",12);
												OLED_ShowSignedNum(24,24,Encoder_Left,4,12);
												
									//			OLED_ShowString(00,24,"er:",12);
									//		  OLED_ShowSignedNum(24,24,Encoder_Err,4,12);			
											//=============  ������=======================//	
												OLED_ShowString(00,36,"ri:",12);
												OLED_ShowSignedNum(24,36,Encoder_Right,4,12);
									//			OLED_ShowString(00,36,"ri:",12);
									//		  OLED_ShowSignedNum(24,36,test111,4,12);			
											//=============  ��5��=======================//		
												OLED_ShowFNum(00,48,Roll,12);	  OLED_ShowFNum(60,48,PWM_out,12);	 
											// û�е�6��
									
												OLED_Refresh();break;									
		}
	}
	else if(page == 1)
	{
	
	}
}

void set_up(void)
{
  //����TIM��ʼ��
   HAL_TIM_Base_Start_IT(&htim2);//ÿ10ms����һ���ж� 
  
   HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);//PWM
	 HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);	
  
	 HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_1);//������
	 HAL_TIM_Encoder_Start(&htim3,TIM_CHANNEL_2);
	 HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_1);
	 HAL_TIM_Encoder_Start(&htim4,TIM_CHANNEL_2);	

	HAL_UARTEx_ReceiveToIdle_IT(&huart3, (uint8_t *)Usart3_RX_Buf,50);//ʹ�ܴ���3�����ж�	 
	
		OLED_Init();	
  	OLED_Clear();	
	
	OLED_ShowString(00,00,"mpu_dmp_init...",12);  
	OLED_Refresh();	
  //-----DMP��ʼ��----
  while(mpu_dmp_init())//�ɹ�����0�����򷵻�1
  {
		uint8_t res;
		res = mpu_dmp_init();
		HAL_Delay(300);
		OLED_ShowNum(00,12,res,1,12);
		OLED_Refresh();	
  }
  	OLED_Clear();	
		OLED_ShowString(103,00,"on",12);  
 HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, 0); //��PC13 LED
 Start_Flag = 1; //��־ϵͳ��ʼ���ɹ�
//	Set_Pwm(1200,1200);
}

void loop(void)
{
	UART_Proc();
	key_proc();
	oled_proc();
}

/**************************************************************************
�������ܣ�10ms��ʱ���жϣ������ƺ���
**************************************************************************/
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{

  if(htim->Instance == TIM2)//TIM2����Ϊ10ms�ж�
  {
		// 1.�ɼ�����������&MPU6050�Ƕ���Ϣ
   // �������԰�װ���պ����180�ȣ�Ϊ�˱������������һ�£�����Ҫ������һ��ȡ��  	
	  mpu_dmp_get_data(&Pitch,&Roll,&Yaw);	    // ��ȡ�Ƕ�
    MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);  // ��ȡ���ٶ�
    MPU_Get_Accelerometer(&aacx,&aacy,&aacz); // ��ȡ���ٶ�

	  Encoder_Left  =   Read_Encoder(3);		//��ȡ�����������ĵ��ת��
		Encoder_Right =   Read_Encoder(4);		

  // 2.������ѹ��ջ������У���������������
	  Velocity_out = GetVelocity(Encoder_Left,Encoder_Right);// �ٶȻ�������  
	  Vertical_out = Vertical(Roll,gyrox);// ֱ�������PWM
//		
	  // -------------��������--------------
//	  if(1==Flag_left||1==Flag_right)    
//			Turn_Pwm =Turn(Encoder_Left,Encoder_Right,gyroz+33);//��������ת��־�Ž���ת�򻷱ջ�����
//	  else   
//			Turn_Pwm=-0.5*(gyroz+33); //������ֱ��
//	  
//	  
		//------------�������----------------
	  PWM_out= Vertical_out+Velocity_out;
	  // 3.�ѿ�����������ص�����ϣ�������տ���
//		Moto1 = PWM_out+Turn_Pwm; // ����
//      Moto2 = PWM_out-Turn_Pwm; // �ҵ��  
			Moto1 = PWM_out; // ����
      Moto2 = PWM_out; // �ҵ��  
      Limit(&Moto1,&Moto1);     // PWM�޷� 
		  Set_Pwm(Moto1,Moto2);        // ���ص������ 
  }
	key_serv_long();
}

/*************************************************************************** 
ֱ����PD��������Kp*Ek+Kd*Ek_D
��ڣ�Med:��е��ֵ(�����Ƕ�)��Angle:��ʵ�Ƕȣ�gyro_Y:��ʵ���ٶ�
���ڣ�PMW��ֵ
******************************************************************************/
int Vertical(float Angle,float Gyro_Y) 
{

	Bias = Angle-Med;//ֱ��ƫ�� //angle = -0.23��ƽ����
	PWM_out = Vertical_Kp*Bias+Vertical_Kd*(Gyro_Y-0);	
	return PWM_out;	
} 


/********************************************************************* 
�ٶȻ�PI��������Kp*Ek+Ki*Ek_S(Ek_S��ƫ��Ļ���)
��ڣ����ұ������⵽����ֵ
���ڣ�                   
**********************************************************************/
int GetVelocity(int Encoder_left,int Encoder_right)	
{
	
//	//--------------��������----------------
//	if(1==Flag_forward) 		    Movement = -200;
//	else if(1==Flag_retreat)    Movement = 200;
//	else							    Movement = 0;
	
	
	// 1.�����ٶ�ƫ�� 	
	Encoder_Err = ((Encoder_Left+Encoder_Right)-0);	//�ٶ�Ŀ��ֵ//Encoder_Left+Encoder_Right ����� 160  7200 /��160 * 50%�� = 90
 
	// 2.���ٶ�ƫ�����--��ͨ�˲�--
  // low_out = (1-a)*Ek+a*low_out_last	
	Encoder = Encoder_Err*0.3 + Encoder_last*0.7;// ʹ�ò��θ���ƽ��
	Encoder_last = Encoder; 							// ��ֹ�ٶȹ���Ӱ��ֱ��������������

	// 3.���ٶ�ƫ����ֳ�λ��,ң�ص��ٶ�ͨ�����������ٶȿ������������ٶ�ͻ���ֱ�����Ƶ�Ӱ��	
  Encoder_Integral += Encoder-Movement;	


	
	

	// 4.�����޷�	
	if(Encoder_Integral>3000)  	Encoder_Integral=3000;   
	if(Encoder_Integral<-3000)	   Encoder_Integral=-3000;           	

	if(Moto_Flag == 1||Start_Flag ==0) 			Encoder_Integral=0;     		//===����رպ���߸�λ�������
    //5.�ٶȻ��������	
  Velocity=Encoder*kp+Encoder_Integral*ki;

	
	
	return Velocity;



}


/*********************************************************************
ת�򻷣�ϵ��*Z����ٶ�+ϵ��*ң������
��ڣ����ҵ�������������ֵ��Z����ٶ�
**********************************************************************/
int Turn(int Encoder_Left,int Encoder_Right,float gyro)
{
   float Turn_Target,Turn_PWM,Bias,Encoder_temp,Turn_Convert=70,Turn_Count; 
   float Turn_Amplitude=100,Turn_Kp=10,Turn_Kd=0;  
	
	// --- �������� ---
	if(1==Flag_left) 			Turn_Target+=Turn_Convert;//��ת��־
	else if(1==Flag_right)  Turn_Target-=Turn_Convert; //��ת��־
	 else 						 Turn_Target=0;
	
	 if(Turn_Target>Turn_Amplitude)  Turn_Target=Turn_Amplitude; //ת���޷�    
    if(Turn_Target<-Turn_Amplitude)  Turn_Target=-Turn_Amplitude;

	 if(Flag_forward==1||Flag_retreat==1) Turn_Kd=7;		          
    else 										  Turn_Kd=0;//ǰ�����ߺ��˱�־
	


	
	 //=============turing PD controller==================//	
		Turn_PWM= -Turn_Target*Turn_Kp+gyro*Turn_Kd;
 
  return Turn_PWM;
  
  
  

}



	
	
/*************************************************************************** 
�������ܣ����Ƶ��
******************************************************************************/
void Contrl(void)
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_15, BIN1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, BIN2);
	
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, AIN1);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, AIN2);	
		

}




/**************************************************************************
�������ܣ�����쳣�رպ���
��ڲ������Ƕ�
����  ֵ��1���رգ�0�����ر�
**************************************************************************/	 		
u8 Stop(signed int angle)
{
	    u8 temp=0;
			if(angle<-50||angle>50)
			{	                                //===��Ǵ���40�ȹرյ��
				temp=1;                   		   //===Flag_Stop��1�رյ��
				Moto1 = 0;
				Moto2 = 0;
      }
	
		return temp;
}



/**************************************************************************
�������ܣ����ת�����ƺ���
��ڲ������ջ������������ֵ
**************************************************************************/	
void Set_Pwm(int Moto1,int Moto2)
{
	
	 int dead_zone = 3000 ;		//L298N�������� 0 - 3000
	 Moto_Flag=Stop(Roll);	//��ȡ�Ƿ��µı�־
	if(Start_Flag == 1)		//һ���ж�ϵͳ�Ƿ�������ʼ��
	{
		if(Moto_Flag==0)	//�����ж�//
		{
			if(Moto1>0)  AIN1 = 1,AIN2 = 0;
			else			 AIN1 = 0,AIN2 = 1;				
			__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,abs(Moto1));		 
			
			if(Moto2>0)  BIN1 = 0,BIN2 = 1;		
			else		    BIN1 = 1 ,BIN2 = 0;
    		__HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,abs(Moto2)); //0-7200
		}
			
		else//���¾͹رյ��
		{
		   __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_1,0);	//4500-6000
		   __HAL_TIM_SET_COMPARE(&htim1,TIM_CHANNEL_4,0);	//4500-6000
		}
		 Contrl();
	}
}


/**************************************************************************
�������ܣ����Ƶ���ٶ�
��ڲ������ջ������������ֵ
**************************************************************************/	
void Limit(int *motoA,int *motoB)
{
	if(*motoA>7000)*motoA=7000;//���7200
	if(*motoA<-7000)*motoA=-7000;

	if(*motoB>7000)*motoB=7000;
	if(*motoB<-7000)*motoB=-7000;
}

/**************************************************************************
�������ܣ�����ң������USART1�����жϻص�
���ţ�		 TX_PA9,RX_PA10
**************************************************************************/
//�����жϻص�����
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
