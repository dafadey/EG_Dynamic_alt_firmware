#include <stdint.h>

#include "utils_common.h"
#include "utils_401.h"

#define HEAP (SRAM_BASE + 0x100UL)

/* HEAP:
 * DMA_tx (32 uint8_t), DMA_rx (32 uint8_t), IA_TABLE (16*16 int_16), IA_TABLE_GAS (16*16 int_16), RPM uint16, MAP utin16, TPS uint16 
 */

volatile uint32_t Ga=1;
volatile uint32_t Gb=2;
volatile uint32_t Gc=3;
volatile uint32_t Gd=4;
volatile uint32_t Ge=5;
volatile uint32_t Gj=6;
volatile uint32_t Gh=7;
volatile uint32_t Gi=8;
volatile uint32_t Gg=9;
volatile uint32_t Gk=10;
volatile uint32_t Gl=11;
volatile uint32_t Gm=12;
volatile uint32_t Gn=13;

const char* msg="asjdhfskdjfhksdjfhksdhgfjsagfksjdgfksjdfhshdfasjdfgaskhdfgasdjhfgasjgfjasdgfajsd";
const char* msg0="XXXXXXX";

uint32_t DMA_tx_sz;
uint32_t DMA_rx_sz;
uint32_t nx;
uint32_t ny;

uint8_t* DMA_rx;
uint8_t* DMA_tx;
int16_t* IA_TABLE_NORMAL; // section for dynamic advance correction vs (MAP, RPM). this is convenient for tuning. have to be written to flash (internal or external) after tuning is done   
int16_t* IA_TABLE_GAS; // same for gas map    
uint16_t* RPM; // EXTI fills this up using TIMER value
uint16_t* MAP; // comes from ADC, MAP line
uint16_t* TPS; // comes from ADC, TPS line

void allocate_memory() {
  ENTER_CRITICAL // in case reallocation is executed during normal operation
  DMA_rx = (uint8_t*) HEAP;
  DMA_tx = DMA_rx + DMA_rx_sz; 
  IA_TABLE_NORMAL = (int16_t*) DMA_tx + DMA_tx_sz;
  IA_TABLE_GAS = IA_TABLE_NORMAL + nx * ny * sizeof(uint16_t);    
  RPM = (uint16_t*) IA_TABLE_GAS + nx * ny * sizeof(uint16_t);
  MAP = RPM + sizeof(uint16_t);
  TPS = MAP + sizeof(uint16_t);
  LEAVE_CRITICAL
}

int reset(void);
void CPS(void); // crankshaft position sensor
void TIM_int(void); // timer interrupt

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
    (unsigned long *) CPS,         // 6 0x058 halt as EXTI0
//    (unsigned long *) stub,         // 6 0x058 halt as EXTI0
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
    (unsigned long *) stub,        //18 0x088 halt as ADC1 ADC2
    (unsigned long *) stub,        //19 0x08C halt as CAN1 TX
    (unsigned long *) stub,        //20 0x090 halt as CAN1 RX0
    (unsigned long *) stub,        //21 0x094 halt as CAN1 RX1
    (unsigned long *) stub,        //22 0x098 halt as CAN1 SCE
    (unsigned long *) stub,        //23 0x09C halt as EXTI9-5
    (unsigned long *) stub,        //24 0x0A0 halt as TIM1 Br
    (unsigned long *) stub,        //25 0x0A4 halt as TIM1 Upd
    (unsigned long *) stub,        //26 0x0A8 halt as TIM1 Trig
    (unsigned long *) stub,        //27 0x0AC halt as TIM1 CapCompare
    (unsigned long *) stub,        //28 0x0B0 halt as TIM2
    (unsigned long *) stub,        //29 0x0B4 halt as TIM3
    (unsigned long *) TIM_int,     //30 0x0B8 halt as TIM4
    (unsigned long *) stub,        //31 0x0BC halt as I2C1_EV
    (unsigned long *) stub,        //32 0x0C0 halt as I2C1_ER
    (unsigned long *) stub,        //33 0x0C4 halt as I2C2_EV
    (unsigned long *) stub,        //34 0x0C8 halt as I2C2_ER
    (unsigned long *) stub,        //35 0x0CC halt as SPI1
    (unsigned long *) stub,        //36 0x0D0 halt as SPI2
    (unsigned long *) stub,        //37 0x0D4 halt as USART1
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

void TIM_int() {
  static int timState=0;
  TIM4->SR = 0;
  
  if(timState == 0) {
    GPIOA->ODR = 0;
    timState = 1;
  } else {
    GPIOA->ODR = 0b100;
    timState = 0;
  }
}

void CPS() {
  static int count=0;
	static int state=0;
  if(EXTI_PR & 1) {
/*
This register is write-1-to-clear (W1C), so never use |= on it — you would clear all pending flags that happen to be 1, including ones from other lines that arrived between the read and the write. Use a direct assignment with only the bit you intend to clear.
*/

    if(count % 11 == 0) {
      if(state % 2 == 0 )
        GPIOB->ODR |= (1 << 12UL);
      else
        GPIOB->ODR &= ~(1 << 12UL);
    }
    count++;
    state++;
		EXTI_PR = 1; // reset corresponding pending bit (first bit in our case)
  }
}

int main();

int reset() {
  return main();
}

extern uint8_t* __data;
extern uint8_t* __data_end__;

int main() {
  /*
  {
    uint8_t* from = __data;
    uint8_t* to = (uint8_t*) SRAM_BASE;
    for(; from < __data_end__; from++, to++)
      *to = *from;
  }
  */
  DMA_tx_sz = 32;
  DMA_rx_sz = 32;
  nx = 16;
  ny = 16;
  
  allocate_memory();
  
  //revert to default if we got here from bootloader via go
  ENTER_CRITICAL
  SCB_VTOR = 0x08000000; // set interrupt vector table to 0 address in code (not SRAM) segment (basically it is default setting)
  asm volatile("dsb"); // data sync barrier 
  LEAVE_CRITICAL
  
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
  const uint32_t PLL_Q = 12; // I do not care actually
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

  RCC_AHB1ENR = 3 | (1 << 22UL); // GPIOA, GPIOB, DMA2 ENABLE
  RCC_APB2ENR = (1 << 14UL) | (1 << 4UL); // ENABLE SYSCFG, USART1
  RCC_APB1ENR = (1 << 2UL); // ENABLE TIM4
  DSB
  
  GPIOA->MODER = (0b01 << 4UL);
  GPIOA->OSPEEDR = (0b10 << 4UL);
  GPIOA->PUPDR = (1 << 4UL);

  GPIOB->MODER = (0b01 << (12*2));
  GPIOB->OSPEEDR = (0b10 << (12*2));
  GPIOB->PUPDR = (1 << (12*2));

  //PA0 as input
  GPIOA->MODER &= ~(3U); // input mode
  GPIOA->PUPDR &= ~(3U); // reset
  GPIOA->PUPDR |=  (1U); // pull-up
  DSB

  SYSCFG_EXTICR1 = 0; //? it is already 0, no?
  DSB

  EXTI_IMR |= 1;
  EXTI_RTSR |= 1;
  EXTI_FTSR |= 1;

  TIM4->CR1 = (uint16_t) 0;
  TIM4->CR1 |= (uint16_t) 0b10000100;
  TIM4->DIER = (uint16_t) 1;
  TIM4->PSC = (uint16_t) 0xf; // 64MHz / (PSC(15)+1) / APB1PRE(4) * (APB1PRE == 1 ? 1 : 2) = 2 MHz
  TIM4->ARR = 31;

  NVIC_ISER0 = ((1 << 6UL) | (1 << 30UL)); // 6 is EXTI0, 30 is TIM4 
  NVIC_ICPR0 = NVIC_ISER0;
  NVIC_IP[6UL] = (uint8_t) 80;
  DSB

  //configure USART
  USART1->CR1 = 0UL;
	USART1->CR2 = 0UL;
	USART1->CR3 = 0UL;
	USART1->GTPR = 0UL;
  GPIOA->MODER &= ~(3U << (9*2)); // PA9 reset
  GPIOA->MODER &= ~(3U << (10*2)); // PA10 reset (input mode)
  GPIOA->MODER |= (2U << (9*2)); // PA9 alternate
  GPIOA->MODER |= (2U << (10*2)); // PA10 alternate
  GPIOA->AFRH |= (0b0111 << 4); // AF7 to AFRH9
  GPIOA->AFRH |= (0b0111 << 8); // AF7 to AFRH10

  DSB
  delay(5000);
  USART1->BRR = 34 * 16 + 11; // 115200 : 555 Mantissa = 34, Fraction = 11
  //USART1->BRR = 69 * 16 + 7; // 56700 : 1111
  //USART1->BRR = 104 * 16 + 2; // 19200 : 1666
  //Tx/Rx baud = fCK / (16 * USARTDIV) = fCK / (16 * DIV_Mantissa + DIV_Fraction)
  //Tx/Rx baud = 115315
  USART1->CR1 |= 0b1010000001100UL; // M, PCE, TX, RX (I do not understand why M is enabled, without it it sort of works but receiver with parity control does not receive correct data)
  USART1->CR3 |= (1 << 7UL) | (1 << 6UL); // enable DMA transmitter, and DMA receiver
  DSB
  delay(5000);
	USART1->CR1 |= 1 << 13UL; //enable USART
  
  //DMA stream7 cha4
  DMA2->streams[7].SCR = 0;
  DMA2->streams[7].SCR = (0b100 << 25UL) | (0b01 << 6UL) | (1 << 10UL);//| (1 << 8UL);// | (1 << 5UL); // select channel 4 for stream 7 (USART1 TX) , direction: memory-to-peripherial, memory increment, circ, periferial controls the transfer
  DMA2->streams[7].SPAR = (uint32_t) &(USART1->DR);
  DMA2->streams[7].SM0AR = (uint32_t) DMA_tx;
  //0001 0100 0000 1100;
  
  DMA2->streams[5].SCR = 0;
  DMA2->streams[5].SCR = (0b100 << 25UL) | (0b00 << 6UL) | (1 << 10UL);//| (1 << 8UL);// | (1 << 5UL); // select channel 4 for stream 7 (USART1 TX) , direction: memory-to-peripherial, memory increment, circ, periferial controls the transfer
  DMA2->streams[5].SPAR = (uint32_t) &(USART1->DR);
  DMA2->streams[5].SM0AR = (uint32_t) DMA_rx;
  
  
  //---MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP----MAIN-LOOP--
  
  TIM4->CR1 |= (uint16_t) 1;

  DMA_tx[0]  = ((uint32_t)(&(DMA2->streams[7].SCR)) >> 24) & 0xff;
  DMA_tx[1]  = ((uint32_t)(&(DMA2->streams[7].SCR)) >> 16) & 0xff;
  DMA_tx[2]  = ((uint32_t)(&(DMA2->streams[7].SCR)) >> 8) & 0xff;
  DMA_tx[3]  = (uint32_t)(&(DMA2->streams[7].SCR)) & 0xff;


  DMA_tx[4]  = ((uint32_t)(&(DMA2->streams[7].SNDTR)) >> 24) & 0xff;
  DMA_tx[5]  = ((uint32_t)(&(DMA2->streams[7].SNDTR)) >> 16) & 0xff;
  DMA_tx[6]  = ((uint32_t)(&(DMA2->streams[7].SNDTR)) >> 8) & 0xff;
  DMA_tx[7]  = (uint32_t)(&(DMA2->streams[7].SNDTR)) & 0xff;

  DMA_tx[8]  = ((uint32_t)(&(DMA2->streams[7].SPAR)) >> 24) & 0xff;
  DMA_tx[9]  = ((uint32_t)(&(DMA2->streams[7].SPAR)) >> 16) & 0xff;
  DMA_tx[10] = ((uint32_t)(&(DMA2->streams[7].SPAR)) >> 8) & 0xff;
  DMA_tx[11] = (uint32_t)(&(DMA2->streams[7].SPAR)) & 0xff;

  DMA_tx[12] = ((uint32_t)(&(DMA2->streams[7].SM0AR)) >> 24) & 0xff;
  DMA_tx[13] = ((uint32_t)(&(DMA2->streams[7].SM0AR)) >> 16) & 0xff;
  DMA_tx[14] = ((uint32_t)(&(DMA2->streams[7].SM0AR)) >> 8) & 0xff;
  DMA_tx[15] = (uint32_t)(&(DMA2->streams[7].SM0AR)) & 0xff;

  NOP
  NOP
  NOP
  NOP
  NOP
  NOP
  NOP
  /*
  for(int i=0;i<16;i++)
    DMA_tx[i] = msg[i];
  for(int i=0;i<3;i++)
    DMA_tx[i+16] = msg0[i];

  for(int i=0;i<DMA_rx_sz;i++)
    DMA_rx[i] = ' ';
  */

/*  
  DMA_tx[0]  = 0b00100001; //0x21
  DMA_tx[1]  = 0b00010001; //0x11
  DMA_tx[2]  = 0b00001001; //0x09
  DMA_tx[3]  = 0b00000101; //0x05 1
  DMA_tx[4]  = 0b00001001; //0x09 2
  DMA_tx[5]  = 0b00010001; //0x11 3
  DMA_tx[6]  = 0b00100001; //0x21 4
  DMA_tx[7]  = 0b01000001; //0x41 5
  DMA_tx[8]  = 0b10100001; //0xA1 
  DMA_tx[9]  = 0b10010001; //0x91
  DMA_tx[10] = 0b10001001; //0x89
  DMA_tx[11] = 0b10000101; //0x85
  DMA_tx[12] = 0b10001001; //0x89
  DMA_tx[13] = 0b10010001; //0x91
  DMA_tx[14] = 0b10100001; //0xA1
  DMA_tx[15] = 0b11000001; //0xC1
*/

  //int sent_id=0;

  bool USARTrxReady = false;
  
  int SNDTR5_prev = DMA2->streams[5].SNDTR;
  
  while(true) {
    int off=11000;
    /*
    while((USART1->SR & (1 << 7UL)) == 0);
    USART1->DR = DMA_tx[sent_id];
    sent_id++;
    if(sent_id == 16)
      sent_id = 0;
    */

/*
      if((DMA2->streams[7].SCR & 1) == 0) {
        DMA2->streams[7].SNDTR = 16;
        DMA2->HIFCR |= ((0b1111 << 24UL) | (1 << 22UL)); // huh it is really important to clear those! 
        DMA2->streams[7].SCR |= 1; // enable stream
        while((DMA2->streams[7].SCR & 1) == 0); //not sure if needed
        off = 5000;
      }
*/


    if((DMA2->streams[5].SCR & 1) == 0) {
      DMA2->streams[5].SNDTR = 16;
      DMA2->HIFCR |= ((0b1111 << 8UL) | (1 << 6UL)); // huh it is really important to clear those! 
      DMA2->streams[5].SCR |= 1; // enable stream
      while((DMA2->streams[5].SCR & 1) == 0); //not sure if needed

      if(USARTrxReady) {
        for(int i=0;i<16;i++) {
          if(DMA_rx[i] >= 97)
            DMA_tx[i] = DMA_rx[i] - 32;
          else
            DMA_tx[i] = DMA_rx[i] + 32;
        }
      }

      if((DMA2->streams[7].SCR & 1) == 0 && USARTrxReady) {
        //USART1->SR &= ~(1<<6UL); // clear the TC
        DMA2->streams[7].SNDTR = 16;
        DMA2->HIFCR |= ((0b1111 << 24UL) | (1 << 22UL)); // huh it is really important to clear those! 
        DMA2->streams[7].SCR |= 1; // enable stream
        while((DMA2->streams[7].SCR & 1) == 0); //not sure if needed
      }

      off = 5000;
      USARTrxReady = true;
    }


/*
    if(DMA2->streams[5].SNDTR != SNDTR5_prev)
    {
      if(USARTrxReady) {
        for(int i=0;i<16;i++) {
          if(DMA_rx[i] >= 97)
            DMA_tx[i] = DMA_rx[i] - 32;
          else
            DMA_tx[i] = DMA_rx[i] + 32;
        }
      }
      
      if((DMA2->streams[7].SCR & 1) == 0 && USARTrxReady) {
        //USART1->SR &= ~(1<<6UL); // clear the TC
        DMA2->streams[7].SNDTR = 16;
        DMA2->HIFCR |= ((0b1111 << 24UL) | (1 << 22UL)); // huh it is really important to clear those! 
        DMA2->streams[7].SCR |= 1; // enable stream
        while((DMA2->streams[7].SCR & 1) == 0); //not sure if needed
      }
      
      SNDTR5_prev = DMA2->streams[5].SNDTR;
    }
*/

/*
    if(USART1->SR & (1<<5UL)) {

      DMA_tx[0] = USART1->DR;

      if((DMA2->streams[7].SCR & 1) == 0 && USARTrxReady) {
        //USART1->SR &= ~(1<<6UL); // clear the TC
        DMA2->streams[7].SNDTR = 16;
        DMA2->HIFCR |= ((0b1111 << 24UL) | (1 << 22UL)); // huh it is really important to clear those! 
        DMA2->streams[7].SCR |= 1; // enable stream
        while((DMA2->streams[7].SCR & 1) == 0); //not sure if needed
      }

      off = 5000;
      USARTrxReady = true;
    }
*/


    delay(5000+DMA2->streams[5].SNDTR*1333);

    GPIOB->ODR &= ~(1 << 12UL);
    delay(off);

    GPIOB->ODR |= (1 << 12UL);
  }
  
}
