/***********************************************
��˾����Ȥ�Ƽ�����ݸ�����޹�˾
Ʒ�ƣ�WHEELTEC
������wheeltec.net
�Ա����̣�shop114407458.taobao.com 
����ͨ: https://minibalance.aliexpress.com/store/4455017
�汾��5.7
�޸�ʱ�䣺2021-04-29

 
Brand: WHEELTEC
Website: wheeltec.net
Taobao shop: shop114407458.taobao.com 
Aliexpress: https://minibalance.aliexpress.com/store/4455017
Version:5.7
Update��2021-04-29

All rights reserved
***********************************************/
#include "usart3.h"

SEND_DATA Send_Data;
RECEIVE_DATA Receive_Data;
extern int Time_count;
#define CONTROL_DELAY		1000
#if SYSTEM_SUPPORT_OS
#include "includes.h"					//ucos ʹ��	  
#endif

u8 Usart1_Receive_buf[1];           //����1�����ж����ݴ�ŵĻ�����
u8 Usart1_Receive;                  //�Ӵ���1��ȡ������
u8 Usart3_Receive_buf[1];          //����3�����ж����ݴ�ŵĻ�����
u8 Usart3_Receive;                 //�Ӵ���3��ȡ������
u8 Uart5_Receive_buf[1];          //����5�����ж����ݴ�ŵĻ�����
u8 Uart5_Receive;                 //�Ӵ���5��ȡ������
/**************************************************************************
Function: Receive interrupt function
Input   : none
Output  : none
�������ܣ�����3�����ж�
��ڲ�������
����  ֵ����
**************************************************************************/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) //���ջص�����
{	
	if(UartHandle->Instance == USART3) //���յ�����
	{	  
	  static	int uart_receive=0;//����������ر���
		static u8 Flag_PID,i,j,Receive[50];
		static float Data;
  	uart_receive=Usart3_Receive_buf[0]; 
		Usart3_Receive=uart_receive;
		if(Mode != Bluetooth_Mode)  // 蓝牙模式下跳过方向指令
		{
		if(uart_receive==0x59)  Target_Velocity-=100,Flag_velocity=2;  //加速
		if(uart_receive==0x58)  Target_Velocity+=100,Flag_velocity=1;  //减速
		
	  if(uart_receive>10)  //默认使用
    {			
			if(uart_receive==0x5A)	    Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;//刹车
			else if(uart_receive==0x41)	Flag_front=1,Flag_back=0,Flag_Left=0,Flag_Right=0;//前
			else if(uart_receive==0x45)	Flag_front=0,Flag_back=1,Flag_Left=0,Flag_Right=0;//后
			else if(uart_receive==0x42||uart_receive==0x43||uart_receive==0x44)	
																	Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=1;  //右
			else if(uart_receive==0x46||uart_receive==0x47||uart_receive==0x48)	    
																	Flag_front=0,Flag_back=0,Flag_Left=1,Flag_Right=0;  //左
			else Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;//刹车
  	}
		if(uart_receive<10)     //老版本app为MiniBalanceV1.0 新版为MiniBalanceV1.0的遥控指令为A~H 其HEX小于10
		{			
			Flag_velocity=1;//转向速度切换为低速挡
			if(uart_receive==0x00)	Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;//刹车
			else if(uart_receive==0x01)	Flag_front=1,Flag_back=0,Flag_Left=0,Flag_Right=0;//前
			else if(uart_receive==0x05)	Flag_front=0,Flag_back=1,Flag_Left=0,Flag_Right=0;//后
			else if(uart_receive==0x02||uart_receive==0x03||uart_receive==0x04)	
														Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=1;  //右
			else if(uart_receive==0x06||uart_receive==0x07||uart_receive==0x08)	    //左
														Flag_front=0,Flag_back=0,Flag_Left=1,Flag_Right=0;
			else Flag_front=0,Flag_back=0,Flag_Left=0,Flag_Right=0;//刹车
  	}	
		}

		
		if(Usart3_Receive==0x7B) Flag_PID=1;   //APP����ָ����ʼλ
		if(Usart3_Receive==0x7D) Flag_PID=2;   //APP����ָ��ֹͣλ

		 if(Flag_PID==1)  //�ɼ�����
		 {
				Receive[i]=Usart3_Receive;
				i++;
		 }
		 if(Flag_PID==2)  //��������
		 {
			  if(Receive[3]==0x50) 				 PID_Send=1;
			  else if(Receive[1]!=0x23) 
				{								
					for(j=i;j>=4;j--)
					{
						Data+=(Receive[j-1]-48)*pow(10,i-j);
					}
					switch(Receive[1])
					{
						case 0x30:  Target_Velocity=Data;break;
						case 0x31:  Balance_Kp=Data;break;
						case 0x32:  Balance_Kd=Data;break;
						case 0x33:  Velocity_Kp=Data;break;
						case 0x34:  Velocity_Ki=Data;break;
						case 0x35:  Turn_Kp=Data;break; 
					  case 0x36:  Turn_Kd=Data;break; 
						case 0x37:  Distance_KP=Data;break;
						case 0x38:  Distance_KD=Data;break;
						default: break;
					}
				}				 
			    Flag_PID=0;
					i=0;
					j=0;
					Data=0;
					memset(Receive, 0, sizeof(u8)*50);//��������
		 } 
		HAL_UART_Receive_IT(&huart3,Usart3_Receive_buf,sizeof(Usart3_Receive_buf));//����3�ص�����ִ�����֮����Ҫ�ٴο��������жϵȴ���һ�ν����жϵķ���
	}  											 
	else if(UartHandle->Instance == UART5) //���յ�����
	{
		static u8 state = 0;//״̬λ	
		static u8 crc_sum = 0;//У���
		static u8 cnt = 0;//����һ֡16����ļ���
		u8 temp_data;
		temp_data=Uart5_Receive_buf[0]; 
		switch(state)
		{
			case 0:
				if(temp_data == HEADER_0)//ͷ�̶�
				{
					Pack_Data.header_0= temp_data;
					state++;
					//У��
					crc_sum += temp_data;
				} else state = 0,crc_sum = 0;
				break;
			case 1:
				if(temp_data == HEADER_1)//ͷ�̶�
				{
					Pack_Data.header_1 = temp_data;
					state++;
					crc_sum += temp_data;
				} else state = 0,crc_sum = 0;
				break;
			case 2:
				if(temp_data == Length_)//�ֳ��̶�
				{
					Pack_Data.ver_len = temp_data;
					state++;
					crc_sum += temp_data;
				} else state = 0,crc_sum = 0;
				break;
			case 3:
				Pack_Data.speed_h = temp_data;//�ٶȸ߰�λ
				state++;
				crc_sum += temp_data;			
				break;
			case 4:
				Pack_Data.speed_l = temp_data;//�ٶȵͰ�λ
				state++;
				crc_sum += temp_data;
				break;
			case 5:
				Pack_Data.start_angle_h = temp_data;//��ʼ�Ƕȸ߰�λ
				state++;
				crc_sum += temp_data;
				break;
			case 6:
				Pack_Data.start_angle_l = temp_data;//��ʼ�ǶȵͰ�λ
				state++;
				crc_sum += temp_data;
				break;
			
			case 7:case 10:case 13:case 16:
			case 19:case 22:case 25:case 28:
			case 31:case 34:case 37:case 40:
			case 43:case 46:case 49:case 52:
				Pack_Data.point[cnt].distance_h = temp_data;//16����ľ������ݣ����ֽ�
				state++;
				crc_sum += temp_data;
				break;
			
			case 8:case 11:case 14:case 17:
			case 20:case 23:case 26:case 29:
			case 32:case 35:case 38:case 41:
			case 44:case 47:case 50:case 53:
				Pack_Data.point[cnt].distance_l = temp_data;//16����ľ������ݣ����ֽ�
				state++;
				crc_sum += temp_data;
				break;
			
			case 9:case 12:case 15:case 18:
			case 21:case 24:case 27:case 30:
			case 33:case 36:case 39:case 42:
			case 45:case 48:case 51:case 54:
				Pack_Data.point[cnt].Strong = temp_data;//16�����ǿ������
				state++;
				crc_sum += temp_data;
				cnt++;
				break;
			
			case 55:
				Pack_Data.end_angle_h = temp_data;//�����Ƕȵĸ߰�λ
				state++;
				crc_sum += temp_data;			
				break;
			case 56:
				Pack_Data.end_angle_l = temp_data;//�����ǶȵĵͰ�λ
				state++;
				crc_sum += temp_data;
				break;
			case 57:
				Pack_Data.crc = temp_data;//У��
				state = 0;
				cnt = 0;
				if(crc_sum == Pack_Data.crc)
				{
					data_process();//���ݴ�����У����ȷ����ˢ�´洢������
				}
				else 
				{
					memset(&Pack_Data,0,sizeof(Pack_Data));//����
				}
				crc_sum = 0;//У�������
				break;
			default: break;
	  }	
   HAL_UART_Receive_IT(&huart5,Uart5_Receive_buf,sizeof(Uart5_Receive_buf));//��������5�����ж�		
	}
	else if(UartHandle->Instance == USART1)//����һ��������
	{
		static u8 Count=0;
		Usart1_Receive = Usart1_Receive_buf[0];//��ȡ����
		Mode=ROS_Mode;//ros����ʱ����С��ģʽ��ΪROSģʽ
////		if(Time_count < CONTROL_DELAY)
////		{
////			//Data is not processed until 10 seconds after startup
////			//����ʮ��ǰ��������������
////		}
		Receive_Data.buffer[Count] = Usart1_Receive;//Fill the array with serial data  ����������������
		
		//Ensure that the first data is the array is FRAME_HEADER
		//ȷ����һ������ʱ֡ͷ
		if(Usart1_Receive == FRAME_HEADER ||Count>0)
			Count++;
		else Count=0;
		if(Count == 11) //Verify the length of the packet  ��֤���ݰ��ĳ���
		{
			Count = 0;
			if(Receive_Data.buffer[10] == FRAME_TAIL)
			{
				if(Receive_Data.buffer[9] == Check_Sum(9,0))
				{
					
					//Calculate the 3-axis target velocity from the serial data,which is divided into 8-bit high and 8-bit low units m/s
					Move_X=XYZ_Target_Speed_transition(Receive_Data.buffer[3],Receive_Data.buffer[4]);//ƽ��С��û�����õ��˶�ѧ�������
					Move_Z=-XYZ_Target_Speed_transition(Receive_Data.buffer[7],Receive_Data.buffer[8])/1000*160/2/Control_Frequency*EncoderMultiples*Reduction_Ratio*Encoder_precision/Perimeter*5.6;//Move_Z��ֱ��������ת��
					//Move_Z=ROS���Ļ���/1000*�־�/2/�ܳ�/��������ȡƵ��*Ƶ��*���ٱ�*���ȣ�
					Ros_Rate=0;  //ֻҪ�жϽ������Ͱ�����0����ֹС�����ƵĿ���
				}
			}
		}
		HAL_UART_Receive_IT(&huart1,Usart1_Receive_buf,sizeof(Usart1_Receive_buf));//��������1�����ж�
	}
} 

/**************************************************************************
Function: Serial port 1 sends data
Input   : none
Output  : none
�������ܣ�����1��������
��ڲ�������
����  ֵ����
**************************************************************************/
void USART1_SEND(void)
{
  unsigned char i = 0;	
	
	for(i=0; i<24; i++)
	{
		usart1_send(Send_Data.buffer[i]);
	}	 
}
/**************************************************************************
Function: Serial port 1 sends data
Input   : The data to send
Output  : none
�������ܣ�����1��������
��ڲ�����Ҫ���͵�����
����  ֵ����
**************************************************************************/
void usart1_send(u8 data)
{
	USART1->DR = data;
	while((USART1->SR&0x40)==0);	
}

/**************************************************************************
Function: The data sent by the serial port is assigned
Input   : none
Output  : none
�������ܣ����ڷ��͵����ݽ��и�ֵ
��ڲ�������
����  ֵ����
**************************************************************************/
void data_transition(void)
{
	Send_Data.Sensor_Str.Frame_Header = FRAME_HEADER; //Frame_header //֡ͷ
	Send_Data.Sensor_Str.Frame_Tail = FRAME_TAIL;     //Frame_tail //֡β
	
	//According to different vehicle types, different kinematics algorithms were selected to carry out the forward kinematics solution, 
	//and the three-axis velocity was obtained from each wheel velocity
	//�����˶�ѧ�㷨�����˶�ѧ���⣬�Ӹ������ٶ���������ٶ�
	
	Send_Data.Sensor_Str.X_speed = ((Velocity_Left+Velocity_Right)/2); //ƽ��С�����ٶ���mm/s
	Send_Data.Sensor_Str.Y_speed = 0;
	Send_Data.Sensor_Str.Z_speed = ((Velocity_Right-Velocity_Left)/1000/0.160*1000);//0.160Ϊ�־�
	
	//The acceleration of the triaxial acceleration //���ٶȼ�������ٶ�
	Send_Data.Sensor_Str.Accelerometer.X_data= -Accel_Y; //The accelerometer Y-axis is converted to the ros coordinate X axis //���ٶȼ�Y��ת����ROS����X��
	Send_Data.Sensor_Str.Accelerometer.Y_data=Accel_X; //The accelerometer X-axis is converted to the ros coordinate y axis //���ٶȼ�X��ת����ROS����Y��
	Send_Data.Sensor_Str.Accelerometer.Z_data= Accel_Z; //The accelerometer Z-axis is converted to the ros coordinate Z axis //���ٶȼ�Z��ת����ROS����Z��
	
	//The Angle velocity of the triaxial velocity //���ٶȼ�������ٶ�
	Send_Data.Sensor_Str.Gyroscope.X_data= -Gyro_Y*4; //The Y-axis is converted to the ros coordinate X axis //���ٶȼ�Y��ת����ROS����X��
	Send_Data.Sensor_Str.Gyroscope.Y_data= Gyro_X*4; //The X-axis is converted to the ros coordinate y axis //���ٶȼ�X��ת����ROS����Y��
	if(Flag_Stop==0) 
		//If the motor control bit makes energy state, the z-axis velocity is sent normall
	  //����������λʹ��״̬����ô��������Z����ٶ�
		Send_Data.Sensor_Str.Gyroscope.Z_data=Gyro_Z*4;  
	else  
		//If the robot is static (motor control dislocation), the z-axis is 0
    //����������Ǿ�ֹ�ģ��������λʧ�ܣ�����ô���͵�Z����ٶ�Ϊ0		
		Send_Data.Sensor_Str.Gyroscope.Z_data=0;        
	
	//Battery voltage (this is a thousand times larger floating point number, which will be reduced by a thousand times as well as receiving the data).
	//��ص�ѹ(���ｫ�����Ŵ�ʮ�����䣬��Ӧ���ڽ��ն��ڽ��յ����ݺ����Сһǧ���õ�������ʾ�ĵ�ѹ������VoltageΪ1140���Ŵ�ʮ��������Сһǧ��Ϊ11.40)
	Send_Data.Sensor_Str.Power_Voltage = (float)Voltage*10; 
	
	Send_Data.buffer[0]=Send_Data.Sensor_Str.Frame_Header; //Frame_heade //֡ͷ
  Send_Data.buffer[1]=Flag_Stop; //Car software loss marker //С������ʧ�ܱ�־λ
	
	//The three-axis speed of / / car is split into two eight digit Numbers
	//С�������ٶ�,���ᶼ���Ϊ����8λ�����ٷ���
	Send_Data.buffer[2]=Send_Data.Sensor_Str.X_speed>>8; 
	Send_Data.buffer[3]=Send_Data.Sensor_Str.X_speed;    
	Send_Data.buffer[4]=Send_Data.Sensor_Str.Y_speed>>8;  
	Send_Data.buffer[5]=Send_Data.Sensor_Str.Y_speed;     
	Send_Data.buffer[6]=Send_Data.Sensor_Str.Z_speed>>8; 
	Send_Data.buffer[7]=Send_Data.Sensor_Str.Z_speed;    
	
	//The acceleration of the triaxial axis of / / imu accelerometer is divided into two eight digit reams
	//IMU���ٶȼ�������ٶ�,���ᶼ���Ϊ����8λ�����ٷ���
	Send_Data.buffer[8]=Send_Data.Sensor_Str.Accelerometer.X_data>>8; 
	Send_Data.buffer[9]=Send_Data.Sensor_Str.Accelerometer.X_data;   
	Send_Data.buffer[10]=Send_Data.Sensor_Str.Accelerometer.Y_data>>8;
	Send_Data.buffer[11]=Send_Data.Sensor_Str.Accelerometer.Y_data;
	Send_Data.buffer[12]=Send_Data.Sensor_Str.Accelerometer.Z_data>>8;
	Send_Data.buffer[13]=Send_Data.Sensor_Str.Accelerometer.Z_data;
	
	//The axis of the triaxial velocity of the / /imu is divided into two eight digits
	//IMU���ٶȼ�������ٶ�,���ᶼ���Ϊ����8λ�����ٷ���
	Send_Data.buffer[14]=Send_Data.Sensor_Str.Gyroscope.X_data>>8;
	Send_Data.buffer[15]=Send_Data.Sensor_Str.Gyroscope.X_data;
	Send_Data.buffer[16]=Send_Data.Sensor_Str.Gyroscope.Y_data>>8;
	Send_Data.buffer[17]=Send_Data.Sensor_Str.Gyroscope.Y_data;
	Send_Data.buffer[18]=Send_Data.Sensor_Str.Gyroscope.Z_data>>8;
	Send_Data.buffer[19]=Send_Data.Sensor_Str.Gyroscope.Z_data;
	
	//Battery voltage, split into two 8 digit Numbers
	//��ص�ѹ,���Ϊ����8λ���ݷ���
	Send_Data.buffer[20]=Send_Data.Sensor_Str.Power_Voltage>>8; 
	Send_Data.buffer[21]=Send_Data.Sensor_Str.Power_Voltage; 

  //Data check digit calculation, Pattern 1 is a data check
  //����У��λ���㣬ģʽ1�Ƿ�������У��
	Send_Data.buffer[22]=Check_Sum(22,1); 
	
	Send_Data.buffer[23]=Send_Data.Sensor_Str.Frame_Tail; //Frame_tail //֡β
}

/**************************************************************************
Function: After the top 8 and low 8 figures are integrated into a short type data, the unit reduction is converted
Input   : 8 bits high, 8 bits low
Output  : The target velocity of the robot on the X/Y/Z axis
�������ܣ�����λ���������ĸ�8λ�͵�8λ�������ϳ�һ��short�����ݺ�������λ��ԭ����
��ڲ�������8λ����8λ
����  ֵ��������X/Y/Z���Ŀ���ٶ�
**************************************************************************/
float XYZ_Target_Speed_transition(u8 High,u8 Low)
{
	//Data conversion intermediate variable
	//����ת�����м����
	short transition; 
	
	//����8λ�͵�8λ���ϳ�һ��16λ��short������
	//The high 8 and low 8 bits are integrated into a 16-bit short data
	transition=((High<<8)+Low); 
	return  transition; 						
}

/**************************************************************************
Function: Calculates the check bits of data to be sent/received
Input   : Count_Number: The first few digits of a check; Mode: 0-Verify the received data, 1-Validate the sent data
Output  : Check result
�������ܣ�����Ҫ����/���յ�����У����
��ڲ�����Count_Number��У���ǰ��λ����Mode��0-�Խ������ݽ���У�飬1-�Է������ݽ���У��
����  ֵ��У����
**************************************************************************/
u8 Check_Sum(unsigned char Count_Number,unsigned char mode)
{
	unsigned char check_sum=0,k;
	
	//Validate the data to be sent
	//��Ҫ���͵����ݽ���У��
	if(mode==1)
	for(k=0;k<Count_Number;k++)
	{
	  check_sum=check_sum^Send_Data.buffer[k];
	}
	
	//Verify the data received
	//�Խ��յ������ݽ���У��
	if(mode==0)
	for(k=0;k<Count_Number;k++)
	{
		check_sum=check_sum^Receive_Data.buffer[k];
	}
	return check_sum;
}
