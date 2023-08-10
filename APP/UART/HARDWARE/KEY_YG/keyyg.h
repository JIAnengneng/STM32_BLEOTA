/*
*********************************************************************************************************
*	模块名称： 
*	文件名称：
*	说明备注：
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
#ifndef __KEYYG_H
#define __KEYYG_H

///////////////////////////////////////////////////////////
/*头文件包含区*/
#include "sys.h"
///////////////////////////////////////////////////////////
/*宏定义区*/
#define KEY GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)

#define KEY_UP 				1
#define KEY_UPRIGHT 	2
#define KEY_RIGHT 		3
#define KEY_DOWNRIGHT 4

#define KEY_DOWN			5
#define KEY_DOWNLEFT	6
#define KEY_LEFT 			7
#define KEY_UPLEFT		8

#define KEY_PRESSED 	9

#define KEY_NODOWN 		0

///////////////////////////////////////////////////////////
/*外部变量声明区*/

///////////////////////////////////////////////////////////
/*函数声明区*/
void keyyg_init(void);
u8 keyyg_scann(void);
///////////////////////////////////////////////////////////
#endif
