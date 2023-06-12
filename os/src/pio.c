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


#include "smp.h"
#include "apic.h"
#include "tty.h"
#include "mmio.h"
#include "libk.h"
#include "mem.h"
#include "io.h"

#include "fuzz.h"


#define STR_PREFIX	" [ISA]  "
#define STR_PIO_PREFIX	" [PIO]  "


#define PNP_ADDRESS		0x279
#define PNP_WRITE_DATA	0xa79

#define PNP_MAX_CARDS 8

#define PNP_SET_RD_DATA		0x00
#define PNP_SERIAL_ISOLATION	0x01
#define	PNP_CONFIG_CONTROL	0x02
#define PNP_CONFIG_CONTROL_RESET_CSN	0x04
#define PNP_CONFIG_CONTROL_WAIT_FOR_KEY	0x02
#define PNP_CONFIG_CONTROL_RESET	0x01
#define PNP_WAKE		0x03

typedef struct _pnp_id {
	uint32_t vendor_id;
	uint32_t serial;
	uint8_t checksum;
} pnp_id;

static void isa_pnp_init(){

	/* freebsd code */

	int cur, i;

	outb(0, PNP_ADDRESS);
	outb(0, PNP_ADDRESS);

	cur = 0x6a;
	outb(cur, PNP_ADDRESS);

	for (i = 1; i < 32; i++) {
		cur = (cur >> 1) | (((cur ^ (cur >> 1)) << 7) & 0xff);
		outb(cur, PNP_ADDRESS);
	}
}

static void pnp_write(uint8_t d, uint8_t r){
	outb(d, PNP_ADDRESS);
	outb(r, PNP_WRITE_DATA);
}

static uint8_t pnp_get_serial(pnp_id *p, uint16_t read_port){
	int i, bit, valid = 0, sum = 0x6a;
	uint8_t *data = (uint8_t *)p;

	memset(data, 0, sizeof(char) * 9);
	outb(PNP_SERIAL_ISOLATION, PNP_ADDRESS);
	for (i = 0; i < 72; i++) {
		bit = inb(read_port) == 0x55;
		//if (bit)
		//	debug_printf("%x\n", bit);
		microdelay(2500000);	/* Delay 250 usec */

		/* Can't Short Circuit the next evaluation, so 'and' is last */
		bit = (inb(read_port) == 0xaa) && bit;
		microdelay(2500000);	/* Delay 250 usec */

		valid = valid || bit;
		if (i < 64)
			sum = (sum >> 1) |
			  (((sum ^ (sum >> 1) ^ bit) << 7) & 0xff);
		data[i / 8] = (data[i / 8] >> 1) | (bit ? 0x80 : 0);
	}

	valid = valid && (data[8] == sum);

	return (valid);
}


uint32_t isa_pnp_isolation(uint16_t read_port){
	pnp_id id;
	isa_pnp_init();

	pnp_write(PNP_CONFIG_CONTROL, PNP_CONFIG_CONTROL_RESET_CSN);
	pnp_write(PNP_WAKE, 0);
	pnp_write(PNP_SET_RD_DATA, read_port);

	for (uint8_t csn = 1; csn < PNP_MAX_CARDS; csn++) {
		outb(PNP_SERIAL_ISOLATION, PNP_ADDRESS);
		microdelay(1000000);

		/* more magic */
		if(pnp_get_serial(&id, read_port)){
			debug_printf(STR_PREFIX"FOUND SOMETHING\n");
		}

		pnp_write(PNP_WAKE, 0);
	}

	return 0;
}


typedef struct{
	uint16_t port;
	uint16_t size;
	char* name;
} known_port_t;

/* well known ports (no need to rediscover those) */
known_port_t well_known_ports[] = {
	{0x0000, 0xf, "DMA"},
	{0x0020, 2, "PIC"},
	{0x0040, 4, "PIT"},

	{0x0060, 1, "i8042"},
	{0x0064, 1, "i8042"},

	{0x0061, 1, "pcspk"},

	{0x0070, 2, "RTC"},

	{0x1f0, 8, "IDE"},
	{0x170, 8, "IDE"},
	{0x376, 1, "IDE"},
	{0x3f6, 1, "IDE"},

	{0xc000, 0x10, "IDE"},

	{0x3F0, 0x8, "Floppy"},

	{0x378, 4, "Parallel"},
	{0x3BC, 4, "Parallel"},
	{0x278, 4, "Parallel"},

	{0x3F8, 8, "SERIAL"},
	{0x2F8, 8, "SERIAL"},
	{0x3E8, 8, "SERIAL"},
	{0x2E8, 8, "SERIAL"},

	{0x534, 4, "cs4231a"}, 

	{0x224, 4, "sb16"},
	{0x22a, 1, "sb16"},
	{0x22c, 4, "sb16"},
};

/* ports to ignore */
known_port_t ignored_ports[] = {


#ifdef TARGET_BHYVE
	{0x00b2, 1, "bhyve"},
	{0x03cd, 1, "bhyve"},
	{0x0404, 1, "bhyve"},
	{0x0003c2, 1, "bhvye: vga_port_in_handler"},
	{0x408, 1, "bhvye: vm_run error -1, errno 5"},
	{0x400, 4, "bhvye: inb 0x03c6 (VGA)"},
	{0x0511, 1, "bhyve"},
	{0x03c6, 3, "bhvye ???"},
	{0x3cb, 2, "bhvye ???"},
	{0x3d0, 4, "bhvye ???"},
	{0x3d6, 4, "bhvye ???"},
	{0x3db, 5, "bhvye ???"},
#endif

	{0x8900, 1, "BOCHS SHUTDOWN"},
	{0x604, 1, "QEMU SHUTDOWN"},

	{0x01f0, 8, "???"},
	{0x0376, 1, "???"},
	{0x03f6, 1, "???"},

#ifdef TARGET_VMWARE
	{0x64, 1, "VMWARE ???"},
	{0x177, 1, "VMWARE ???"},
	{0x1f7, 1, "VMWARE ???"},

	{0x376, 1, "VMWARE ???"},
	{0x3f6, 1, "VMWARE ???"},
	{0x3f8, 1, "VMWARE ???"},
	{0x3f9, 1, "VMWARE ???"},

	{0x3fa, 1, "VMWARE ???"},
	{0x1005, 1, "VMWARE ???"},
	{0x102c, 1, "VMWARE ???"},
	{0x102d, 1, "VMWARE ???"},
	{0x102e, 1, "VMWARE ???"},
	{0x455, 1, "VMWARE SHUTDOWN"},
#endif

};



void isa_state_enum(fuzzer_state_t* fuzzer){
	uint32_t num_devs = 0; 
	
	debug_printf(STR_PREFIX"Searching for IO ports...\n\r");

	uint32_t last_io = 0;
	bool last_pending = false;
	uint32_t prev_io = 0xfff;
	for(uint32_t i = 0xa0; i < 0x5000; i++){

		/* check if i is either in ignored_ports or well_known_ports. skip if this is the case */
		bool skip = false;
		for(uint32_t j = 0; j < sizeof(well_known_ports)/sizeof(known_port_t); j++){
			if(i >= well_known_ports[j].port && i < well_known_ports[j].port + well_known_ports[j].size){
				debug_printf(STR_PIO_PREFIX"found well known port %s at %x\n\r", well_known_ports[j].name, i);
				skip = true;
				break;
			}
		}

		if (skip) continue;

		for(uint32_t j = 0; j < sizeof(ignored_ports)/sizeof(known_port_t); j++){
			if(i >= ignored_ports[j].port && i < ignored_ports[j].port + ignored_ports[j].size){
				debug_printf(STR_PIO_PREFIX"found well known port %s at %x\n\r", well_known_ports[j].name, i);
				skip = true;
				break;
			}
		}

		if (skip) continue;
		
		uint8_t value = inb(i);
		if(value != 0xff){
		
			debug_printf("PORT FOUND %x\n", i);
			if(last_pending && prev_io+1 != i){
				debug_printf(STR_PIO_PREFIX"found ports at %x - %x\n\r", last_io, prev_io);

				register_area(fuzzer, last_io, (prev_io-last_io)+1, 0, "PIO");
				last_pending = false;
			}
			
			if(!last_pending){
				last_io = i;
				last_pending = true;
			}
			

			prev_io = i;
			
		}
		else{
			//debug_printf("TRY %x\n", i);
			
			for(uint8_t j = 0; j < 0xf; j++){
				debug_printf("TRY - %x %x\n", i, j);

				outb(j, i);
				if(value != inb(i)){
					debug_printf("=>>>>> PORT FOUND %x\n", i);
					outb(i, value);
					break;
				}
				outb(value, i);
			}
			if(value && value != 0xff){
								debug_printf("=>>>>> PORT FOUND %x %x\n", i, value);

			}
			outb(value+1, i);
			if(value != inb(i)){
				debug_printf("=>>>>> PORT FOUND %x\n", i);
			}
			outb(value, i);
			
		}
	}
	return;


	debug_printf(STR_PREFIX"Searching for pnp devices...\n\r");

	for(uint16_t read_port = 0x203; read_port <= 0x3ff; read_port++){
		//debug_printf(STR_PREFIX"Trying read port %x\n", read_port);
		num_devs = isa_pnp_isolation(read_port);
		if(num_devs){
			break;
		}
	}
	debug_printf(STR_PREFIX"%d pnp devices found!\n", num_devs);
}

void default_io_enum(fuzzer_state_t* fuzzer){
	for (uint32_t i = 0; i < sizeof(well_known_ports)/sizeof(known_port_t); i++){
		known_port_t* port = &well_known_ports[i];
		register_area(fuzzer, port->port, port->size, 0, port->name);
	}
}
