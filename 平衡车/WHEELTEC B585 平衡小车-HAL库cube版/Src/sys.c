#include "sys.h"


// ARMCLANG V6 compatible inline assembly (was ARMCC V5 __asm)
__attribute__((naked)) void WFI_SET(void)
{
	__ASM volatile("wfi");
	__ASM volatile("bx lr");
}
// ????????§Ø?
__attribute__((naked)) void INTX_DISABLE(void)
{
	__ASM volatile("cpsid i");
	__ASM volatile("bx lr");
}
// ?????????§Ø?
__attribute__((naked)) void INTX_ENABLE(void)
{
	__ASM volatile("cpsie i");
	__ASM volatile("bx lr");
}
// ??????????
// addr:??????
__attribute__((naked)) void MSR_MSP(u32 addr)
{
	__ASM volatile("msr msp, r0");
	__ASM volatile("bx lr");
}

/**************************************************************************
Function: Set JTAG mode
Input   : mode:JTAG, swd mode settings??00??all enable??01??enable SWD??10??Full shutdown
Output  : none
?????????????JTAG??
????????mode:jtag,swd??????;00,????;01,???SWD;10,????;	
????  ?????
**************************************************************************/
//#define JTAG_SWD_DISABLE   0X02
//#define SWD_ENABLE         0X01
//#define JTAG_SWD_ENABLE    0X00	
void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     //???????????	   
	AFIO->MAPR&=0XF8FFFFFF; //???MAPR??[26:24]
	AFIO->MAPR|=temp;       //????jtag??
} 
