MEMORY{
	FLASH ( rx ) : ORIGIN = 0x08000000 , LENGTH = 128K
	SRAM  ( xrw) : ORIGIN = 0x20000000 , LENGTH = 64K
	SRAM_DATA  ( xrw) : ORIGIN = 0x20000000 , LENGTH = 0x80
	SRAM_BSS   ( xrw) : ORIGIN = 0x20000080 , LENGTH = 0x80
}

/*FLASH and SRAM are just labels
so I added some custom chunks for .data and .bss
*/

/* output sections */
SECTIONS
{
    /* program code into FLASH */
    .text :
    {
        *(.vector_table)    /* Vector table */
        *(.text.startup)    /* main function */
        *(.text)            /* Program code */
        *(.rodata.*)        /* constant data */
        /*data cannot be included here since we need all data symbols to have addresses in RAM*/
        . = ALIGN(4);
        __text_end__ = .;
    } >FLASH
         
    /* initialized global variables go into RAM, starting at SRAM_DATA */
    .data :
    {
        __data_start__ = .; /*this will point to RAM, specifically to SRAM_DATA*/
        *(.data) /*note that (.data*) does not work, wft?*/
        __data_end__ = .;
    } >SRAM_DATA AT >FLASH
    /*i am not sure if this AT trick is needed here since i append .data binary section manually in Makefile/build*/
    
    /*uninitialized variables goes to RAM, starting ad SRAM_BSS*/
    .bss :
    {
        *(.bss)
        . = ALIGN(4);
        __gcc_RAM_end__ = .;
    } >SRAM_BSS
    
    /* I made separate sections for BSS and DATA with size specification
    for learning puporoses, by shrinking those I can get section oveflow linker errors */
}   
