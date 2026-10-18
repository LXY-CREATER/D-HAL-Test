/***********************************************
��˾����Ȥ�Ƽ�(��ݸ)���޹�˾
Ʒ�ƣ�WHEELTEC
������wheeltec.net
�Ա����̣�shop114407458.taobao.com 
����ͨ: https://minibalance.aliexpress.com/store/4455017
�汾��V1.0
�޸�ʱ�䣺2022-09-05

Brand: WHEELTEC
Website: wheeltec.net
Taobao shop: shop114407458.taobao.com 
Aliexpress: https://minibalance.aliexpress.com/store/4455017
Version: V1.0
Update��2022-09-05

All rights reserved
***********************************************/
#include "key.h"


/*************************************************************************
Function:Mode Choose
Iput    :None
Output  :Noce
�������ܣ�С��ģʽѡ��
��ڲ�������
����  ֵ����
*************************************************************************/
void Mode_Choose(void)
{
	switch(User_Key_Scan())
		{
			//�������������л���
			//1.��ͨң��ģʽ
			//2.�״�Ѳ��ģʽ
			//3.�״����ģʽ
			//4.���Ѳ��ģʽ
			//5.CCDѲ��ģʽ
			//��������������λ��
			case Click:
				Mode++;
				if(Mode == ELE_Line_Patrol_Mode)
					ELE_ADC_Init();							//��ʼ�����Ѳ��ģʽ
				
				else if(Mode == CCD_Line_Patrol_Mode)		//CCDѲ��ģʽ
				{
					CCD_Init();								//CCD��ʼ����CCDģ��͵��Ѳ��ģ�鹲��һ���ӿڣ���������ͬʱʹ��
				}
				else if(Mode == 10)							//10个模式循环切换(0~9)
				{
					Mode = Normal_Mode;
				}
				break;
			case Long_Press:
				Flag_Show = !Flag_Show;								//���� ����/�˳� ��λ��ģʽ
				break;
			case Double_Click:										
				if((Mode == ELE_Line_Patrol_Mode)||(Mode==CCD_Line_Patrol_Mode))					//Ѳ��״̬ʱ��˫�����Դ�/�ر��״����ϰ��Ĭ�ϴ�
				{
					Lidar_Detect = !Lidar_Detect;
					if(Lidar_Detect == Lidar_Detect_OFF)
						memset(PointDataProcess,0, sizeof(PointDataProcessDef)*40);		//�����״����ϰ������������
				}
				break;
			default:break;
		}
}

/*************************************************************************
Function:User_Key_Scan
Input:None
Output:Key_status
�������ܣ��û��������
��ڲ�������
����ֵ  ������״̬
**************************************************************************/
//����5ms�ж��е���
uint8_t User_Key_Scan(void)
{
	static u16 count_time = 0;					//���㰴�µ�ʱ�䣬ÿ5ms��1
	static u8 key_step = 0;						//��¼��ʱ�Ĳ���
	switch(key_step)
	{
		case 0:
			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == KEY_ON )
				key_step++;						//��⵽�а������£�������һ��
			break;
		case 1:
			if((++count_time) == 5)				//��ʱ����
			{
				if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == KEY_ON )//����ȷʵ������
					key_step++,count_time = 0;	//������һ��
				else
					count_time = 0,key_step = 0;//����λ
			}
			break;
		case 2:
			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == KEY_ON )
				count_time++;					//���㰴�µ�ʱ��
			else 								//��ʱ���ɿ���
				key_step++;						//������һ��
			break;
		case 3:									//��ʱ�����µ�ʱ�䣬���ж��ǳ������Ƕ̰�
			if(count_time > 400)				//��5ms�ж��е��ã��ʰ���ʱ��������400*5 = 2000ms�����ֵ��
			{							
				key_step = 0;					//��־λ��λ
				count_time = 0;
				return Long_Press;				//���� ���� ��״̬ 
 			}
			else if(count_time > 5)				//��ʱ�ǵ�����һ��
			{
				key_step++;						//��ʱ������һ�����ж��Ƿ���˫��
				count_time = 0;					//���µ�ʱ������
			}
			else
			{
				key_step = 0;
				count_time = 0;	
			}
			break;
		case 4:									//�ж��Ƿ���˫���򵥻�
			if(++count_time >20&&count_time<70)				//5*50= 250ms���жϰ����Ƿ���
			{
				if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == KEY_ON )	//����ȷʵ������
				{																	//����˫�����ܰ�̫�죬��ʶ��ɵ���
					key_step++;														//������һ������Ҫ�����ֲ����ͷ�״̬
					count_time = 0;
				}
				else																//190ms���ް������£���ʱ�ǵ�����״̬
				{
					if(count_time>65)
					{
						key_step = 0;				//��־λ��λ
						count_time = 0;					
						return Click;				//���ص�����״̬
					}
				}
			}
			break;
		case 5:
			if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0) == KEY_ON )//�������ڰ���
			{
				count_time++;
			}
			else								//�����Ѿ�����
			{
//				if(count_time>400)				//����ڶ��εĵ���Ҳ�����ж�ʱ��ģ�Ĭ�ϲ��ж�ʱ�䣬ȫ��������˫��
//				{
//				}
				count_time = 0;
				key_step = 0;
				return Double_Click;
			}
			break;
		default:break;
	}
	return No_Action;							//�޶���
}
