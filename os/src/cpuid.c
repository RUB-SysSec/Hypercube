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

#include "cpuid.h"
#include "tty.h"
#include "msr.h"

#define STR_PREFIX	" [CPUID]"

bool supports_vmx(void){
	uint32_t eax = 1;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;

	cpuid(&eax, &ebx, &ecx, &edx);

	//debug_printf("Checking VMX...(%x)\n\r", (CHECK(ecx, f_ecx_bit_VMX)));
	return  (CHECK(ecx, f_ecx_bit_VMX));

	//return 1;
}

static void print_cpu_features(void){
	uint32_t eax = 1;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;

	cpuid(&eax, &ebx, &ecx, &edx);
	debug_printf(STR_PREFIX" features: ");
	if (CHECK(edx, f_edx_bit_FPU))			debug_printf("FPU ");
	if (CHECK(edx, f_edx_bit_VME))			debug_printf("VME ");
	if (CHECK(edx, f_edx_bit_DE)) 			debug_printf("DE ");
	if (CHECK(edx, f_edx_bit_PSE))			debug_printf("PSE ");
	if (CHECK(edx, f_edx_bit_TSC))			debug_printf("TSC ");
	if (CHECK(edx, f_edx_bit_MSR))			debug_printf("MSR ");
	if (CHECK(edx, f_edx_bit_PAE))			debug_printf("PAE ");
	if (CHECK(edx, f_edx_bit_MCE))			debug_printf("MCE ");
	if (CHECK(edx, f_edx_bit_CX8))			debug_printf("CX8 ");
	if (CHECK(edx, f_edx_bit_APIC))			debug_printf("APIC ");
	if (CHECK(edx, f_edx_bit_SEP))			debug_printf("SEP ");
	if (CHECK(edx, f_edx_bit_MTRR))			debug_printf("MTRR ");
	if (CHECK(edx, f_edx_bit_PGE))			debug_printf("PEG ");
	if (CHECK(edx, f_edx_bit_MCA))			debug_printf("MCA ");
	if (CHECK(edx, f_edx_bit_CMOV))	 		debug_printf("CMOV ");
	if (CHECK(edx, f_edx_bit_CMOV))			debug_printf("PSE36 ");
	if (CHECK(edx, f_edx_bit_PSN))			debug_printf("PSN ");
	if (CHECK(edx, f_edx_bit_CLFSH))		debug_printf("CLFSH ");
	if (CHECK(edx, f_edx_bit_DS))			debug_printf("DS ");
	if (CHECK(edx, f_edx_bit_ACPI))			debug_printf("ACPI ");
	if (CHECK(edx, f_edx_bit_MMX))			debug_printf("MMX ");
	if (CHECK(edx, f_edx_bit_FXSR))			debug_printf("FXSR ");
	if (CHECK(edx, f_edx_bit_SSE))			debug_printf("SSE ");
	if (CHECK(edx, f_edx_bit_SSE2))			debug_printf("SSE2 ");
	if (CHECK(edx, f_edx_bit_SS))			debug_printf("SS ");
	if (CHECK(edx, f_edx_bit_HTT))	 		debug_printf("HTT ");
	if (CHECK(edx, f_edx_bit_TM))		  	debug_printf("TM ");
	if (CHECK(edx, f_edx_bit_IA64))	  		debug_printf("IA64 ");
	if (CHECK(edx, f_edx_bit_PBE))	 		debug_printf("PBE ");
	if (CHECK(ecx, f_ecx_bit_SSE3))	 		debug_printf("SSE3 ");
	if (CHECK(ecx, f_ecx_bit_PCLMULQDQ))	debug_printf("PCLMULQDQ ");
	if (CHECK(ecx, f_ecx_bit_DTES64))	 	debug_printf("DTES64 ");
	if (CHECK(ecx, f_ecx_bit_MONITOR)) 		debug_printf("MONITOR ");
	if (CHECK(ecx, f_ecx_bit_DS_CPL))	 	debug_printf("DS_CPL ");
	if (CHECK(ecx, f_ecx_bit_VMX))	 		debug_printf("VMX ");
	if (CHECK(ecx, f_ecx_bit_SMX))	 		debug_printf("SMX ");
	if (CHECK(ecx, f_ecx_bit_EST))	 		debug_printf("EST ");
	if (CHECK(ecx, f_ecx_bit_TM2))	 		debug_printf("TM2 ");
	if (CHECK(ecx, f_ecx_bit_SSSE3))	 	debug_printf("SSSE3 ");
	if (CHECK(ecx, f_ecx_bit_CNXT_ID))		debug_printf("CNXT_ID ");
	if (CHECK(ecx, f_ecx_bit_SDBG))	 		debug_printf("SDBG ");
	if (CHECK(ecx, f_ecx_bit_FMA))	 		debug_printf("FMA ");
	if (CHECK(ecx, f_ecx_bit_CX16))	 		debug_printf("CX16 ");
	if (CHECK(ecx, f_ecx_bit_XTPR))	 		debug_printf("XTPR ");
	if (CHECK(ecx, f_ecx_bit_PDCM))	 		debug_printf("PDCM ");
	if (CHECK(ecx, f_ecx_bit_PCID))	 		debug_printf("PCID ");
	if (CHECK(ecx, f_ecx_bit_DCA))	 		debug_printf("DCA ");
	if (CHECK(ecx, f_ecx_bit_SSE41))	 	debug_printf("SSE41 ");
	if (CHECK(ecx, f_ecx_bit_SSE42))	 	debug_printf("SSE42 ");
	if (CHECK(ecx, f_ecx_bit_X2APIC))	 	debug_printf("X2APIC ");
	if (CHECK(ecx, f_ecx_bit_MOVBE))	 	debug_printf("MOVBE ");
	if (CHECK(ecx, f_ecx_bit_POPCNT))	 	debug_printf("POPCNT ");
	if (CHECK(ecx, f_ecx_bit_TSC_DL))	 	debug_printf("TSC_DL ");
	if (CHECK(ecx, f_ecx_bit_AES))	 		debug_printf("AES ");
	if (CHECK(ecx, f_ecx_bit_XSAVE))	 	debug_printf("XSAVE ");
	if (CHECK(ecx, f_ecx_bit_OXSAVE))	 	debug_printf("OXSAVE ");
	if (CHECK(ecx, f_ecx_bit_AVX))	 		debug_printf("AVX ");
	if (CHECK(ecx, f_ecx_bit_F16C))	 		debug_printf("F16C ");
	if (CHECK(ecx, f_ecx_bit_RDRND))	 	debug_printf("RDRND ");
	if (CHECK(ecx, f_ecx_bit_HYPERVISOR))	debug_printf("HYPERVISOR ");
	debug_printf("\n\r");
}

static void print_extended_cpu_features(void){
	uint32_t eax = 7;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;

	cpuid(&eax, &ebx, &ecx, &edx);
	debug_printf(STR_PREFIX" extended features: ");
	if (CHECK(ebx, ef_ebx_bit_FSGSBASE))		debug_printf("FSGSBASE ");
	if (CHECK(ebx, ef_ebx_bit_TSC_ADJUST))		debug_printf("TSC_ADJUST ");
	if (CHECK(ebx, ef_ebx_bit_SGX))				debug_printf("SGX ");
	if (CHECK(ebx, ef_ebx_bit_BM1))				debug_printf("BM1 ");
	if (CHECK(ebx, ef_ebx_bit_HLE))				debug_printf("HLE ");
	if (CHECK(ebx, ef_ebx_bit_AVX))				debug_printf("AVX ");
	if (CHECK(ebx, ef_ebx_bit_SMEP))			debug_printf("SMEP ");
	if (CHECK(ebx, ef_ebx_bit_BMI2))			debug_printf("BMI2 ");
	if (CHECK(ebx, ef_ebx_bit_ERMS))			debug_printf("ERMS ");
	if (CHECK(ebx, ef_ebx_bit_INVPCID))			debug_printf("INVPCID ");
	if (CHECK(ebx, ef_ebx_bit_RTM))				debug_printf("RTM ");
	if (CHECK(ebx, ef_ebx_bit_PQM))				debug_printf("PQM ");
	if (CHECK(ebx, ef_ebx_bit_MPX))				debug_printf("MPX ");
	if (CHECK(ebx, ef_ebx_bit_PQE))				debug_printf("PQE ");
	if (CHECK(ebx, ef_ebx_bit_AVX512F))			debug_printf("AVX512F ");
	if (CHECK(ebx, ef_ebx_bit_AVX512DQ))		debug_printf("AVX512DQ ");
	if (CHECK(ebx, ef_ebx_bit_RDSEED))			debug_printf("RDSEED ");
	if (CHECK(ebx, ef_ebx_bit_ADX))				debug_printf("ADX ");
	if (CHECK(ebx, ef_ebx_bit_SMAP))			debug_printf("SMAP ");
	if (CHECK(ebx, ef_ebx_bit_AVX512IFMA))		debug_printf("AVX512IFMA ");
	if (CHECK(ebx, ef_ebx_bit_PCOMMIT))			debug_printf("PCOMMIT ");
	if (CHECK(ebx, ef_ebx_bit_CLFLUSHOPT))		debug_printf("CLFLUSHOPT ");
	if (CHECK(ebx, ef_ebx_bit_CLWB))			debug_printf("CLWB ");
	if (CHECK(ebx, ef_ebx_bit_INTEL_PT))		debug_printf("INTEL_PT ");
	if (CHECK(ebx, ef_ebx_bit_AVX512PF))		debug_printf("AVX512PF ");
	if (CHECK(ebx, ef_ebx_bit_AVX512ER))		debug_printf("AVX512ER ");
	if (CHECK(ebx, ef_ebx_bit_AVX512CD))		debug_printf("AVX512CD ");
	if (CHECK(ebx, ef_ebx_bit_SHA))				debug_printf("SHA ");
	if (CHECK(ebx, ef_ebx_bit_AVX512BW))		debug_printf("AVX512BW ");
	if (CHECK(ebx, ef_ebx_bit_AVX512VL))		debug_printf("AVX512VL ");
	if (CHECK(ecx, ef_ecx_bit_PREFETCHWT1))		debug_printf("PREFETCHWT1 ");
	if (CHECK(ecx, ef_ecx_bit_AVX512VBMI))		debug_printf("AVX512VBMI ");
	if (CHECK(ecx, ef_ecx_bit_UMIP))			debug_printf("UMIP ");
	if (CHECK(ecx, ef_ecx_bit_PKU))				debug_printf("PKU ");
	if (CHECK(ecx, ef_ecx_bit_OSPKE))			debug_printf("OSPKE ");
	if (CHECK(ecx, ef_ecx_bit_AVX512VPOPCNTFQ))	debug_printf("AVX512VPOPCNTFQ ");
	if (CHECK(ecx, ef_ecx_bit_RDPID))			debug_printf("RDPID ");
	if (CHECK(ecx, ef_ecx_bit_SGX_LC))			debug_printf("SGX_LC ");
	if (CHECK(edx, ef_edx_bit_AVX512_4VNIW))	debug_printf("AVX512_4VNIW ");
	if (CHECK(edx, ef_edx_bit_AVX512_4FMAPS))	debug_printf("AVX512_4FMAPS ");
	debug_printf("\n\r");
}

static void print_cpu_vendor(void){
	uint32_t eax = 0;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	volatile uint8_t vendor[16] = {0};

	cpuid(&eax, &ebx, &ecx, &edx);
	vendor[0] = (ebx >> 0)  & 0xFF;
	vendor[1] = (ebx >> 8)  & 0xFF;
	vendor[2] = (ebx >> 16) & 0xFF;
	vendor[3] = (ebx >> 24) & 0xFF;
	vendor[4] = (edx >> 0)  & 0xFF;
	vendor[5] = (edx >> 8)  & 0xFF;
	vendor[6] = (edx >> 16) & 0xFF;
	vendor[7] = (edx >> 24) & 0xFF;
	vendor[8] =  (ecx >> 0)  & 0xFF;
	vendor[9] =  (ecx >> 8)  & 0xFF;
	vendor[10] = (ecx >> 16) & 0xFF;
	vendor[11] = (ecx >> 24) & 0xFF;

	debug_printf(STR_PREFIX" vendor:  %s\n\r", vendor);

}

static void print_cpu_brand(void){
	uint32_t eax = 0x80000002;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	uint8_t pname[48] = {0};

	cpuid(&eax, &ebx, &ecx, &edx);
	pname[0] =   (eax >>  0) & 0xFF;
	pname[1] =   (eax >>  8) & 0xFF;
	pname[2] =   (eax >> 16) & 0xFF;
	pname[3] =   (eax >> 24) & 0xFF;
	pname[4] =   (ebx >>  0) & 0xFF;
	pname[5] =   (ebx >>  8) & 0xFF;
	pname[6] =   (ebx >> 16) & 0xFF;
	pname[7] =   (ebx >> 24) & 0xFF;
	pname[8] =   (ecx >>  0) & 0xFF;
	pname[9] =   (ecx >>  8) & 0xFF;
	pname[10] =  (ecx >> 16) & 0xFF;
	pname[11] =  (ecx >> 24) & 0xFF;
	pname[12] =  (edx >>  0) & 0xFF;
	pname[13] =  (edx >>  8) & 0xFF;
	pname[14] =  (edx >> 16) & 0xFF;
	pname[15] =  (edx >> 24) & 0xFF;

	eax = 0x80000003;
	cpuid(&eax, &ebx, &ecx, &edx);
	pname[16] =  (eax >>  0) & 0xFF;
	pname[17] =  (eax >>  8) & 0xFF;
	pname[18] =  (eax >> 16) & 0xFF;
	pname[19] =  (eax >> 24) & 0xFF;
	pname[20] =  (ebx >>  0) & 0xFF;
	pname[21] =  (ebx >>  8) & 0xFF;
	pname[22] =  (ebx >> 16) & 0xFF;
	pname[23] =  (ebx >> 24) & 0xFF;
	pname[24] =  (ecx >>  0) & 0xFF;
	pname[25] =  (ecx >>  8) & 0xFF;
	pname[26] =  (ecx >> 16) & 0xFF;
	pname[27] =  (ecx >> 24) & 0xFF;
	pname[28] =  (edx >>  0) & 0xFF;
	pname[29] =  (edx >>  8) & 0xFF;
	pname[30] =  (edx >> 16) & 0xFF;
	pname[31] =  (edx >> 24) & 0xFF;

	eax = 0x80000004;
	cpuid(&eax, &ebx, &ecx, &edx);
	pname[32] =  (eax >>  0) & 0xFF;
	pname[33] =  (eax >>  8) & 0xFF;
	pname[34] =  (eax >> 16) & 0xFF;
	pname[35] =  (eax >> 24) & 0xFF;
	pname[36] =  (ebx >>  0) & 0xFF;
	pname[37] =  (ebx >>  8) & 0xFF;
	pname[38] =  (ebx >> 16) & 0xFF;
	pname[39] =  (ebx >> 24) & 0xFF;
	pname[40] =  (ecx >>  0) & 0xFF;
	pname[41] =  (ecx >>  8) & 0xFF;
	pname[42] =  (ecx >> 16) & 0xFF;
	pname[43] =  (ecx >> 24) & 0xFF;
	pname[44] =  (edx >>  0) & 0xFF;
	pname[45] =  (edx >>  8) & 0xFF;
	pname[46] =  (edx >> 16) & 0xFF;
	pname[47] =  (edx >> 24) & 0xFF;
	
	debug_printf(STR_PREFIX" product: %s\n\r", pname);
}

static void print_cpu_info(void){
	uint32_t eax = 0;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	uint32_t highest_function, cpuid_value, stepping, model, family, type, extmodel, extfamily;

	cpuid(&eax, &ebx, &ecx, &edx);
	highest_function = eax;

	eax = 1;
	cpuid(&eax, &ebx, &ecx, &edx);
	cpuid_value = 	eax;
	stepping = 		eax & 0xF;
	model = 		(eax >> 4) & 0xF;
	family = 		(eax >> 8) & 0xF;
	type = 			(eax >> 12) & 0x3;
	extmodel = 		(eax >> 16) & 0xF;
	extfamily = 	(eax >> 20) & 0xFF;

	debug_printf(STR_PREFIX" hfunc: %d, type=%d family=%d model=%d stepping=%d extfam=%d extmodel=%d\n\r",
					highest_function, cpuid_value,
					type, family, model, stepping, extfamily, extmodel);
}

static void print_cpu_vendor_pt(void){
	uint32_t eax = 0;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	volatile uint8_t vendor[16] = {0};

	cpuid(&eax, &ebx, &ecx, &edx);
	vendor[0] = (ebx >> 0)  & 0xFF;
	vendor[1] = (ebx >> 8)  & 0xFF;
	vendor[2] = (ebx >> 16) & 0xFF;
	vendor[3] = (ebx >> 24) & 0xFF;
	vendor[4] = (edx >> 0)  & 0xFF;
	vendor[5] = (edx >> 8)  & 0xFF;
	vendor[6] = (edx >> 16) & 0xFF;
	vendor[7] = (edx >> 24) & 0xFF;
	vendor[8] =  (ecx >> 0)  & 0xFF;
	vendor[9] =  (ecx >> 8)  & 0xFF;
	vendor[10] = (ecx >> 16) & 0xFF;
	vendor[11] = (ecx >> 24) & 0xFF;

	debug_printf("vendor:\n\r-> %s\n\r", vendor);

}

static void print_cpu_brand_pt(void){
	uint32_t eax = 0x80000002;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	uint8_t pname[48] = {0};

	cpuid(&eax, &ebx, &ecx, &edx);
	pname[0] =   (eax >>  0) & 0xFF;
	pname[1] =   (eax >>  8) & 0xFF;
	pname[2] =   (eax >> 16) & 0xFF;
	pname[3] =   (eax >> 24) & 0xFF;
	pname[4] =   (ebx >>  0) & 0xFF;
	pname[5] =   (ebx >>  8) & 0xFF;
	pname[6] =   (ebx >> 16) & 0xFF;
	pname[7] =   (ebx >> 24) & 0xFF;
	pname[8] =   (ecx >>  0) & 0xFF;
	pname[9] =   (ecx >>  8) & 0xFF;
	pname[10] =  (ecx >> 16) & 0xFF;
	pname[11] =  (ecx >> 24) & 0xFF;
	pname[12] =  (edx >>  0) & 0xFF;
	pname[13] =  (edx >>  8) & 0xFF;
	pname[14] =  (edx >> 16) & 0xFF;
	pname[15] =  (edx >> 24) & 0xFF;

	eax = 0x80000003;
	cpuid(&eax, &ebx, &ecx, &edx);
	pname[16] =  (eax >>  0) & 0xFF;
	pname[17] =  (eax >>  8) & 0xFF;
	pname[18] =  (eax >> 16) & 0xFF;
	pname[19] =  (eax >> 24) & 0xFF;
	pname[20] =  (ebx >>  0) & 0xFF;
	pname[21] =  (ebx >>  8) & 0xFF;
	pname[22] =  (ebx >> 16) & 0xFF;
	pname[23] =  (ebx >> 24) & 0xFF;
	pname[24] =  (ecx >>  0) & 0xFF;
	pname[25] =  (ecx >>  8) & 0xFF;
	pname[26] =  (ecx >> 16) & 0xFF;
	pname[27] =  (ecx >> 24) & 0xFF;
	pname[28] =  (edx >>  0) & 0xFF;
	pname[29] =  (edx >>  8) & 0xFF;
	pname[30] =  (edx >> 16) & 0xFF;
	pname[31] =  (edx >> 24) & 0xFF;

	eax = 0x80000004;
	cpuid(&eax, &ebx, &ecx, &edx);
	pname[32] =  (eax >>  0) & 0xFF;
	pname[33] =  (eax >>  8) & 0xFF;
	pname[34] =  (eax >> 16) & 0xFF;
	pname[35] =  (eax >> 24) & 0xFF;
	pname[36] =  (ebx >>  0) & 0xFF;
	pname[37] =  (ebx >>  8) & 0xFF;
	pname[38] =  (ebx >> 16) & 0xFF;
	pname[39] =  (ebx >> 24) & 0xFF;
	pname[40] =  (ecx >>  0) & 0xFF;
	pname[41] =  (ecx >>  8) & 0xFF;
	pname[42] =  (ecx >> 16) & 0xFF;
	pname[43] =  (ecx >> 24) & 0xFF;
	pname[44] =  (edx >>  0) & 0xFF;
	pname[45] =  (edx >>  8) & 0xFF;
	pname[46] =  (edx >> 16) & 0xFF;
	pname[47] =  (edx >> 24) & 0xFF;
	
	debug_printf("product:\n\r-> %s\n\r", pname);
}

bool check_support_intel_pt(void){
	terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_GREEN));

	debug_printf("Checking Intel PT support...\n\r");

	print_cpu_vendor_pt();
	print_cpu_brand_pt();
	uint32_t eax = 1;
	uint32_t ebx = 0;
	uint32_t ecx = 0;
	uint32_t edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if(eax < 0x14){
		terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		debug_printf("ERROR: Not enough CPUID support for Intel PT!\n\r");
		return false;
	}
	debug_printf("[X] CPUID > 0x14 (0x%x)\n\r", eax);

	eax = 0x7;
	ebx = 0;
	ecx = 0;
	edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if(!(ebx & BIT(25))){
		terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		debug_printf("ERROR: No Intel PT support!\n\r");
		return false;
	}
	debug_printf("[X] Basic Intel PT\n\r");

	eax = 0x14;
	ebx = 0;
	ecx = 0;
	edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);
	if(!(ecx & BIT(0))){
		terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		debug_printf("ERROR: No ToPA support!\n\r");
		return false;
	}
	debug_printf("[X] ToPA Support\n\r");

	if(!(ebx & BIT(0))){
		terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		debug_printf("ERROR: CR3 filtering unsupported!\n\r");
		return false;
	}
	debug_printf("[X] CR3 filtering\n\r");

	if(ecx & BIT(31)){
		terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		debug_printf("ERROR: IP Payloads are LIP!\n\r");
		return false;
	}
	debug_printf("[X] IP != LIP\n\r");


	if(!(ecx & BIT(1))){
		terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		debug_printf("ERROR: Only one ToPA block supported!\n\r");
		return false;
	}
	debug_printf("[X] Multi ToPA\n\r");


	if(!(ebx & BIT(2))){
		terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		debug_printf("ERROR: No IP-Filtering support!\n\r");
		return false;
	}
	debug_printf("[X] IP-Filtering\n\r");


	uint64_t msr_value = rdmsr64(0x00000485); // MSR_IA32_VMX_MISC  

	if (!(msr_value & BIT(14))){
		terminal_setcolor(vga_entry_color(VGA_COLOR_BLACK, VGA_COLOR_LIGHT_RED));
		debug_printf("ERROR: VMX operations are not supported in Intel PT tracing mode!");
		return false; 
	}
	debug_printf("[X] Intel PT + VMX\n\r");


	eax = 0x14;
	ebx = 0;
	ecx = 1;
	edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);

	debug_printf("[X] Trace Regions: %d!\n\r", eax&0x7);


	eax = 0x14;
	ebx = 0;
	ecx = 0;
	edx = 0;
	cpuid(&eax, &ebx, &ecx, &edx);

	if(!(ebx & BIT(4))){
		debug_printf("[ ] PTWRITE\n\r");
	}
	else{
		debug_printf("[X] PTWRITE\n\r");
	}

	debug_printf("--> Intel PT is supported!\n\r");
	return true;
}

void print_cpuid(void){
	print_cpu_info();
	print_cpu_vendor();
	print_cpu_brand();
	print_cpu_features();
	print_extended_cpu_features();
}