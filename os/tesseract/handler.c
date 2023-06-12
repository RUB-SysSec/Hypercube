#include <stdint.h>
#include <stdlib.h>
#include "handler.h"
#include "opcodes.h"
#include "decompiler.h"

handler_t exec_handler[] = {
	{"mmio_write_32", exec_mmio_write_32, decompiler_mmio_write_32, sizeof(op_mmio_write_32)},
	{"mmio_write_16", exec_mmio_write_16, decompiler_mmio_write_16, sizeof(op_mmio_write_16)},
	{"mmio_write_8", exec_mmio_write_8, decompiler_mmio_write_8, sizeof(op_mmio_write_8)},

	{"io_write_32", exec_io_write_32, decompiler_io_write_32, sizeof(op_io_write_32)},
	{"io_write_16", exec_io_write_16, decompiler_io_write_16, sizeof(op_io_write_16)},
	{"io_write_8", exec_io_write_8, decompiler_io_write_8, sizeof(op_io_write_8)},
	
	{"mmio_write_scratch_ptr", exec_mmio_write_scratch_ptr, decompiler_mmio_write_scratch_ptr, sizeof(op_mmio_write_scratch_ptr)},
	{"mmio_write_scratch_ptr", exec_mmio_write_scratch_ptr, decompiler_mmio_write_scratch_ptr, sizeof(op_mmio_write_scratch_ptr)},
	{"mmio_write_scratch_ptr", exec_mmio_write_scratch_ptr, decompiler_mmio_write_scratch_ptr, sizeof(op_mmio_write_scratch_ptr)},

	{"io_write_scratch_ptr", exec_io_write_scratch_ptr, decompiler_io_write_scratch_ptr, sizeof(op_io_write_scratch_ptr)},
	{"io_write_scratch_ptr", exec_io_write_scratch_ptr, decompiler_io_write_scratch_ptr, sizeof(op_io_write_scratch_ptr)},
	{"io_write_scratch_ptr", exec_io_write_scratch_ptr, decompiler_io_write_scratch_ptr, sizeof(op_io_write_scratch_ptr)},

	{"mmio_memset_32", exec_mmio_memset_32, decompiler_mmio_memset_32, sizeof(op_mmio_memset_32)},
	{"mmio_memset_16", exec_mmio_memset_16, decompiler_mmio_memset_16, sizeof(op_mmio_memset_16)},
	{"mmio_memset_8", exec_mmio_memset_8, decompiler_mmio_memset_8, sizeof(op_mmio_memset_8)},

	{"io_memset_32", exec_io_memset_32, decompiler_io_memset_32, sizeof(op_io_memset_32)},
	{"io_memset_16", exec_io_memset_16, decompiler_io_memset_16, sizeof(op_io_memset_16)},
	{"io_memset_8", exec_io_memset_8, decompiler_io_memset_8, sizeof(op_io_memset_8)},

	{"mmio_xor_32", exec_mmio_xor_32, decompiler_mmio_xor_32, sizeof(op_mmio_xor_32)},
	{"mmio_xor_16", exec_mmio_xor_16, decompiler_mmio_xor_16, sizeof(op_mmio_xor_16)},
	{"mmio_xor_8", exec_mmio_xor_8, decompiler_mmio_xor_8, sizeof(op_mmio_xor_8)},

	{"io_xor_32", exec_io_xor_32, decompiler_io_xor_32, sizeof(op_io_xor_32)},
	{"io_xor_16", exec_io_xor_16, decompiler_io_xor_16, sizeof(op_io_xor_16)},
	{"io_xor_8", exec_io_xor_8, decompiler_io_xor_8, sizeof(op_io_xor_8)},

	{"mmio_write_bruteforce_32", exec_mmio_write_bruteforce_32, decompiler_mmio_write_bruteforce_32, sizeof(op_mmio_bruteforce_32)},
	{"mmio_write_bruteforce_16", exec_mmio_write_bruteforce_16, decompiler_mmio_write_bruteforce_16, sizeof(op_mmio_bruteforce_16)},
	{"mmio_write_bruteforce_8", exec_mmio_write_bruteforce_8, decompiler_mmio_write_bruteforce_8, sizeof(op_mmio_bruteforce_8)},
 
	{"io_write_bruteforce_32", exec_io_write_bruteforce_32, decompiler_io_write_bruteforce_32, sizeof(op_io_bruteforce_32)},
	{"io_write_bruteforce_16", exec_io_write_bruteforce_16, decompiler_io_write_bruteforce_16, sizeof(op_io_bruteforce_16)},
	{"io_write_bruteforce_8", exec_io_write_bruteforce_8, decompiler_io_write_bruteforce_8, sizeof(op_io_bruteforce_8)},

	{"mmio_write_dict_32", exec_mmio_write_dict_32, decompiler_mmio_write_dict_32, sizeof(op_mmio_write_dict_32)},
	{"mmio_write_dict_16", exec_mmio_write_dict_16, decompiler_mmio_write_dict_16, sizeof(op_mmio_write_dict_16)},
	{"mmio_write_dict_8", exec_mmio_write_dict_8, decompiler_mmio_write_dict_8, sizeof(op_mmio_write_dict_8)},

	{"mmio_write_scratch_dict_ptr", exec_mmio_write_scratch_dict_ptr, decompiler_mmio_write_scratch_dict_ptr, sizeof(op_mmio_write_scratch_dict_ptr)},
	{"mmio_write_scratch_dict_ptr", exec_mmio_write_scratch_dict_ptr, decompiler_mmio_write_scratch_dict_ptr, sizeof(op_mmio_write_scratch_dict_ptr)},
	{"mmio_write_scratch_dict_ptr", exec_mmio_write_scratch_dict_ptr, decompiler_mmio_write_scratch_dict_ptr, sizeof(op_mmio_write_scratch_dict_ptr)},

	{"io_write_dict_32", exec_io_write_dict_32, decompiler_io_write_dict_32, sizeof(op_io_write_dict_32)},
	{"io_write_dict_16", exec_io_write_dict_16, decompiler_io_write_dict_16, sizeof(op_io_write_dict_16)},
	{"io_write_dict_8", exec_io_write_dict_8, decompiler_io_write_dict_8, sizeof(op_io_write_dict_8)},

	{"mmio_read_dict_32", exec_mmio_read_dict_32, decompiler_mmio_read_dict_32, sizeof(op_mmio_read_dict)},
	{"mmio_read_dict_16", exec_mmio_read_dict_16, decompiler_mmio_read_dict_16, sizeof(op_mmio_read_dict)},
	{"mmio_read_dict_8", exec_mmio_read_dict_8, decompiler_mmio_read_dict_8, sizeof(op_mmio_read_dict)},

	{"io_reads_32", exec_io_reads_32, decompiler_io_reads_32, sizeof(op_io_bruteforce_32)},
	{"io_reads_16", exec_io_reads_16, decompiler_io_reads_16, sizeof(op_io_bruteforce_16)},
	{"io_reads_8", exec_io_reads_8, decompiler_io_reads_8, sizeof(op_io_bruteforce_8)},

	{"io_writes_32", exec_io_writes_32, decompiler_io_writes_32, sizeof(op_io_bruteforce_32)},
	{"io_writes_16", exec_io_writes_16, decompiler_io_writes_16, sizeof(op_io_bruteforce_16)},
	{"io_writes_8", exec_io_writes_8, decompiler_io_writes_8, sizeof(op_io_bruteforce_8)},

	{"mmio_read_32", exec_mmio_read_32, decompiler_mmio_read_32, sizeof(op_mmio_read)},
	{"mmio_read_16", exec_mmio_read_16, decompiler_mmio_read_16, sizeof(op_mmio_read)},
	{"mmio_read_8", exec_mmio_read_8, decompiler_mmio_read_8, sizeof(op_mmio_read)},
	
	{"io_read_32", exec_io_read_32, decompiler_io_read_32, sizeof(op_io_read)},
	{"io_read_16", exec_io_read_16, decompiler_io_read_16, sizeof(op_io_read)},
	{"io_read_8", exec_io_read_8, decompiler_io_read_8, sizeof(op_io_read)},
	
	
	//{"vmport_in", exec_vmport_in, void_handler_decompiler, sizeof(vmport_in)},
	//{"vmport_in_scratch_ptr", exec_vmport_in_scratch_ptr, void_handler_decompiler, sizeof(vmport_in_scratch_ptr)},
	

	//{"kvm_hypercall", exec_kvm_hypercall, void_handler_decompiler, sizeof(kvm_hypercall)},

	//{"msr_xor", exec_msr_xor, void_handler_decompiler, sizeof(msr_xor)},
	//{"msr_xor_2", exec_msr_xor_2, void_handler_decompiler, sizeof(msr_xor)},
	//{"msr_xor_3", exec_msr_xor_3, void_handler_decompiler, sizeof(msr_xor)},

	{"mmio_write_dict_data_32", exec_mmio_write_dict_data_32, decompiler_mmio_write_dict_data_32, sizeof(op_mmio_write_dict_32)},
	{"mmio_write_dict_data_16", exec_mmio_write_dict_data_16, decompiler_mmio_write_dict_data_16, sizeof(op_mmio_write_dict_16)},
	{"mmio_write_dict_data_8", exec_mmio_write_dict_data_8, decompiler_mmio_write_dict_data_8, sizeof(op_mmio_write_dict_8)},

	{"mmio_write_data_32", exec_mmio_write_data_32, decompiler_mmio_write_dict_data_32, sizeof(op_mmio_write_dict_32)},
	{"mmio_write_data_16", exec_mmio_write_data_16, decompiler_mmio_write_dict_data_16, sizeof(op_mmio_write_dict_16)},
	{"mmio_write_data_8", exec_mmio_write_data_8, decompiler_mmio_write_dict_data_8, sizeof(op_mmio_write_dict_8)},

	//{"mmio_write_data_8", exec_x86, void_handler_decompiler, sizeof(op_x86)},
	//
};

uint32_t exec_handler_size = sizeof(exec_handler);