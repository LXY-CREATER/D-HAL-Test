#include "sys.h"


void WFI_SET(void)
{
	__asm volatile("wfi");
}

void INTX_DISABLE(void)
{
	__asm volatile("cpsid i");
}

void INTX_ENABLE(void)
{
	__asm volatile("cpsie i");
}

void MSR_MSP(u32 addr) 
{
    __asm volatile("msr msp, r0");
    __asm volatile("bx lr");
}


void JTAG_Set(u8 mode)
{
	u32 temp;
	temp=mode;
	temp<<=25;
	RCC->APB2ENR|=1<<0;     
	AFIO->MAPR&=0XF8FFFFFF; 
	AFIO->MAPR|=temp;       
} 
