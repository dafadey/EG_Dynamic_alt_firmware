#include <stdint.h>

#include "utils_common.h"
#include "utils_401.h"

void delay(uint32_t count)
{
	for(volatile uint32_t i=0;i<count;i++)
    NOP
}

int firmware() {
  FLASH_ACR = 0;
  while(FLASH_ACR != 0);
  
  RCC_AHB1ENR = 1UL; // GPIOC GPIOA ENABLE
  DSB
  
  GPIOA->MODER = (1 << 4UL);
  GPIOA->PUPDR = (1 << 4UL);
  DSB
  
  //---MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP--
  
  while(true) {
    delay(3000);
    GPIOA->ODR ^= 0b100;
  }
  
  return 0;
}

unsigned long *vector_table[] __attribute__((section(".vector_table"))) =
{
    (unsigned long *) SRAM_END,    //   0x000 initial stack pointer
    (unsigned long *) firmware,        //   0x004 main as Reset
};
