#pragma once

/* memory and peripheral start addresses */
#define FLASH_BASE      0x08000000
#define SRAM_BASE       0x20000000
#define PERIPH_BASE     0x40000000
#define FLASH_INTERFACE_BASE       (PERIPH_BASE + 0x23C00)
#define RCC_BASE        (PERIPH_BASE + 0x23800)
 
/* work out end of RAM address as initial stack pointer */
#define SRAM_SIZE       64*1024     // STM32F401RBT6 has 64 Kbyte of RAM
#define SRAM_END        (SRAM_BASE + SRAM_SIZE)
 
  #define OUTPUT_MODE     (0x02) // output mode: gp push-pull + 2MHz
  #define APPOUT_MODE     (0x0b) // output mode: b = [CNF]10 MODE[11]  50MHz + Alternate Push-pull
  #define APPOUT_MODE10   (0x09) // output mode: 9 = [CNF]10 MODE[01]  10MHz + Alternate Push-pull
  #define FINPUT_MODE     (0x04) // output mode: 4 = [CNF]01 MODE[00]  Input mode + floating input
  #define PINPUT_MODE     (0x08) // output mode: 8 = [CNF]10 MODE[00]  Input mode + pull up / pull down mode
 



#define FLASH_ACR       (*(volatile unsigned long*) FLASH_INTERFACE_BASE)

  /* RCC peripheral addresses*/

#define RCC_CR          (*(volatile unsigned long*)(RCC_BASE + 0x00))
#define RCC_PLLCFGR     (*(volatile unsigned long*)(RCC_BASE + 0x04))
#define RCC_CFGR        (*(volatile unsigned long*)(RCC_BASE + 0x08))
#define RCC_BDCR        (*(volatile unsigned long*)(RCC_BASE + 0x70))
#define RCC_AHB1ENR     (*(volatile unsigned long*)(RCC_BASE + 0x30))
#define RCC_APB1ENR     (*(volatile unsigned long*)(RCC_BASE + 0x40))
#define RCC_APB2ENR     (*(volatile unsigned long*)(RCC_BASE + 0x44))
#define RCC_APB1RSTR    (*(volatile unsigned long*)(RCC_BASE + 0x20))
#define RCC_APB2RSTR    (*(volatile unsigned long*)(RCC_BASE + 0x24))

/*
RCC_CIR 0x0C
RCC_AHB1RSTR 0x10
RCC_AHB2RSTR 0x14
RCC_AHB1ENR 0x30
RCC_AHB2ENR 0x34
RCC_AHB1LPENR 0x50
RCC_AHB2LPENR 0x54
RCC_APB1LPENR 0x60
RCC_APB2LPENR 0x64
RCC_CSR 0x74
RCC_SSCGR 0x80
RCC_PLLI2SCFGR 0x84
RCC_DCKCFGR 0x8C
*/

#define EXTI_BASE       (PERIPH_BASE + 0x13C00)
#define EXTI_IMR        (*(volatile unsigned long*)(EXTI_BASE))         // interrupts
#define EXTI_EMR        (*(volatile unsigned long*)(EXTI_BASE + 0x04))  // events
#define EXTI_RTSR       (*(volatile unsigned long*)(EXTI_BASE + 0x08))
#define EXTI_FTSR       (*(volatile unsigned long*)(EXTI_BASE + 0x0C))
#define EXTI_SWIER      (*(volatile unsigned long*)(EXTI_BASE + 0x10))
#define EXTI_PR         (*(volatile unsigned long*)(EXTI_BASE + 0x14))

#define SYSCFG          (PERIPH_BASE + 0x13800)
#define SYSCFG_EXTICR1    (*(volatile unsigned long*)(SYSCFG + 0x08))

//CORTEX-M4 NTERNAL Peripherials
#define NVIC_BASE       0xE000E000
//enable disable
#define NVIC_ISER0      (*(volatile unsigned long*)(NVIC_BASE + 0x100))
#define NVIC_ISER1      (*(volatile unsigned long*)(NVIC_BASE + 0x104))
#define NVIC_ICER0      (*(volatile unsigned long*)(NVIC_BASE + 0x180))
#define NVIC_ICER1      (*(volatile unsigned long*)(NVIC_BASE + 0x184))

//pending
#define NVIC_ISPR0      (*(volatile unsigned long*)(NVIC_BASE + 0x200))
#define NVIC_ISPR1      (*(volatile unsigned long*)(NVIC_BASE + 0x204))
#define NVIC_ICPR0      (*(volatile unsigned long*)(NVIC_BASE + 0x280))
#define NVIC_ICPR1      (*(volatile unsigned long*)(NVIC_BASE + 0x284))

//status
#define NVIC_IABR0      (*(volatile unsigned long*)(NVIC_BASE + 0x300))

//priority
#define NVIC_IP         ((uint8_t*)(NVIC_BASE + 0x400))

//Software trigger interrupt register
#define NVIC_STIR       (*(volatile unsigned long*)(NVIC_BASE + 0xE00))

#define SCB_BASE        0xE000ED00
#define SCB_VTOR        (*(volatile unsigned long*)(SCB_BASE+0x08))

#define GPIOA_BASE      (PERIPH_BASE + 0x20000)
#define GPIOB_BASE      (PERIPH_BASE + 0x20400)
#define GPIOC_BASE      (PERIPH_BASE + 0x20800)

typedef struct 
{
  uint32_t MODER;
  uint32_t OTYPER;
  uint32_t OSPEEDR;
  uint32_t PUPDR;
  uint32_t IDR;
  uint32_t ODR;
  uint32_t BSRR;
  uint32_t LCKR;
  uint32_t AFRL;
  uint32_t AFRH;
} GPIO;

#define GPIOA           ((volatile GPIO*) GPIOA_BASE)
#define GPIOB           ((volatile GPIO*) GPIOB_BASE)
#define GPIOC           ((volatile GPIO*) GPIOC_BASE)

typedef struct {
  uint32_t CR1; //(BASE)) //p338
  uint32_t CR2; //(BASE+0x04)) //p339
  uint32_t SMCR; //(BASE+0x08)) //p342
  uint32_t DIER; //(BASE+0x0C)) //p344
  uint32_t SR; //(BASE+0x10)) //p346
  uint32_t EGR; //(BASE+0x14)) //p348
  uint32_t CCMR1; //(BASE+0x18)) //p349
  uint32_t CCMR2; //(BASE+0x1C)) //p351
  uint32_t CCER; //(BASE+0x20)) //p353
  uint32_t CNT; //(BASE+0x24)) //p356 counter value 16bit
  uint32_t PSC; //(BASE+0x28)) //p356 prescaler (x+1) 16bit
  uint32_t ARR; //(BASE+0x2C)) //p356 auto reload value 16bit
  uint32_t RCR; //(BASE+0x30)) //p357 repetition counter 8bit //reserved
  uint32_t CCR1; //(BASE+0x34)) //p357
  uint32_t CCR2; //(BASE+0x38)) //p358
  uint32_t CCR3; //(BASE+0x3C)) //p358
  uint32_t CCR4; //(BASE+0x40)) //p359
  uint32_t BDTR; //(BASE+0x44)) //p359 // reserved
  uint32_t DCR; //(BASE+0x48)) //p361
  uint32_t DMAR; //(BASE+0x4C)) //p362 DMA address for full transfer
} TIM;

#define TIM4      ((volatile TIM*)(PERIPH_BASE+0x800))
