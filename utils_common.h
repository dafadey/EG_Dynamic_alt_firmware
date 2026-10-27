#pragma once

#define ENTER_CRITICAL asm volatile("cpsid i"); //disable all interrupts
#define LEAVE_CRITICAL asm volatile("cpsie i"); //enable all interrupts
#define DSB asm volatile("dsb"); //data barrier
#define NOP asm volatile("nop"); //data barrier

void stub() {}
void haltstub() {while(1);}
