MEMORY{
	FLASH ( rx ) : ORIGIN = 0x08000000 , LENGTH = 128K
	SRAM  ( xrw) : ORIGIN = 0x20000000 , LENGTH = 64K
	SRAM_DATA  ( xrw) : ORIGIN = 0x20000000 , LENGTH = 0x80
	SRAM_BSS   ( xrw) : ORIGIN = 0x20000080 , LENGTH = 0x80
}

/*FLASH and SRAM are just labels*/

/* output sections */
SECTIONS
{
    /* program code into FLASH */
    .text :
    {
        *(.vector_table)    /* Vector table */
        *(.text.startup)    /* main function */
        *(.text)            /* Program code */
        *(.rodata.*)
        /**(.data)*/
    } >FLASH
     
    /* uninitialized global and static variables into SRAM */
    .data :
    {
        *(.data)
    } >SRAM_DATA AT >FLASH
    .bss :
    {
        *(.bss)
    } >SRAM_BSS
}   
