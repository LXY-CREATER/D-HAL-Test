/***********************************************
????????????????????????
????WHEELTEC
??????wheeltec.net
????????shop114407458.taobao.com 
?????: https://minibalance.aliexpress.com/store/4455017
?????5.7
??????2021-04-29

Brand: WHEELTEC
Website: wheeltec.net
Taobao shop: shop114407458.taobao.com 
Aliexpress: https://minibalance.aliexpress.com/store/4455017
Version:5.7
Update??2021-04-29

All rights reserved
***********************************************/
#include "show.h"
#include "usart3.h"
float Velocity_Left,Velocity_Right;	//???????(mm/s)
/**************************************************************************
Function: OLED display
Input   : none
Output  : none
?????????OLED???
??????????
????  ?????
**************************************************************************/
void oled_show(void)
{
		//=============??????????????=======================//	
		    if(Mode==0||Mode==1||Mode==2)
	  {
		     if(Way_Angle==1)	OLED_ShowString(0,0,"DMP");
		else if(Way_Angle==2)	OLED_ShowString(0,0,"Kalman");
		else if(Way_Angle==3)	OLED_ShowString(0,0,"C F");
	  }
	  if(Mode==3||Mode==4||Mode==5)
	  {
		  if(LD_Successful_Receive_flag==1)
		       OLED_ShowString(0,0,"SUCCES");
		  else
			  OLED_ShowString(0,0,"DEFEAT");
	  }
                   
	      if(Mode==2) OLED_ShowString(60,0,"U_Follow");
				 else if (Mode == 1)   OLED_ShowString(60,0,"U_Avoid ");
 		     else if(Mode==0)            OLED_ShowString(60,0,"Normal  ");
	       else if(Mode==3)            OLED_ShowString(60,0,"LD_Avoid");
	       else if(Mode==4)            OLED_ShowString(60,0,"LD_Follow");
	       else  if(Mode==5)           OLED_ShowString(60,0,"LD_Straight");
	       else if(Mode==6)            OLED_Show_CCD();
	       else if(Mode==8)            OLED_ShowString(60,0,"Coop    ");
	       else if(Mode==7)
			   {
					  OLED_ShowNumber(0,0,Sensor_Left,5,12);	
						OLED_ShowNumber(30,0,Sensor_Middle,4,12);
						OLED_ShowNumber(60,0,Sensor_Right,4,12);
						OLED_ShowNumber(90,0,Sensor,4,12);		
				 }
		//=============???????????=======================//	
		                      OLED_ShowString(00,10,"Angle");
		if( Angle_Balance<0)	OLED_ShowString(48,10,"-");
		if(Angle_Balance>=0)	OLED_ShowString(48,10,"+");
		                      OLED_ShowNumber(56,10, myabs((int)Angle_Balance),3,12);
	  //=============??????????????/????/???????===============//	
	  if(Mode==8) {
			OLED_ShowString(0,20,"Cmd:");  OLED_ShowNumber(32,20,Master_Coop_Cmd,1,12);
			OLED_ShowString(48,20,"Spd:"); OLED_ShowNumber(76,20,(int)Master_Target_Speed,3,12);
	  } else {
			OLED_ShowString(0,20,"Gyrox");
		if(Gyro_Balance<0)	  OLED_ShowString(42,20,"-");
		if(Gyro_Balance>=0)	  OLED_ShowString(42,20,"+");
		                      OLED_ShowNumber(50,20, myabs((int)Gyro_Balance),4,12);
													
													OLED_ShowNumber(82,20,(u16)Distance,5,12);
			                    OLED_ShowString(114,20,"mm");
	  }

		//=============????????????????PWM?????=======================//		
		                      OLED_ShowString(00,30,"L");
		if(Motor_Left<0)		  OLED_ShowString(16,30,"-"),
													OLED_ShowNumber(26,30,myabs((int)Motor_Left),4,12);
		if(Motor_Left>=0)	    OLED_ShowString(16,30,"+"),
		                      OLED_ShowNumber(26,30,myabs((int)Motor_Left),4,12);
													
		if(Velocity_Left<0)	  OLED_ShowString(60,30,"-");
		if(Velocity_Left>=0)	OLED_ShowString(60,30,"+");
		                      OLED_ShowNumber(68,30,myabs((int)Velocity_Left),4,12);
													OLED_ShowString(96,30,"mm/s");
	
		//=============????????????????PWM?????=======================//		
		                      OLED_ShowString(00,40,"R");
		if(Motor_Right<0)		  OLED_ShowString(16,40,"-"),
													OLED_ShowNumber(26,40,myabs((int)Motor_Right),4,12);
		if(Motor_Right>=0)	  OLED_ShowString(16,40,"+"),
		                      OLED_ShowNumber(26,40,myabs((int)Motor_Right),4,12);
													
		if(Velocity_Right<0)	OLED_ShowString(60,40,"-");
		if(Velocity_Right>=0)	OLED_ShowString(60,40,"+");
		                      OLED_ShowNumber(68,40,myabs((int)Velocity_Right),4,12);
													OLED_ShowString(96,40,"mm/s");

		//=============????????????????????=======================//
		                      OLED_ShowString(0,50,"V");
													OLED_ShowString(30,50,".");
													OLED_ShowString(64,50,"V");
													OLED_ShowNumber(19,50,Voltage/100,2,12);
													OLED_ShowNumber(42,50,Voltage/10%10,1,12);
													OLED_ShowNumber(50,50,Voltage%10,1,12);
		if(Flag_Stop)         OLED_ShowString(95,50,"OFF");
		if(!Flag_Stop)        OLED_ShowString(95,50,"ON ");
											
		//=============???=======================//
		OLED_Refresh_Gram();	
}
/**************************************************************************
Function: Send data to APP
Input   : none
Output  : none
???????????APP????????
??????????
????  ?????
**************************************************************************/
void APP_Show(void)
{    
  static u8 flag;
	int Encoder_Left_Show,Encoder_Right_Show,Voltage_Show;
	Voltage_Show=(Voltage-1000)*2/3;		if(Voltage_Show<0)Voltage_Show=0;if(Voltage_Show>100) Voltage_Show=100;   //???????????????
	Encoder_Right_Show=Velocity_Right*1.1; if(Encoder_Right_Show<0) Encoder_Right_Show=-Encoder_Right_Show;			  //??????????????????????????????
	Encoder_Left_Show=Velocity_Left*1.1;  if(Encoder_Left_Show<0) Encoder_Left_Show=-Encoder_Left_Show;
	flag=!flag;
	if(PID_Send==1)			//????PID????,??APP???????????
	{
		printf("{C%d:%d:%d:%d:%d:%d:%d:%d:%d}$",(int)Balance_Kp,(int)Balance_Kd,(int)Velocity_Kp,(int)Velocity_Ki,(int)Turn_Kp,(int)Turn_Kd,0,0,0);//?????APP????	
		PID_Send=0;	
	}	
   else	if(flag==0)		// ?????????????????????????APP??????
		printf("{A%d:%d:%d:%d}$",(int)Encoder_Left_Show,(int)Encoder_Right_Show,(int)Voltage_Show,(int)Angle_Balance); //?????APP????
	 else								//??????????????????????????
	  printf("{B%d:%d:%d}$",(int)Pitch,(int)Roll,(int)Yaw); //x??y??z???? ??APP???????????
																													//?????????????????????????????????
}
/**************************************************************************
Function: Virtual oscilloscope sends data to upper computer
Input   : none
Output  : none
?????????????????????????????????? ????????
??????????
????  ?????
**************************************************************************/
void DataScope(void)
{   
	u8 i;//????????
	float Vol;								//???????
	unsigned char Send_Count; //???????????????????
	Vol=(float)Voltage/100;
	DataScope_Get_Channel_Data( Angle_Balance, 1 );       //?????? ???????????
	DataScope_Get_Channel_Data( Distance/10, 2 );         //?????????????????? ??????CM 
	DataScope_Get_Channel_Data( Vol, 3 );                 //???????? ??????V
//		DataScope_Get_Channel_Data( 0 , 4 );   
//		DataScope_Get_Channel_Data(0, 5 ); //????????????????I0??????
//		DataScope_Get_Channel_Data(0 , 6 );//????????????????I0??????
//		DataScope_Get_Channel_Data(0, 7 );
//		DataScope_Get_Channel_Data( 0, 8 ); 
//		DataScope_Get_Channel_Data(0, 9 );  
//		DataScope_Get_Channel_Data( 0 , 10);
	Send_Count = DataScope_Data_Generate(3);
	for(i = 0 ; i < Send_Count; i++) 
	{
		while((USART1->SR&0X40)==0);  
		USART1->DR = DataScope_OutPut_Buffer[i]; 
	}
}

//????????????????
void oled_show_once(void)
{
   OLED_ShowString(0,00,"Turn Right Wheel");
   OLED_ShowString(0,10,"TO Select Mode");
	 OLED_ShowString(0,20,"Current Mode Is");
	if(Mode==0)          OLED_ShowString(20,30,"   Normal  ");
	if(Mode==1)			 OLED_ShowString(20,30,"  U_Avoid  ");
	if(Mode==2)		     OLED_ShowString(20,30," U_Follow  ");
	if(Mode==3)			 OLED_ShowString(20,30," LD_Avoid  ");
	if(Mode==4)          OLED_ShowString(20,30," LD_Follow ");
	if(Mode==5)          OLED_ShowString(20,30,"LD_Straight");
	if(Mode==6)          OLED_ShowString(20,30,"    CCD    ");
	if(Mode==7)          OLED_ShowString(20,30,"    ELE    ");
	if(Mode==8)          OLED_ShowString(20,30,"   Coop    ");
	
	OLED_ShowString(0,40,"D-Press User Key");
  OLED_ShowString(0,50,"TO End Selection");

		OLED_Refresh_Gram();	
}
