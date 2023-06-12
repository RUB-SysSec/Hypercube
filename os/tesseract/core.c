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
#ifdef HYPERCUBE
#include "tty.h"
#else
#include <stdio.h>
#include <stdlib.h>
#endif
#include "state.h"
#include "opcodes.h"
#include "decompiler.h"
#include "core.h"
#include "handler.h"

#ifdef PAYLOAD
void run(hexa_op* instructions, uint32_t len, state_t* state_obj, opcode_bitmap_t opcode_bitmap){
#else
void run(hexa_op* instructions, uint32_t len, state_t* state_obj){
#endif
	hexa_op* input = NULL;
	handler_t* handler;

#ifdef PAYLOAD
	uint32_t counter = 0;
#endif

	reset_state(state_obj);
	
	while(1){
		input = ((void*)instructions) + state_obj->ip;

		handler = &exec_handler[input->op_type%(exec_handler_size/sizeof(handler_t))];

		if((state_obj->ip + handler->size) >= len){
			//printf("EXIT 1 (%x vs %x)\n\r", state_obj->ip, len);
			return;
		}

#if defined(DISASSEMBLER) || defined(DECOMPILER) || defined(HYBRID)
		if (state_obj->loop_count == 0xFF){
#ifdef DECOMPILER
			//printf("\t");
			((void (*)(state_t* state_obj, hexa_op* input))handler->decompiler)(state_obj, input);
#endif
#ifdef DISASSEMBLER
			printf("\t0x%08x\t\t%s\n\r", state_obj->ip, (const char*)handler->name);//, input->arg);
#endif
#ifdef HYBRID
			printf("\t0x%08x\t\t%s\t0x%08x\t\t//", state_obj->ip, (const char*)handler->name, input->arg);
			((void (*)(state_t* state_obj, hexa_op* input))handler->decompiler)(state_obj, input);
#endif
		}
#ifdef LOOP
		else{
#ifdef DECOMPILER
			//printf("0x%02x\t0x%08x\t\t%s\t0x%08x\t\t", state_obj->loop_count, state_obj->ip, (const char*)handler[0], input->arg);
			//printf("0x%02x\t", state_obj->loop_count);
			((void (*)(state_t* state_obj, hexa_op* input))handler->decompiler)(state_obj, input);
#endif
#ifdef DISASSEMBLER
			printf("0x%02x\t0x%08x\t\t%s\t0x%08x\n", state_obj->loop_count, state_obj->ip, (const char*)handler->name, input->arg);
#endif
#ifdef HYBRID
			printf("0x%02x\t0x%08x\t\t%s\t0x%08x\t\t//", state_obj->loop_count, state_obj->ip, (const char*)handler->name, input->arg);
			((void (*)(state_t* state_obj, hexa_op* input))handler->decompiler)(state_obj, input);
#endif
		}	
#endif	
#endif

#ifdef PAYLOAD
	if (CHECK_OPCODE_BITMAP(opcode_bitmap, counter)) {
#endif
		//base_exec_handler(state_obj, input);
		((void (*)(state_t* state_obj, hexa_op* input))handler->exec)(state_obj, input); 
#ifdef PAYLOAD
	}
	else {
		//printf("skipping: %x (%d)\n", state_obj->ip, counter);
	}
#endif

		state_obj->ip += handler->size;
		//printf("IP: %x\n", state_obj->ip);

		if((state_obj->ip + sizeof(hexa_op)) >= len){
			//printf("EXIT 2 (%x vs %x)\n\r", state_obj->ip, len);
			return;
		}

		//input += (uint)handler[3];
#ifdef PAYLOAD
		counter++;
#endif
	}
}
