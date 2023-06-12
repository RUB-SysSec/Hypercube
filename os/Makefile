#
# HyperCube OS
# (c) Sergej Schumilo, 2019 <sergej@schumilo.de> 
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
# 


AS=as --32
CC=gcc -m32

CFLAGS=-c -std=gnu99 -ffreestanding -O3 -Wall -Wextra -static -DHYPERCUBE -DLOOP #-DDECOMPILER #-DINTERPRETER_BENCHMARK #-DDECOMPILER 
LDFLAGS=-T misc/linker.ld -ffreestanding -O3 -nostdlib -static
LIBS=-lgcc
INCLUDE=-Iinclude -Itesseract

AS_SOURCES= src/asm/boot.s src/asm/cpu_tables.s src/asm/ap_boot.s
AS_OBJECTS=$(AS_SOURCES:.s=.o)

SOURCES=src/kernel.c src/libk.c src/serial.c src/tty.c src/gdt.c src/idt.c src/isr.c src/pci.c src/panic.c src/msr.c src/test.c src/acpi.c src/cpuid.c src/apic.c src/pic.c src/mboot.c src/efi.c src/mem.c src/smp.c src/fuzz.c src/pio.c tesseract/core.c tesseract/state.c tesseract/opcodes.c tesseract/decompiler.c tesseract/dict.c tesseract/handler.c
OBJECTS=$(SOURCES:.c=.o) bin/logo.o

.PHONY: all clean run 

all: clean misc/linker.ld $(AS_SOURCES) $(SOURCES) hypercube_os.bin 

bin/logo.o:
	objcopy -I binary -O elf32-i386 -B i386 misc/logo.bmp bin/logo.o

bin/payload.o:
	objcopy -I binary -O elf32-i386 -B i386 --rename-section .data=.runtime_data misc/crash.hexa bin/payload.o

hypercube_os_crash.bin: CFLAGS := $(CFLAGS) -DPAYLOAD
hypercube_os_crash.bin: clean $(AS_OBJECTS) $(OBJECTS) bin/payload.o
	$(CC) $(LDFLAGS) -o bin/hypercube_os.bin $(AS_OBJECTS) $(OBJECTS) $(LIBS) bin/payload.o
	rm -rf isofiles/
	mkdir isofiles
	mkdir isofiles/boot
	mkdir isofiles/boot/grub
	cp misc/grub.cfg isofiles/boot/grub/
	cp bin/hypercube_os.bin isofiles/boot/kernel.bin
	./grub/bios/bin/grub-mkrescue --xorriso=./xorriso -d ./grub/bios/lib/grub/i386-pc/ -o iso/hypercube_os_bios_crash.iso isofiles 
	rm -r isofiles/

hypercube_os.bin: $(AS_OBJECTS) $(OBJECTS)
	$(CC) $(LDFLAGS) -o bin/hypercube_os.bin $(AS_OBJECTS) $(OBJECTS) $(LIBS)
	rm -rf isofiles/
	mkdir isofiles
	mkdir isofiles/boot
	mkdir isofiles/boot/grub
	cp misc/grub.cfg isofiles/boot/grub/
	cp bin/hypercube_os.bin isofiles/boot/kernel.bin
	./grub/bios/bin/grub-mkrescue --xorriso=./xorriso -d ./grub/bios/lib/grub/i386-pc/ -o iso/hypercube_os_bios.iso isofiles 

efi: clean hypercube_os.bin
	$(CC) $(LDFLAGS) -o bin/hypercube_os.bin $(AS_OBJECTS) $(OBJECTS) $(LIBS)
	rm -rf isofiles/
	mkdir isofiles
	mkdir isofiles/boot
	mkdir isofiles/boot/grub
	cp misc/grub.cfg isofiles/boot/grub/
	cp bin/hypercube_os.bin isofiles/boot/kernel.bin
	./grub/efi/bin/grub-mkrescue --xorriso=./xorriso  -d ./grub/efi/lib/grub/i386-efi/ -o iso/hypercube_os_efi.iso  isofiles 
	rm -r isofiles/

efi-app: hypercube_os.bin
	$(CC) $(LDFLAGS) -o bin/hypercube_os.bin $(AS_OBJECTS) $(OBJECTS) $(LIBS)
	rm -rf isofiles/
	mkdir isofiles
	mkdir isofiles/boot
	mkdir isofiles/boot/grub
	cp misc/grub.cfg isofiles/boot/grub/
	cp bin/hypercube_os.bin isofiles/boot/kernel.bin
	./grub/efi_app/bin/grub-mkrescue --xorriso=./xorriso -d ./grub/efi_app/lib/grub/i386-efi/ -o iso/hypercube_os_efi_app.iso isofiles 
	rm -r isofiles/

.s.o:
	$(AS) $< -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@ $(INCLUDE)

clean:
	rm -rf isofiles/
	rm -f $(AS_OBJECTS)
	rm -f $(OBJECTS)
	rm -f bin/hypercube_os.bin
	rm -f iso/hypercube_os_bios.iso
	rm -f iso/hypercube_os_efi.iso
	rm -f iso/hypercube_os_efi_app.iso
	rm -f bin/payload.o
