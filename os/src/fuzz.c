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

#include "fuzz.h"
#include "tty.h"
#include "state.h"
#include "core.h"
#include "libk.h"
#include "mem.h"
#include "serial.h"
#include "../../config.h"

#define BLOCK_SIZE 0x400
#define BITMAP_SIZE 32

//#define DEBUG

#define STR_PREFIX	" [FUZZ] "

#ifdef PAYLOAD
extern uint8_t _binary_misc_crash_hexa_start;
extern uint8_t _binary_misc_crash_hexa_end;

#endif


static uint64_t rdtsc(){

   uint32_t hi, lo;
   __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
   return ( (uint64_t)lo)|( ((uint64_t)hi)<<32 );
}
 

typedef uint32_t  u4;
typedef struct ranctx { u4 a; u4 b; u4 c; u4 d; } ranctx;

#define rot(x,k) (((x)<<(k))|((x)>>(32-(k))))
u4 ranval( ranctx *x ) {
    u4 e = x->a - rot(x->b, 27);
    x->a = x->b ^ rot(x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}

void raninit( ranctx *x, int init) {
	if (init){
		microdelay(rdtsc());
	}
	uint32_t seed = rdtsc() & 0xFF;
	seed = seed | ((rdtsc() & 0xFF) << 8);
	seed = seed | ((rdtsc() & 0xFF) << 16);
	seed = seed | ((rdtsc() & 0xFF) << 24);

#ifdef SEED
	seed = SEED;
#endif

	if(init){
		printf("Seed: %x\n", seed);
	}

    u4 i;
    x->a = 0xf1ea5eed, x->b = x->c = x->d = seed;
    for (i=0; i<20; ++i) {
        (void)ranval(x);
    }
}

fuzzer_state_t* new_fuzzer(void){
	fuzzer_state_t* self = NULL;

#ifdef DEBUG
	if(1){
#else
	//if (supports_vmx()){
#endif
	self = kmalloc(sizeof(fuzzer_state_t));
	self->hexa_state = new_state();
	//self->payload_buffer = NULL;

	self->hexa_state->mmio_area = kmalloc(sizeof(area_t*) * 256);
	self->hexa_state->io_area = kmalloc(sizeof(area_t*) * 256);

	debug_printf(STR_PREFIX"Allocating 64kB memory for fuzzer payload buffer\n\r");
	self->payload_buffer = (uintptr_t)kvmalloc( (64 * 1024)); // Alloc 64kB
	debug_printf(STR_PREFIX"payload_buffer = 0x%x-0x%x\n\r", self->payload_buffer, self->payload_buffer+(64*1024));
	memset((void*)self->payload_buffer, 0xff, 64*1024);
	debug_printf(STR_PREFIX"Payload buffer prepared!\n\r");

	//printf(STR_PREFIX"kAFL_hypercall -> \n\r");
//#ifndef DEBUG
//		kAFL_hypercall(1337, 0x1000);
//#endif
	debug_printf(STR_PREFIX"Done!\n\r");

	return self;
}

void destroy_fuzzer(fuzzer_state_t* self){
	UNUSED(self);
	/* ... */
	return;
}

int count_areas(fuzzer_state_t* self){
	return self->hexa_state->num_io_areas + self->hexa_state->num_mmio_areas;
}

int i = 0;
void register_area(fuzzer_state_t* self, uintptr_t base_address, uint32_t size, bool mmio, char* description){

	if (mmio){
		printf(STR_PREFIX"available area - base:0x%x size:0x%x MMIO -> \"%s\"\n\r", base_address, size, description);
	}
	else{
		printf(STR_PREFIX"available area - base:0x%x size:0x%x PIO  -> \"%s\"\n\r", base_address, size, description);
	}

#ifdef TARGET_FILTERS
	TARGET_FILTERS

	bool match = false;
	for (uint32_t i = 0; i < sizeof(target_filters)/sizeof(char*); i++){
		if (strcmp(target_filters[i], description) == 0){
			match = true;
			break;
		}
	}
	if (!match){
		return;
	}
#endif
	



	if(!mmio){
		//	printf(STR_PREFIX"%s - base:0x%x size:0x%x mmio:0x%d -> %s\n\r", __func__, base_address, size, mmio, description);
		self->hexa_state->io_area[self->hexa_state->num_io_areas] = kvmalloc(sizeof(area_t));
		self->hexa_state->io_area[self->hexa_state->num_io_areas]->base = base_address;
		self->hexa_state->io_area[self->hexa_state->num_io_areas]->size = size;
		strncpy(self->hexa_state->io_area[self->hexa_state->num_io_areas]->desc, description, strlen(description));
		self->hexa_state->io_area[self->hexa_state->num_io_areas]->virtual_base = 0;
		self->hexa_state->num_io_areas++;		
	}
	
	else{
		//printf(STR_PREFIX"%s - base:0x%x size:0x%x mmio:0x%d -> %s\n\r", __func__, base_address, size, mmio, description);
		self->hexa_state->mmio_area[self->hexa_state->num_mmio_areas] = kvmalloc(sizeof(area_t));
		self->hexa_state->mmio_area[self->hexa_state->num_mmio_areas]->base = base_address;
		self->hexa_state->mmio_area[self->hexa_state->num_mmio_areas]->size = size;
		strncpy(self->hexa_state->mmio_area[self->hexa_state->num_mmio_areas]->desc, description, strlen(description));
		self->hexa_state->mmio_area[self->hexa_state->num_mmio_areas]->virtual_base = (uintptr_t)virtual_map(base_address, size);
		self->hexa_state->num_mmio_areas++;
	}
	
}

uint8_t* null_page = (uint8_t*)0x0;

void output_configured_areas(volatile fuzzer_state_t* self){
	print_state(self->hexa_state);
}

void start_fuzzing(volatile fuzzer_state_t* self){

#ifndef PAYLOAD
	enable_printf();
#endif

	null_page = (uint8_t*) 0x000;
	self->hexa_state->alloc_areas[0] = null_page;

#ifdef PAYLOAD

	enable_printf();
	
	uint32_t payload_size = (uint32_t)&_binary_misc_crash_hexa_end - (uint32_t)&_binary_misc_crash_hexa_start;

	uintptr_t payload_data = PAYLOAD_BUFFER_VIRTUAL_OFFSET + &_binary_misc_crash_hexa_start;

	assert((payload_size%(BLOCK_SIZE+BITMAP_SIZE)) == 0);
	opcode_bitmap_t bitmap = kvmalloc(BITMAP_SIZE);

	for(uint32_t i = 0; i < payload_size/(BLOCK_SIZE+BITMAP_SIZE); i++) {

		/* fill up an entire page */
		for(uint8_t j = 0; j < 4; j++) {
			memcpy(self->hexa_state->alloc_areas[0]+(j*BLOCK_SIZE), payload_data+(i*(BLOCK_SIZE+BITMAP_SIZE)), BLOCK_SIZE);
		}

		memcpy(bitmap, payload_data+(i*(BLOCK_SIZE+BITMAP_SIZE))+BLOCK_SIZE, BITMAP_SIZE);

		run((void*)self->hexa_state->alloc_areas[0], BLOCK_SIZE, self->hexa_state, bitmap);
	}

	printf("reproducer finished!\n\r");

	return;
#else

	enable_printf();
	ranctx rand_state;
	raninit(&rand_state, 1);

	while(1){

		for(uint32_t i = (uint32_t)self->hexa_state->alloc_areas[0]; i < BLOCK_SIZE; i++){
			self->hexa_state->alloc_areas[0][i] = ranval(&rand_state);
		}

		/* fill up an entire page */
		for(uint8_t j = 1; j < 4; j++) {
			memcpy(self->hexa_state->alloc_areas[0]+j*(BLOCK_SIZE), self->hexa_state->alloc_areas[0], BLOCK_SIZE);
		}

		run((void*)self->hexa_state->alloc_areas[0], BLOCK_SIZE, self->hexa_state);
	}
#endif
}
