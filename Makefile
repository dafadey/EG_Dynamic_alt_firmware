all: firmware.bin firmware.sym firmware.dis firmware_direct.dis bootloader go USART_prober

firmware.bin: firmware.elf Makefile
	arm-none-eabi-objcopy -O binary -j .text $< $@.text
	arm-none-eabi-objcopy -O binary -j .data $< $@.data
	cat $@.text > $@
	cat $@.data >> $@
	rm -f $@.text $@.data
#	arm-none-eabi-objcopy -O binary $< $@ # this adds some ARM section and .data shifts and idk what to do with it

firmware.elf: firmware.o linker.ld Makefile
	arm-none-eabi-ld -T linker.ld $< -o $@

firmware.dis: firmware.elf Makefile
	arm-none-eabi-objdump -D $< > $@

firmware_direct.dis: firmware.bin Makefile
	arm-none-eabi-objdump -D -bbinary -marm $< -Mforce-thumb > $@

firmware.sym: firmware.elf Makefile
	arm-none-eabi-nm $< > $@
	
#arm-none-eabi-g++ firmware.cpp -MM
firmware.o : firmware.cpp utils_common.h utils_401.h Makefile
	arm-none-eabi-g++ -Os -c -fno-common -mcpu=cortex-m4 $< -o $@

bootloader: HOST_stm32_bootloader.cpp  HOST_stm32_bootloader.h HOST_uart.h HOST_utils.h HOST_delay.h Makefile
	g++ -O3 $< -o $@

go: HOST_stm32_go.cpp HOST_stm32_bootloader.h HOST_uart.h HOST_utils.h HOST_delay.h Makefile
	g++ -O3 $< -o $@

USART_prober: USART_prober.cpp HOST_uart.h HOST_utils.h HOST_delay.h Makefile
	g++ -O3 $< -o $@

clean:
	rm *.o *.dis *.elf *.bin *.sym bootloader go USART_prober
