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

#include "test.h"
#include "io.h"
#include "msr.h"
#include "serial.h"
#include "tty.h"
#include "pic.h"
#include "mmio.h"
#include "libk.h"

#define STR_PREFIX  " [FUZZ] "

static unsigned long int next = 1;
 
static uint32_t rand(void){
    next = next * 1103515245 + 12345;
    return (uint32_t)(next / 65536) % 32768;
}

static uint64_t rdtsc(){
   uint32_t hi, lo;
   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
   return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}
 
static void srand(void){
    uint32_t seed;
    /*
    uint32_t error = 0;
    while(!error){
        asm volatile ("rdrand %0; setc %1" : "=r" (seed), "=m" (error));
    }
    */

    seed = (uint32_t)rdtsc();
    next = seed;
}

static void div_by_zero_test(void){
	for (int i = 0; i < 5; i++){
		terminal_printf("%d\n", 5/i);
	}
}

static void page_fault_test(void){
	volatile uint8_t* bad;
	bad = (uint8_t*)0xbad0000;
	bad[0] = 0x0;
}


void pci_mmio_accesses_test2(void){
    volatile uint8_t* test = (uint8_t*)0x810000;
    test[0] = 'H';
    test[1] = 'Y';
    test[2] = 'P';
    test[3] = 'E';
    test[4] = 'R';

}

static void pci_mmio_accesses_test(void){
    //bool low = true;

	//volatile uint8_t* test;
    //volatile uint32_t tmp;
    //uint8_t value;
    //uint16_t port;
    srand();
	printf(STR_PREFIX" PCI MMIO...\n");
    //asm volatile("cli\n");
	//test = (uint8_t*)0x810000;
	//for(uint32_t j = 0; j != 0x4000; j++){
    for(uint64_t j = 0; j < 0xffffff ; j++){
       // printf("%x\n", j);
        for(uint32_t i = 0; i != 0xff; i++){
            //printf("%x\n", j);

            /* IRQ polling */
            
            /*
            bool intr = pci_interrupt_pending(11);
            if(intr && low){
                printf("INTERRUPT!\n");
                low = false;
            }

            if(intr == false && low == false){
                printf("NO INTERRUPT!\n");
                low = true;
            }
            */
            //value = (uint8_t)rand();
            //port = rand()%0x00003000;
            //printf(" %2x -> %1x\n", port, value);
            //test[port] = value;
            //tmp = (uint32_t*)(0x810000 + (rand()%0x2000));

            //outb(rand(), 0x0000c080 + (rand()%0x20));
            //inb(0x0000c080 + (rand()%0x20));

            /*
            uint32_t a = (rand()%0x119);
            uint32_t b = rand();
            //printf("%x\t%x\n", a,b);
            mmio_write8(0x810000, a, b);
            */

            hexdump("apic_base: ", (void *)0x820000, 128);

            for (uint32_t d= 0; d < 100; d++){
                for (uint32_t c = 0; c < 0x120; c++){
                    mmio_write8(0x810000, c, rand());
                }
            }            
            
            hexdump("apic_base: ", (void *)0x820000, 128);

            return;
            mmio_write8(0x810000, 0x0004d5c, 0x18);

            mmio_write8(0x810000, 0x0004d5c, 0x18);
            mmio_write8(0x810000, 0x0004293, 0x08);

            mmio_write8(0x810000, 0x0004293, 0x4);
            mmio_write8(0x810000, 0x0004293, 0x10);
            mmio_write8(0x810000, 0x0004293, 0x10);
            //return;
  
            //mmio_read8(0x810000, (rand()%0x1000));
            //mmio_write8(0x810000, (rand()%0x1000), rand());
           
            //tmp = (uint32_t*)*tmp;
            //outl(rand(), (rand()%0x00000020) + 0x0000c180);
        }		
	}
}

static void pci_io_accesses_test(void){
	printf(STR_PREFIX" PCI IO...\n");
	for(uint32_t j = 0; j != 0x40; j++){
		uint32_t addr = 0xc000+j;
		outb(j, addr);
		outw(j, addr);
		outl(j, addr);
		inb(addr);
		inw(addr);
		inl(addr);
	}
	printf(" -> Done\n");
}

static void msr_access_test(void){
	/* MSR Bruteforcing + GPF Handling */
    /*
    uint32_t lo, hi;
    uint64_t result;
    for(uint32_t i = 0; i != 0x1000000; i++){
            result = rdmsr64(i);
            wrmsr64(i, result ^ 0xFFFFFFFFFFFFFFFF);
            wrmsr64(i, result ^ 0xAAAAAAAAAAAAAAAA);
            wrmsr64(i, result ^ 0x5555555555555555);

            lo = result;
            hi = result>>32;
            //printf("%x%x [%x]\n", hi, lo, i);
            
            if (result){
                    lo = result;
                    hi = result>>32;
                    printf("rdmsr64 %x: %x%x\n", i, hi, lo);
            }
                }
    */
}

static void smep_test(void){
	/* Code Exec on SMEP Page */

	/* enable SMEP */
	asm volatile ("mov %%cr4, %%eax\n"
			"orl $0x100000, %%eax\n"
			"mov %%eax, %%cr4\n"::: "%eax");

	void(*test_func)(void) = (void*)0x810000;
	test_func();
}

static void trigger_cve_2011_1751_test(void){
	/* Insufficient checks in PCI hotplug CVE-2011-1751 */
	outl(0x2, 0xae08);
}

static void io_access_test(void){
	for(uint32_t port = 0; port < 0xffff; port++) {
         serial_printf(SERIAL_PORT_A, "IO %x\n", port);
         for(uint16_t value = 0; value < 0x0500; value++) {
            if (port == 0x62 || port == 0x63){
                continue;
            } 

            /* Virtual Box */
            if (port == 0x00003b7){
            	continue;
            }

            /* VMware Fusion */
            if (port == 0x0000064 || 
            	port == 0x0000177 || 
            	port == 0x00001f7 || 
            	port == 0x0000376 || 
            	port == 0x00003f6 || 
            	port == 0x00003f8 || 
            	port == 0x00003f9 || 
            	port == 0x00003fa || 
            	port == 0x0001005 || 
            	port == 0x000102c || 
            	port == 0x000102d || 
            	port == 0x000102e){
            	continue;
            }
            //if (port == 0x64 && value >= 0xF0){ // Reset
            //    continue;
            //}
            //if (port == 0x7b && value >= 0x00){
            //    continue;
            //}
	/*
            if (port == 0x7f && value > 0x00){
                continue;
            }
            if (port == 0x91){ 
                continue;
            }
            if (port == 0x92){ //A20 + Fast Reset
                continue;
            }
            if (port == 0x3fb){
                continue;
            }
            if (port == 0x3fc){
                continue;
            }
            if (port == 0xcf9){
                continue;
            }
		*/
            outl(value, port);
         }
     }
}


void test(enum test_type id){
	switch (id){
		case TEST_DIV_BY_ZERO: 		div_by_zero_test(); break;
		case TEST_PAGE_FAULT: 		page_fault_test(); break;
		case TEST_PCI_MMIO: 		pci_mmio_accesses_test(); break;
		case TEST_PCI_IO:			pci_io_accesses_test(); break;
		case TEST_MSR:				msr_access_test(); break;
		case TEST_SMEP:				smep_test(); break;
		case TEST_CVE_2015_3456:	break;
		case TEST_CVE_2011_1751:	trigger_cve_2011_1751_test(); break;
		case TEST_IO:				io_access_test(); break;
	}
}
