all: firmware.bin firmware.dis firmware_direct.dis bootloader go USART_prober

firmware.bin: firmware.elf
	arm-none-eabi-objcopy -O binary -j .text $< $@

firmware.elf: firmware.o linker.ld
	arm-none-eabi-ld -T linker.ld $< -o $@

firmware.dis: firmware.elf
	arm-none-eabi-objdump -D $< > $@

firmware_direct.dis: firmware.bin
	arm-none-eabi-objdump -D -bbinary -marm $< -Mforce-thumb > $@

#arm-none-eabi-g++ firmware.cpp -MM
firmware.o : firmware.cpp utils_common.h utils_401.h
	arm-none-eabi-g++ -Os -c -fno-common -mcpu=cortex-m4 $< -o $@

bootloader: HOST_stm32_bootloader.cpp  HOST_stm32_bootloader.h HOST_uart.h HOST_utils.h HOST_delay.h
	g++ -O3 $< -o $@

go: HOST_stm32_go.cpp HOST_stm32_bootloader.h HOST_uart.h HOST_utils.h HOST_delay.h
	g++ -O3 $< -o $@

USART_prober: USART_prober.cpp HOST_uart.h HOST_utils.h HOST_delay.h
	g++ -O3 $< -o $@

clean:
	rm *.o *.dis *.elf *.bin bootloader go USART_prober
