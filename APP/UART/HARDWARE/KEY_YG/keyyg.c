/*
*********************************************************************************************************
*	ģ�����ƣ� ҡ��ģ�������ļ�
*	�ļ����ƣ� keyyg.c
*	˵����ע��
*		����
*   STM32F103C8T6    ҡ��ģ��
*		GND								GND
*		+5								+5v
*		VRX								A0
*		VRY								A1
*		SW								A3
*	�޸ļ�¼����
*	�汾��     ����          ����     ˵��
*	V1.0       22-8-1      	 ��Ԫ��	 
*	Copyright by JIA
*********************************************************************************************************
*/
///////////////////////////////////////////////////////////
/*ͷ�ļ�������*/
#include "keyyg.h"
#include "sys.h" 
#include "delay.h" 
#include "usart.h"
///////////////////////////////////////////////////////////
/*�궨����*/


///////////////////////////////////////////////////////////
/*�ⲿ����������*/

///////////////////////////////////////////////////////////
/*����������*/
void  Adc_Init(void);
u16 	Get_Adc(u8 ch);
u16 	Get_Adc_Average(u8 ch,u8 times);
///////////////////////////////////////////////////////////
void  Adc_Init(void)
{     
		GPIO_InitTypeDef  GPIO_InitStructure;
		ADC_InitTypeDef ADC_InitStruct;

		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_ADC1, ENABLE);     

		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1;                 
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;         
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;         
		GPIO_Init(GPIOA, &GPIO_InitStructure);                    

		RCC_ADCCLKConfig(RCC_PCLK2_Div6);//����ADC��ʱ�ӣ�6��Ƶ��72m/6=12m<14m
		ADC_DeInit(ADC1);//��λADC1

		ADC_InitStruct.ADC_ContinuousConvMode=DISABLE;//����������ת������Ϊ����ת����
		ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;//�����Ҷ���
		ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;//������ʽΪ�����������������ΪNone
		ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;//����ģʽ
		ADC_InitStruct.ADC_NbrOfChannel=2;//һ��ͨ��
		ADC_InitStruct.ADC_ScanConvMode=DISABLE;//ɨ��ģʽ�ر�
		ADC_Init(ADC1,&ADC_InitStruct);

		ADC_Cmd(ADC1,ENABLE);//ʹ��ADC1

		//У׼
		ADC_ResetCalibration(ADC1);//ʹ�ܸ�λУ׼
		while(ADC_GetResetCalibrationStatus(ADC1));//�ȴ�ʹ�ܸ�λУ׼����
		ADC_StartCalibration(ADC1);//��ʼУ׼
		while(ADC_GetCalibrationStatus(ADC1));//�ȴ�����У׼
 
}                  
 //���ADCֵ
 //ch:ͨ��ֵ 0~3
u16 Get_Adc(u8 ch)   
{
		 ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_239Cycles5);
	 /*ADC����ͨ�����ú��� ADC1��ĳ��ͨ������1��ת������������239.5
	 ����ж������ͨ������Ҫ������ô˺���*/
	 
	 ADC_SoftwareStartConvCmd(ADC1,ENABLE);//�������ת������
	 
	 while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));//ת����־λ��ת����ɽ���ѭ��
			 
	 return ADC_GetConversionValue(ADC1);//����ת�����
}

u16 Get_Adc_Average(u8 ch,u8 times)
{
	 u32 temp_val=0;
	 u8 t;
	 for(t=0;t<times;t++)
	 {
			 temp_val+=Get_Adc(ch);
			 delay_ms(5);
	 }
	 return temp_val/times;
}      
void keyyg_init()
{
  GPIO_InitTypeDef GPIO_InitStructure;                    //����GPIO�ṹ��    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);    //ʹ��GPIOCʱ��     
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_3;              //��������    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //��������    
	GPIO_Init(GPIOA,&GPIO_InitStructure);       //��ʼ��GPIOC	
	Adc_Init();
}
/*
*********************************************************************************************************
* �� �� ��: keyyg_scann
* ����˵��: ҡ��ɨ��
* �� �Σ���
* �� �� ֵ: 0-8
*					  	1
*					8	   		 2
*								
*				7			0/9 	 3
*
*			  	6 		  4
*						  5
*********************************************************************************************************
*/

u8 keyyg_scann()
{
    float tempx,tempy;
    u16 adcx,adcy;
	
		if(KEY==0)//��������
    {
        delay_ms(10);
        if(KEY==0)
            return KEY_PRESSED;
    }	
    adcx=Get_Adc_Average(ADC_Channel_0,10);
    adcy=Get_Adc_Average(ADC_Channel_1,10);
    tempx=(float)adcx*(3.3/4096);/*12λadc�������3.3v��ѹ�ֳ�4096��*/
    tempy=(float)adcy*(3.3/4096);
		
		//printf("x=%1.1f,y=%1.1f\r\n",tempx,tempy);

		tempx=(int)tempx;
		tempy=(int)tempy;
		//�����Կɵ�tempx��tempy��ֹʱΪ2.4v
		//�趨tempx��tempyΪ2ʱΪ��ֹ
    if((tempx==2)&&(tempy==2))	return KEY_NODOWN;
		
		//������ҡ�˴����Ϸ�ʱtempx=2.4��tempy=0
		//�趨tempx��[2.0-2.4]֮����tempy<1ʱ��ҡ��Ϊ��
		else if(tempx==2&&tempy<=1)	return KEY_UP;//��
		
		//������ҡ�˴������Ϸ�ʱtempx=3.3��tempy=0
		//�趨tempx=3��tempy<1ʱ��ҡ��Ϊ��
		else if(tempx==3&&tempy<1)	return KEY_UPRIGHT;//����	
		
		//������ҡ�˴����ҷ�ʱtempx=3.3��tempy=2.4
		//�趨tempx=3��tempy=2ʱ��ҡ��Ϊ��
		else if(tempx==3&&tempy==2)	return KEY_RIGHT;//��
		
		//�����Կɵ�tempx��tempyΪ3.3vʱҡ�˴������·�
		//�趨tempx��tempy=3ʱΪ���·�
   else if(tempx==3&&tempy==3) return KEY_DOWNRIGHT;//����
	 
		//������ҡ�˴����·�ʱtempx=2.4��tempy=3.3
		//�趨tempx=2��tempy=3ʱΪ�·�
   else if(tempx==2&&tempy==3) return KEY_DOWN;//��
	 
		//������ҡ�˴������·�ʱtempx=0.0��tempy=3.3
		//�趨tempx<1��tempy=3ʱΪ�·�
   else if(tempx<1&&tempy==3) return KEY_DOWNLEFT;//����
	 
		//������ҡ�˴�����ʱtempx=0.0��tempy=2.4
		//�趨tempx<1��tempy=2ʱΪ��
   else if(tempx<1&&tempy==2) return KEY_LEFT;//��	 

		//������ҡ�˴�����ʱtempx=0.0��tempy=0.0
		//�趨tempx<1��tempy<1Ϊ���Ϸ�
   else if(tempx<1&&tempy<1) return KEY_UPLEFT;//����
	 
	 return KEY_NODOWN;
}

