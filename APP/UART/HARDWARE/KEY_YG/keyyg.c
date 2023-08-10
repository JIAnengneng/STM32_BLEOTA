/*
*********************************************************************************************************
*	模块名称： 摇杆模块驱动文件
*	文件名称： keyyg.c
*	说明备注：
*		接线
*   STM32F103C8T6    摇杆模块
*		GND								GND
*		+5								+5v
*		VRX								A0
*		VRY								A1
*		SW								A3
*	修改记录：无
*	版本号     日期          作者     说明
*	V1.0       22-8-1      	 贾元文	 
*	Copyright by JIA
*********************************************************************************************************
*/
///////////////////////////////////////////////////////////
/*头文件包含区*/
#include "keyyg.h"
#include "sys.h" 
#include "delay.h" 
#include "usart.h"
///////////////////////////////////////////////////////////
/*宏定义区*/


///////////////////////////////////////////////////////////
/*外部变量声明区*/

///////////////////////////////////////////////////////////
/*函数声明区*/
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

		RCC_ADCCLKConfig(RCC_PCLK2_Div6);//设置ADC的时钟，6分频，72m/6=12m<14m
		ADC_DeInit(ADC1);//复位ADC1

		ADC_InitStruct.ADC_ContinuousConvMode=DISABLE;//不开启连续转换（故为单次转换）
		ADC_InitStruct.ADC_DataAlign=ADC_DataAlign_Right;//数据右对齐
		ADC_InitStruct.ADC_ExternalTrigConv=ADC_ExternalTrigConv_None;//触发方式为软件触发，所以设置为None
		ADC_InitStruct.ADC_Mode=ADC_Mode_Independent;//独立模式
		ADC_InitStruct.ADC_NbrOfChannel=2;//一个通道
		ADC_InitStruct.ADC_ScanConvMode=DISABLE;//扫描模式关闭
		ADC_Init(ADC1,&ADC_InitStruct);

		ADC_Cmd(ADC1,ENABLE);//使能ADC1

		//校准
		ADC_ResetCalibration(ADC1);//使能复位校准
		while(ADC_GetResetCalibrationStatus(ADC1));//等待使能复位校准结束
		ADC_StartCalibration(ADC1);//开始校准
		while(ADC_GetCalibrationStatus(ADC1));//等待结束校准
 
}                  
 //获得ADC值
 //ch:通道值 0~3
u16 Get_Adc(u8 ch)   
{
		 ADC_RegularChannelConfig(ADC1,ch,1,ADC_SampleTime_239Cycles5);
	 /*ADC规则通道配置函数 ADC1的某个通道，第1个转换，采样周期239.5
	 如果有多个规则通道，需要多次配置此函数*/
	 
	 ADC_SoftwareStartConvCmd(ADC1,ENABLE);//开启软件转换启动
	 
	 while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));//转换标志位，转换完成结束循环
			 
	 return ADC_GetConversionValue(ADC1);//返回转换结果
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
  GPIO_InitTypeDef GPIO_InitStructure;                    //定义GPIO结构体    
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);    //使能GPIOC时钟     
	GPIO_InitStructure.GPIO_Pin =GPIO_Pin_3;              //配置引脚    
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;           //上拉输入    
	GPIO_Init(GPIOA,&GPIO_InitStructure);       //初始化GPIOC	
	Adc_Init();
}
/*
*********************************************************************************************************
* 函 数 名: keyyg_scann
* 功能说明: 摇杆扫描
* 形 参：无
* 返 回 值: 0-8
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
	
		if(KEY==0)//按键按下
    {
        delay_ms(10);
        if(KEY==0)
            return KEY_PRESSED;
    }	
    adcx=Get_Adc_Average(ADC_Channel_0,10);
    adcy=Get_Adc_Average(ADC_Channel_1,10);
    tempx=(float)adcx*(3.3/4096);/*12位adc所以最大将3.3v电压分成4096份*/
    tempy=(float)adcy*(3.3/4096);
		
		//printf("x=%1.1f,y=%1.1f\r\n",tempx,tempy);

		tempx=(int)tempx;
		tempy=(int)tempy;
		//经测试可得tempx和tempy静止时为2.4v
		//设定tempx和tempy为2时为静止
    if((tempx==2)&&(tempy==2))	return KEY_NODOWN;
		
		//经测试摇杆处于上方时tempx=2.4，tempy=0
		//设定tempx在[2.0-2.4]之间且tempy<1时，摇杆为上
		else if(tempx==2&&tempy<=1)	return KEY_UP;//上
		
		//经测试摇杆处于右上方时tempx=3.3，tempy=0
		//设定tempx=3且tempy<1时，摇杆为上
		else if(tempx==3&&tempy<1)	return KEY_UPRIGHT;//右上	
		
		//经测试摇杆处于右方时tempx=3.3，tempy=2.4
		//设定tempx=3且tempy=2时，摇杆为右
		else if(tempx==3&&tempy==2)	return KEY_RIGHT;//右
		
		//经测试可得tempx和tempy为3.3v时摇杆处于右下方
		//设定tempx和tempy=3时为右下方
   else if(tempx==3&&tempy==3) return KEY_DOWNRIGHT;//右下
	 
		//经测试摇杆处于下方时tempx=2.4，tempy=3.3
		//设定tempx=2，tempy=3时为下方
   else if(tempx==2&&tempy==3) return KEY_DOWN;//下
	 
		//经测试摇杆处于左下方时tempx=0.0，tempy=3.3
		//设定tempx<1且tempy=3时为下方
   else if(tempx<1&&tempy==3) return KEY_DOWNLEFT;//左下
	 
		//经测试摇杆处于左方时tempx=0.0，tempy=2.4
		//设定tempx<1且tempy=2时为左方
   else if(tempx<1&&tempy==2) return KEY_LEFT;//左	 

		//经测试摇杆处于左方时tempx=0.0，tempy=0.0
		//设定tempx<1且tempy<1为左上方
   else if(tempx<1&&tempy<1) return KEY_UPLEFT;//左上
	 
	 return KEY_NODOWN;
}

