/*
 * HyperCube OS
 * (c) Sergej Schumilo, 2019 <sergej@schumilo.de> 
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 */ 

#include "system.h"
#include "libk.h"
#include "mboot.h"
#include "serial.h"
#include "mem.h"
#include "vga.h"
#include "tty.h"
#include "pci.h"
#include "panic.h"
#include "io.h"
#include "msr.h"
#include "test.h"
#include "acpi.h"
#include "cpuid.h"
#include "apic.h"
#include "pic.h"
#include "smp.h"
#include "pio.h"
#include "mmio.h"

#include "fuzz.h"
#include "../../config.h"

extern uintptr_t pci_ecam_ptr;

//#define HYPERCUBE

#define STR_PREFIX	" [KERN] "

#ifdef SHARKOS
extern uint8_t _binary_misc_logo_shark_bmp_start;
#else
extern uint8_t _binary_misc_logo_bmp_start;
#endif

fuzzer_state_t* fuzzer;


static void kernel_ready(void){
	terminal_setcolor(DEFAULT_COLOR);

	terminal_drawbmp(&_binary_misc_logo_bmp_start);
	terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_BROWN));
	terminal_setpos(54, 24);
	terminal_writestring(" Hypercube OS ");
	terminal_setpos(0, 24);
	return;
}

static inline void kernel_init_console(void){
	terminal_initialize(DEFAULT_COLOR);
	serial_enable(SERIAL_PORT_A);
	terminal_setcolor(SUCCESS_COLOR);
	printf(STR_PREFIX"Booting Hypercube OS Kernel...\n\r");

#ifdef ENABLE_TTY_VGA
		kernel_ready();
#endif
#ifdef DEBUG_PRINTF
	//serial_printf(SERIAL_PORT_A, " Booting Hypercube OS Kernel...\n\r");
	serial_printf(SERIAL_PORT_A,"-----------------------------------------\n\r");
	serial_printf(SERIAL_PORT_A,"                 #######           \n\r");
	serial_printf(SERIAL_PORT_A,"             ####       ####       \n\r");
	serial_printf(SERIAL_PORT_A,"           ##               ##     \n\r");
	serial_printf(SERIAL_PORT_A,"         ##                   ##   \n\r");
	serial_printf(SERIAL_PORT_A,"        #                       #  \n\r");
	serial_printf(SERIAL_PORT_A,"       #  ####        ######     # \n\r");
	serial_printf(SERIAL_PORT_A,"      #  #   ##      #    ###     #\n\r");
	serial_printf(SERIAL_PORT_A,"      # #      #     #       #    #\n\r");
	serial_printf(SERIAL_PORT_A,"      # #########    #########    #\n\r");
	serial_printf(SERIAL_PORT_A,"      #                           #\n\r");
	serial_printf(SERIAL_PORT_A,"      #                           #\n\r");
	serial_printf(SERIAL_PORT_A,"      #   ####################    #\n\r");
	serial_printf(SERIAL_PORT_A,"       #   #        #####   #    # \n\r");
	serial_printf(SERIAL_PORT_A,"       #    #     ##    ## ##    # \n\r");
	serial_printf(SERIAL_PORT_A,"        #    ##  #        ##    #  \n\r");
	serial_printf(SERIAL_PORT_A,"         ##    ##       ##    ##   \n\r");
	serial_printf(SERIAL_PORT_A,"           ##    #######    ##     \n\r");
	serial_printf(SERIAL_PORT_A,"             ###         ###       \n\r");
	serial_printf(SERIAL_PORT_A,"                #########          \n\r");
	serial_printf(SERIAL_PORT_A,"-----------------------------------------\n\r\n\r");
#endif

	terminal_setcolor(DEFAULT_COLOR);
}

static inline void kernel_init_interrupts(void){
	/* setup ISRs */
	debug_printf (STR_PREFIX" installing GDT\n\r");
	gdt_install();
	debug_printf (STR_PREFIX" installing IDT\n\r");
	idt_install();
	debug_printf (STR_PREFIX" installing ISRs\n\r");
	isrs_install();

	debug_printf (STR_PREFIX" installing default ISR handlers\n\r");
	/* install default handler for ISRs */
	for(size_t i = 0; i < 32; i++) {
		isrs_install_handler(i, panic_handler);
	}
	return;
	isrs_install_handler(SYSCALL_VECTOR, panic_handler);

}

void ap_main(void){
	mem_set_ap_cr3();

	asm volatile(
        "movl $0x5000, %esp\n\r"
    );

	gdt_install();
	smp_boot_ack();
	while(1){
		asm volatile("hlt\n\r");
	}
}

void kernel_main(struct multiboot_tag* mbi, uint32_t mboot_magic, uint32_t foo) {

	UNUSED(foo);

	kernel_init_console();
	disable_printf();

	if(!mboot_init(mbi, mboot_magic)){
		goto fail;
	}
	kernel_init_interrupts();
	print_cpuid();
	acpi_init();
	pic_init();
	apic_init();
	
	if(!mem_init()){
	goto fail;
	}

	smp_init();

	pci_state_t* pci_state = pci_state_new();
	pci_state_enum(pci_state);

	fuzzer = new_fuzzer();

	if(fuzzer){

#ifdef DEFAULT_IO_FUZZING
		default_io_enum(fuzzer);
#endif

#ifdef IO_FUZZING
		isa_state_enum(fuzzer);		
#endif

#ifdef PCI_FUZZING	
		pci_register_areas(pci_state, fuzzer);
#endif

#ifdef APIC_FUZZING
		apic_register_areas(fuzzer);
#endif

#ifdef HPET_FUZZING
		register_area(fuzzer, 0xFed00000, 0x1000, 1, "HPET");
#endif

#ifdef EXTRA_AREAS
		EXTRA_AREAS
		for (uint32_t i = 0; i < sizeof(extra_areas)/sizeof(extra_areas_t); i++){
			register_area(fuzzer, extra_areas[i].base, extra_areas[i].length, extra_areas[i].type, extra_areas[i].name);
		}
#endif

#ifdef MEM_DEBUG
		print_mem_stats();
#endif


#ifndef DISABLE_FUZZER
		if(count_areas(fuzzer)){
			output_configured_areas(fuzzer);
			printf(STR_PREFIX"Fuzzer ready!\n\r");
			start_fuzzing(fuzzer);
		}
		else{
			printf(STR_PREFIX"Error: No fuzzing regions found...\n\r");
			goto fail;
		}
#else
		printf(STR_PREFIX"Fuzzer disabled! - We're done here...\n\r");
#endif

		printf(STR_PREFIX"Halting kernel...\n\r");

		/* reboot me */	
		//outw(0x2000, 0x604);
		while(1){
        	asm("hlt\n\r");
        }
	}
	else{
		goto fail;
	}

	fail:
	terminal_setcolor(FAIL_COLOR);
}
