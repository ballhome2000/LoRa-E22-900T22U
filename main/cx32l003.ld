
	/* Memory Spaces Definitions */
	MEMORY
	{
/*---------------------------------------------*/
		ISR_VECTOR (rx) : 		ORIGIN = 0x00000000, LENGTH = 0x100
/*---------------------------------------------*/
		CODE (rx) : 			ORIGIN = 0x00000100, LENGTH = 0x0000FFFF - 0x00000100
/*---------------------------------------------*/
		SRAM (xrw) :			ORIGIN = 0x20000000, LENGTH = 0x1000
/*---------------------------------------------*/
	}
	
		/* Sections Definitions */
	SECTIONS
	{
/*---------------------------------------------*/
    	.isr_vector :
    	{
			_isr_vector = . ;
	    	KEEP(*(.isr_vector))
    	} >ISR_VECTOR
/*---------------------------------------------*/
    	.text :
    	{
			. = ALIGN(4);
        	*(.text)
        	*(.text.*)
        	*(.rodata)
        	*(.rodata*)
        	*(.glue_7)
        	*(.glue_7t)
	    	. = ALIGN(4);
   	 		_etext = .;
    	} >CODE

		.data : 
		{
			. = ALIGN(4);
			_sdata = . ;
			*(.data)
			*(.data*)
			. = ALIGN(4);
			_edata = . ;
		} >SRAM AT >CODE

		.bss : 
		{
			. = ALIGN(4);
			_sbss = . ;
			*(.bss)
			*(.bss*)
			*(COMMON)
			_ebss = . ;
		} >SRAM
		_stack = 0x20001000;
/*---------------------------------------------*/
		/DISCARD/ :
		{
			*(.eh_frame)
			*(.comment)
			libc.a ( * )
			libm.a ( * )
			libgcc.a ( * )
			*(.ARM.attributes)
			*(.ARM.exidx*)
		}
	}