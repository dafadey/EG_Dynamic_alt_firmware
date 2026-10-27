MEMORY{
	FLASH ( rx ) : ORIGIN = 0x08000000 , LENGTH = 128K
	SRAM ( xrw) : ORIGIN = 0x20000000 , LENGTH = 64K
}

/* output sections */
SECTIONS
{
    /* program code into FLASH */
    .text :
    {
        *(.vector_table)    /* Vector table */
        *(.text)            /* Program code */
        *(.rodata)
        *(.data)
    } >FLASH
 
    /* uninitialized global and static variables (which 
       we don't have any in this example) into SRAM */
    .data :
    {
        *(.data)
    } >SRAM
}   
