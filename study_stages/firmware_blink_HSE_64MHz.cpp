#include <stdint.h>

#include "utils_common.h"
#include "utils_401.h"

int reset(void);

/* vector table */
unsigned long *vector_table[] __attribute__((section(".vector_table"))) =
{
    (unsigned long *) SRAM_END,    //   0x000 initial stack pointer
    (unsigned long *) reset,        //   0x004 main as Reset
    (unsigned long *) haltstub,    //   0x008 halt as NMI
    (unsigned long *) haltstub,    //   0x00C halt as HardFailt
    (unsigned long *) haltstub,    //   0x010 halt as MemManage
    (unsigned long *) haltstub,    //   0x014 halt as BusFault
    (unsigned long *) haltstub,    //   0x018 halt as UsageFault
    (unsigned long *) stub,        //   0x01C RESERVED
    (unsigned long *) stub,        //   0x020 RESERVED
    (unsigned long *) stub,        //   0x024 RESERVED
    (unsigned long *) stub,        //   0x028 RESERVED
    (unsigned long *) haltstub,    //   0x02C halt as SVCall
    (unsigned long *) haltstub,    //   0x030 halt as Debug Monitor
    (unsigned long *) stub,        //   0x034 RESERVED
    (unsigned long *) haltstub,    //   0x038 halt as PendSV
    (unsigned long *) haltstub,    //   0x03C halt as SysTick
    (unsigned long *) stub,        // 0 0x040 halt as WWDG
    (unsigned long *) stub,        // 1 0x044 halt as PVD
    (unsigned long *) stub,        // 2 0x048 halt as Tamper
    (unsigned long *) stub,        // 3 0x04C halt as RTC
    (unsigned long *) stub,        // 4 0x050 halt as FLASH
    (unsigned long *) stub,        // 5 0x054 halt as RCC
    (unsigned long *) stub,      // 6 0x058 halt as EXTI0
    (unsigned long *) stub,        // 7 0x05C halt as EXTI1
    (unsigned long *) stub,        // 8 0x060 halt as EXTI2
    (unsigned long *) stub,        // 9 0x064 halt as EXTI3
    (unsigned long *) stub,        //10 0x068 halt as EXTI4
    (unsigned long *) stub,        //11 0x06C halt as DMA1_cha1
    (unsigned long *) stub,        //12 0x070 halt as DMA1_cha2
    (unsigned long *) stub,        //13 0x074 halt as DMA1_cha3
    (unsigned long *) stub,        //14 0x078 halt as DMA1_cha4
    (unsigned long *) stub,        //15 0x07C halt as DMA1_cha5
    (unsigned long *) stub,        //16 0x080 halt as DMA1_cha6
    (unsigned long *) stub,        //17 0x084 halt as DMA1_cha7
    (unsigned long *) stub,     //18 0x088 halt as ADC1 ADC2
    (unsigned long *) stub,        //19 0x08C halt as CAN1 TX
    (unsigned long *) stub,        //20 0x090 halt as CAN1 RX0
    (unsigned long *) stub,        //21 0x094 halt as CAN1 RX1
    (unsigned long *) stub,        //22 0x098 halt as CAN1 SCE
    (unsigned long *) stub,        //23 0x09C halt as EXTI9-5
    (unsigned long *) stub,        //24 0x0A0 halt as TIM1 Br
    (unsigned long *) stub,        //25 0x0A4 halt as TIM1 Upd
    (unsigned long *) stub,        //26 0x0A8 halt as TIM1 Trig
    (unsigned long *) stub,        //27 0x0AC halt as TIM1 CapCompare
    (unsigned long *) stub,    //28 0x0B0 halt as TIM2
    (unsigned long *) stub,    //29 0x0B4 halt as TIM3
    (unsigned long *) stub,        //30 0x0B8 halt as TIM4
    (unsigned long *) stub,        //31 0x0BC halt as I2C1_EV
    (unsigned long *) stub,        //32 0x0C0 halt as I2C1_ER
    (unsigned long *) stub,        //33 0x0C4 halt as I2C2_EV
    (unsigned long *) stub,        //34 0x0C8 halt as I2C2_ER
    (unsigned long *) stub,        //35 0x0CC halt as SPI1
    (unsigned long *) stub,        //36 0x0D0 halt as SPI2
    (unsigned long *) stub,  //37 0x0D4 halt as USART1
};

void small_delay()
{
	for(volatile uint32_t i=0;i<13;i++);
}

void delay(uint32_t count)
{
	for(volatile uint32_t i=0;i<count;i++)
    small_delay();
}

int reset() {
  //revert to default if we got here from bootloader via go
  //ENTER_CRITICAL
  //SCB_VTOR = 0x08000000; // set interrupt vector table to 0 address in code (not SRAM) segment (basically it is default setting)
  //asm volatile("dsb"); // data sync barrier 
  //LEAVE_CRITICAL
  
  //---setupCLOCK---setupCLOCK---setupCLOCK---setupCLOCK---setupCLOCK---setupCLOCK---setupCLOCK

  FLASH_ACR |= 0b100000010; // prefetch + two wait states
  DSB
  
	//switch to HSE
  RCC_CR |= (1 << 16UL); //HSE ON
  while((RCC_CR & (1 << 17UL)) == 0); // wait for HSE to become stable
   
  //PLLOUT = HSE / M * N / P;
  const uint32_t PLL_M = 4; // M = 4 in [2...63] (8 MHz / 4 = 2 = recomended VCO input)
  const uint32_t PLL_N = 192; // N = in [192...432]
  const uint32_t PLL_P = 0b10; // P in {2(0b00), 4(0b01), 6(0b10), 8(0b11)}
  const uint32_t PLL_Q = 4; // I do not care actually
  //RCC_PLLCFGR &= ~(0x0F437FFF); // clear all that multipliers and PLLSRC
  RCC_PLLCFGR = 0x24000000;
  RCC_PLLCFGR |= PLL_M | (PLL_N << 6UL) | (PLL_P << 16UL) | (PLL_Q << 24UL) + (1 << 22UL); // set multipliers and PLLSRC to HSE
  //PLL is disabled for now but HSE is set run with HSE
  
  //enable PLL and wait for it to become stable
  RCC_CR |= (1 << 24UL);
  while((RCC_CR & (1 << 25UL)) == 0); // wait for PLL to become stable
  
  
  //now switch to PLL source
  RCC_CFGR |= (0b000101 << 10UL) | 2UL; // set APB1(low) to 4 (84Mhz/4) APB2(hi) to 1 (84MHz) and switch to PLL source
  DSB
  //while((RCC_CR & 0b1100) != 0b1000); // wait for switch (not sure if this step is needed)
  
  //---setupPORTS---setupPORTS---setupPORTS---setupPORTS---setupPORTS---setupPORTS---setupPORTS

  RCC_AHB1ENR = 1; // GPIOA ENABLE
  DSB
  
  GPIOA->MODER = (0b01 << 4UL);
  GPIOA->OSPEEDR = (0b10 << 4UL);
  GPIOA->PUPDR = (1 << 4UL);

  //---MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP--
  
  while(true) {
    delay(5000);
    GPIOA->ODR = 0;
    delay(5000);
    GPIOA->ODR = 0b100;
  }
  
}
