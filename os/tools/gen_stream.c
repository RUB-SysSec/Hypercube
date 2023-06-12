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

#include <stdint.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#define BLOCK_SIZE 0x400
#define BITMAP_SIZE 32

//#include "../tesseract/opcodes.h"
#include "../tesseract/handler.h"
#include "../tesseract/state.h"
#include "../tesseract/decompiler.h"
#include "../tesseract/dict.h"

void exec_mmio_write_32(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_16(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_8(state_t* state_obj, hexa_op* input){}

void exec_io_write_32(state_t* state_obj, hexa_op* input){}
void exec_io_write_16(state_t* state_obj, hexa_op* input){}
void exec_io_write_8(state_t* state_obj, hexa_op* input){}

void exec_mmio_write_scratch_ptr(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_scratch_val(state_t* state_obj, hexa_op* input){}

void exec_mmio_read_32(state_t* state_obj, hexa_op* input){}
void exec_mmio_read_16(state_t* state_obj, hexa_op* input){}
void exec_mmio_read_8(state_t* state_obj, hexa_op* input){}

void exec_io_reads_32(state_t* state_obj, hexa_op* input){}
void exec_io_reads_16(state_t* state_obj, hexa_op* input){}
void exec_io_reads_8(state_t* state_obj, hexa_op* input){}

void exec_io_read_32(state_t* state_obj, hexa_op* input){}
void exec_io_read_16(state_t* state_obj, hexa_op* input){}
void exec_io_read_8(state_t* state_obj, hexa_op* input){}

void exec_mmio_read_scratch_ptr(state_t* state_obj, hexa_op* input){}
void exec_mmio_read_scratch_val(state_t* state_obj, hexa_op* input){}

void exec_mmio_memset_32(state_t* state_obj, hexa_op* input){}
void exec_mmio_memset_16(state_t* state_obj, hexa_op* input){}
void exec_mmio_memset_8(state_t* state_obj, hexa_op* input){}

void exec_io_memset_32(state_t* state_obj, hexa_op* input){}
void exec_io_memset_16(state_t* state_obj, hexa_op* input){}
void exec_io_memset_8(state_t* state_obj, hexa_op* input){}

void exec_io_write_scratch_ptr(state_t* state_obj, hexa_op* input){}

void exec_mmio_xor_32(state_t* state_obj, hexa_op* input){}
void exec_mmio_xor_16(state_t* state_obj, hexa_op* input){}
void exec_mmio_xor_8(state_t* state_obj, hexa_op* input){}

void exec_io_xor_32(state_t* state_obj, hexa_op* input){}
void exec_io_xor_16(state_t* state_obj, hexa_op* input){}
void exec_io_xor_8(state_t* state_obj, hexa_op* input){}

void exec_mmio_write_bruteforce_32(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_bruteforce_16(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_bruteforce_8(state_t* state_obj, hexa_op* input){}

void exec_io_write_bruteforce_32(state_t* state_obj, hexa_op* input){}
void exec_io_write_bruteforce_16(state_t* state_obj, hexa_op* input){}
void exec_io_write_bruteforce_8(state_t* state_obj, hexa_op* input){}

void exec_mmio_write_dict_32(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_dict_16(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_dict_8(state_t* state_obj, hexa_op* input){}

void exec_mmio_write_scratch_dict_ptr(state_t* state_obj, hexa_op* input){}

void exec_io_write_dict_32(state_t* state_obj, hexa_op* input){}
void exec_io_write_dict_16(state_t* state_obj, hexa_op* input){}
void exec_io_write_dict_8(state_t* state_obj, hexa_op* input){}

void exec_mmio_read_dict_32(state_t* state_obj, hexa_op* input){}
void exec_mmio_read_dict_16(state_t* state_obj, hexa_op* input){}
void exec_mmio_read_dict_8(state_t* state_obj, hexa_op* input){}

void exec_io_writes_32(state_t* state_obj, hexa_op* input){}
void exec_io_writes_16(state_t* state_obj, hexa_op* input){}
void exec_io_writes_8(state_t* state_obj, hexa_op* input){}

void exec_mmio_write_dict_data_32(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_dict_data_16(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_dict_data_8(state_t* state_obj, hexa_op* input){}

void exec_mmio_write_data_32(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_data_16(state_t* state_obj, hexa_op* input){}
void exec_mmio_write_data_8(state_t* state_obj, hexa_op* input){}


/* 
	File format:
		Array:
	 		- BLOCK_SIZE DATA
	 		- 0x20 BITMAP
*/

typedef uint32_t  u4;
typedef struct ranctx { u4 a; u4 b; u4 c; u4 d; } ranctx;

#define rot(x,k) (((x)<<(k))|((x)>>(BITMAP_SIZE-(k))))
u4 rng_val( ranctx *x ) {
    u4 e = x->a - rot(x->b, 27);
    x->a = x->b ^ rot(x->c, 17);
    x->b = x->c + x->d;
    x->c = x->d + e;
    x->d = e + x->a;
    return x->d;
}

void _srand(ranctx *rng_state, uint32_t seed){
	u4 i;
    rng_state->a = 0xf1ea5eed, rng_state->b = rng_state->c = rng_state->d = seed;
    for (i=0; i<20; ++i) {
        (void)rng_val(rng_state);
    }
}

void _srand_state(ranctx *rng_state, uint32_t a, uint32_t b, uint32_t c, uint32_t d){
	rng_state->a = a;
	rng_state->b = b;
	rng_state->c = c;
	rng_state->d = d;
}

int run_gen_block(char* filename, uint32_t size, ranctx* rand_state){

	uint8_t* buffer = malloc(BLOCK_SIZE);
	opcode_bitmap_t bitmap;
	ALLOC_OPCODE_BITMAP(bitmap);
	INIT_OPCODE_BITMAP(bitmap);

	/* max size => 15887 * (BLOCK_SIZE + BITMAP_SIZE) = 16776672 (15.9990 MB)  */
	assert(size <= 17301504);

	FILE* ptr = fopen(filename,"wb");


	for(uint32_t j = 0; j < size; j++){
		for(uint32_t i = 0; i < BLOCK_SIZE; i++){
			uint8_t byte = rng_val(rand_state);
			buffer[i] = byte;
		}
		fwrite(buffer, BLOCK_SIZE, 1, ptr);
		fwrite(bitmap, BITMAP_SIZE, 1, ptr);
	}

	fclose(ptr);
	free(buffer);
	free(bitmap);

	if (size == 1) {
		printf("[!] Generated %d block\n", size);
	}
	else {
		printf("[!] Generated %d blocks\n", size);
	}

	printf("[*] rng_state->a: %x\n", rand_state->a);
	printf("[*] rng_state->b: %x\n", rand_state->b);
	printf("[*] rng_state->c: %x\n", rand_state->c);
	printf("[*] rng_state->d: %x\n", rand_state->d);

	return 0;
}

void _count_opcodes_chunk(state_t* state_obj, void* data, uint32_t* op_code_counter, opcode_bitmap_t bitmap, bool decompile) {

	hexa_op* input = NULL;
	hexa_op* instructions = data;

	handler_t* handler;

	uint32_t counter = 0;
	uint32_t skipped = 0;

	uint32_t len = BLOCK_SIZE; 

	while(1){
		input = ((void*)instructions) + state_obj->ip;

		handler = &exec_handler[input->op_type%(exec_handler_size/sizeof(handler_t))];
		//printf("handler -> %p\n", handler->decompiler);

		if((state_obj->ip + handler->size) >= len){
			//printf("EXIT 1 (%x vs %x)\n\r", state_obj->ip, len);
			//abort();
			break;
		}

		if (CHECK_OPCODE_BITMAP(bitmap, counter)) {
			(*op_code_counter)++;
			if(decompile){
				printf("IP: 0x%x - ", state_obj->ip);
				((void (*)(state_t* state_obj, hexa_op* input))handler->decompiler)(state_obj, input);
			}
		}
		else{
			skipped++;
			//printf("SKIP => %d\n", counter);
			//abort();
		}

		state_obj->ip += handler->size;
		//printf("IP: %x\n", state_obj->ip);
		counter++;
	}

	//printf("skipped: %d\n", skipped);
}

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

int trim(char* filename, char* output, uint32_t start_offset, uint32_t end_offset, bool remove_empty_blocks) {
	FILE* in_ptr = fopen(filename, "rb");

	fseek(in_ptr, 0L, SEEK_END);
	uint64_t file_size = ftell(in_ptr);
	fseek(in_ptr, 0L, SEEK_SET);

	void* data = malloc(BLOCK_SIZE);

	state_t* state_obj = malloc(sizeof(state_t));
	memset(state_obj, 0, sizeof(state_t));
	
	opcode_bitmap_t bitmap;
	ALLOC_OPCODE_BITMAP(bitmap);

	uint32_t op_code_counter = 0;

	//printf("file_size: %llx\n", file_size);
	//printf("file_size: %llx\n", file_size % (BLOCK_SIZE + BITMAP_SIZE));
	assert((file_size % (BLOCK_SIZE + BITMAP_SIZE)) == 0);

	FILE* out_ptr = fopen(output, "wb");

	for(int i = 0; i < (file_size / (BLOCK_SIZE + BITMAP_SIZE)); i++){
		reset_state(state_obj);
		assert(fread(data, BLOCK_SIZE, 1, in_ptr) == 1);
		assert(fread(bitmap, BITMAP_SIZE, 1, in_ptr) == 1);


		int prev_c = op_code_counter;
		//printf("opcodes: %d\n", op_code_counter);

		_count_opcodes_chunk(state_obj, data, &op_code_counter, bitmap, false);
		//printf("opcodes: %d\n", op_code_counter);


		int v1 = MAX(prev_c, start_offset);
		int v2 = MIN(op_code_counter, end_offset);

		/*
		printf("v1: %d\n", v1);
		printf("v2: %d\n", v2);

		printf("prev_c: %d\n", prev_c);
		printf("start_offset: %d\n", start_offset);
		printf("end_offset: %d\n", end_offset);
		printf("opcodes: %d\n", op_code_counter);
		*/

		if (v1 < v2 && v1 >= prev_c && v1 < op_code_counter && v2 > prev_c && v2 <= op_code_counter) {

			//printf("removing stuff from block: %d\n", i);

			//printf("v1: %d / v2: %d\n", v1, v2);

			v1 -= prev_c;
			v2 -= prev_c;

			uint32_t bitmap_pos = 0;
			uint32_t bitmap_index = 0;
			while (bitmap_pos < v1) {
				if(CHECK_OPCODE_BITMAP(bitmap, bitmap_index)){
					bitmap_pos++;
				}
				bitmap_index++;
			}

			while (bitmap_pos < v2) {
				if(CHECK_OPCODE_BITMAP(bitmap, bitmap_index)){
					RESET_OPCODE_BITMAP(bitmap, bitmap_index);
					bitmap_pos++;
				}
				bitmap_index++;
			}

			/*
			for(int i = v1; i < v2; i++){

				RESET_OPCODE_BITMAP(bitmap, i);
				//printf("RESETTING %d\n", i);
			}
			*/
			printf("[!] removing opcodes from range %d-%d (block: %d)\n", v1, v2, i);

			/*
			printf("START -> %d\n", MAX(prev_c, start_offset));
			printf("END -> %d\n", MIN(op_code_counter, end_offset));
			*/
		}

		if(op_code_counter != prev_c || !remove_empty_blocks){
			fwrite(data, BLOCK_SIZE, 1, out_ptr);
			fwrite(bitmap, BITMAP_SIZE, 1, out_ptr);
		}
	}

	//printf("executed opcodes: %d\n", op_code_counter);

	//destroy_state(hexa_state);
	free(state_obj);
	free(data);
	free(bitmap);
	fclose(out_ptr);
	fclose(in_ptr);
	return 0;
}

void count_opcodes(char* filename) {

	FILE* ptr = fopen(filename,"rb");

	fseek(ptr, 0L, SEEK_END);
	uint64_t file_size = ftell(ptr);
	fseek(ptr, 0L, SEEK_SET);

	void* data = malloc(BLOCK_SIZE);
	
	opcode_bitmap_t bitmap;
	ALLOC_OPCODE_BITMAP(bitmap);

	//printf("file_size: %llx\n", file_size);
	//printf("file_size: %llx\n", file_size % (BLOCK_SIZE + BITMAP_SIZE));
	assert((file_size % (BLOCK_SIZE + BITMAP_SIZE)) == 0);

	uint32_t op_code_counter = 0;

	state_t* state_obj = malloc(sizeof(state_t));
	memset(state_obj, 0, sizeof(state_t));

	for(int i = 0; i < (file_size / (BLOCK_SIZE + BITMAP_SIZE)); i++){
		reset_state(state_obj);
		assert(fread(data, BLOCK_SIZE, 1, ptr) == 1);
		assert(fread(bitmap, BITMAP_SIZE, 1, ptr) == 1);
		_count_opcodes_chunk(state_obj, data, &op_code_counter, bitmap, false);
	}

	fclose(ptr);
	free(state_obj);
	free(bitmap);
	free(data);

	printf("[!] opcodes: %d\n", op_code_counter);

	return;
}

int run_decompiler(int argc, char** argv){

	state_t* hexa_state;
	hexa_state = new_state();
	hexa_state->mmio_area = malloc(sizeof(area_t*) * 256);
	hexa_state->io_area = malloc(sizeof(area_t*) * 256);

	uint32_t num_ranges = (argc-3)/3;

	printf("num_ranges: %d\n", num_ranges);

	for(int i = 0; i < (argc-3)/3; i++){
		uint8_t mmio = strtoul(argv[3+(3*i)], NULL, 10);
		uint32_t base_address = strtoul(argv[4+(3*i)], NULL, 16);
		uint32_t area_size = strtoul(argv[5+(3*i)], NULL, 16);

		if(!mmio){
			hexa_state->io_area[hexa_state->num_io_areas] = malloc(sizeof(area_t));
			hexa_state->io_area[hexa_state->num_io_areas]->base = base_address;
			hexa_state->io_area[hexa_state->num_io_areas]->size = area_size;
			strcpy(hexa_state->io_area[hexa_state->num_io_areas]->desc, "-");
			hexa_state->io_area[hexa_state->num_io_areas]->virtual_base = 0;
			hexa_state->num_io_areas++;		
		}
		
		else{
			hexa_state->mmio_area[hexa_state->num_mmio_areas] = malloc(sizeof(area_t));
			hexa_state->mmio_area[hexa_state->num_mmio_areas]->base = base_address;
			hexa_state->mmio_area[hexa_state->num_mmio_areas]->size = area_size;
			strcpy(hexa_state->mmio_area[hexa_state->num_mmio_areas]->desc, "-");
			hexa_state->mmio_area[hexa_state->num_mmio_areas]->virtual_base = base_address;
			hexa_state->num_mmio_areas++;
		}
	}
	
	print_state(hexa_state);

	FILE* ptr = fopen(argv[2], "rb");

	fseek(ptr, 0L, SEEK_END);
	uint64_t file_size = ftell(ptr);
	fseek(ptr, 0L, SEEK_SET);

	void* data = malloc(BLOCK_SIZE);
	
	opcode_bitmap_t bitmap;
	ALLOC_OPCODE_BITMAP(bitmap);

	uint32_t op_code_counter = 0;

	//printf("file_size: %llx\n", file_size);
	//printf("file_size: %llx\n", file_size % (BLOCK_SIZE + BITMAP_SIZE));
	assert((file_size % (BLOCK_SIZE + BITMAP_SIZE)) == 0);

	for(int i = 0; i < (file_size / (BLOCK_SIZE + BITMAP_SIZE)); i++){
		reset_state(hexa_state);
		assert(fread(data, BLOCK_SIZE, 1, ptr) == 1);
		assert(fread(bitmap, BITMAP_SIZE, 1, ptr) == 1);
		_count_opcodes_chunk(hexa_state, data, &op_code_counter, bitmap, true);
	}

	printf("executed opcodes: %d\n", op_code_counter);

	destroy_state(hexa_state);
	free(data);
	free(bitmap);
	fclose(ptr);
	return 0;
}

void show_usage_pre(char* argv_0){
	printf("Usage: %s", argv_0);
}

void show_usage_gen_blocks(void){
	printf("\tgen_blocks\t<filename> <seed> <buffers> <randstate-a> <randstate-b> <randstate-c> <randstate-d>\n");
}

void show_usage_trim(void){
	printf("\ttrim\t\t<input> <output> <start> <end> <remove_empty_blocks>\n");
}

void show_usage_count(void){
	printf("\tcount\t\t<input>\n");
}

void show_usage_decompile(void){
	printf("\tdecompile\t<input> <mmio base size>\n");
}

void show_usage_seed2state(void){
	printf("\tseed2state\t<seed>\n");
}

void show_usage(char* argv_0){
	printf("Usage: %s <Mode> <...>\n", argv_0);
		printf("Modes:\n");
		show_usage_gen_blocks();
		show_usage_trim();
		show_usage_count();
		show_usage_decompile();
		show_usage_seed2state();
}

int main(int argc, char** argv){

	if (argc < 2){
		show_usage(argv[0]);
		return 0;
	}

	if (strcmp(argv[1], "gen_blocks") == 0){
		if (argc < 5){
			show_usage_pre(argv[0]);
			show_usage_gen_blocks();
			return 0;
		}
		
		uint32_t rng_state_a = strtoul(argv[4], NULL, 16);
		uint32_t rng_state_b = strtoul(argv[5], NULL, 16);
		uint32_t rng_state_c = strtoul(argv[6], NULL, 16);
		uint32_t rng_state_d = strtoul(argv[7], NULL, 16);

		ranctx rand_state;
		_srand_state(&rand_state, rng_state_a, rng_state_b, rng_state_c, rng_state_d);

		return run_gen_block(argv[2], atoi(argv[3]), &rand_state);
	}

	else if (strcmp(argv[1], "trim") == 0){
		if (argc < 6){
			show_usage_pre(argv[0]);
			show_usage_trim();
			return 0;
		}

		uint32_t start_offset = strtoul(argv[4], NULL, 10);
		uint32_t end_offset = strtoul(argv[5], NULL, 10);
		bool remove_empty_blocks = strtoul(argv[6], NULL, 10);

		return trim(argv[2], argv[3], start_offset, end_offset, remove_empty_blocks);
	}

	else if (strcmp(argv[1], "count") == 0){
		if (argc < 3){
			show_usage_pre(argv[0]);
			show_usage_count();
			return 0;
		}

		count_opcodes(argv[2]);
		return 0;
	}

	else if (strcmp(argv[1], "decompile") == 0) {
		if (argc < 5 || (argc-3)%3 != 0){
			show_usage_pre(argv[0]);
			show_usage_decompile();
			return 0;
		}
		return run_decompiler(argc, argv);
	}

	else if (strcmp(argv[1], "seed2state") == 0) {
		if (argc < 3){
			show_usage_pre(argv[0]);
			show_usage_seed2state();
			return 0;
		}

		uint32_t seed = strtoul(argv[2], NULL, 16);

		ranctx rand_state;
		_srand(&rand_state, seed);

		printf("[*] rng_state->a: %x\n", rand_state.a);
		printf("[*] rng_state->b: %x\n", rand_state.b);
		printf("[*] rng_state->c: %x\n", rand_state.c);
		printf("[*] rng_state->d: %x\n", rand_state.d);

		return 0;
	}

	else{
		show_usage(argv[0]);
	}
}	