import os
import subprocess
import sys
import shutil

def __config_extra_areas(extra_areas):
    content = ""
    if extra_areas and len(extra_areas) > 0:    

        content += "\ntypedef struct{\n"
        content += "    uint32_t base;\n"
        content += "    uint32_t length;\n"
        content += "    uint8_t type;\n"
        content += "    char name[128];\n"
        content += "} extra_areas_t;\n\n"


        content += "\n#define EXTRA_AREAS "
        content += "extra_areas_t extra_areas[] = {\\\n"

        for extra_area in extra_areas:
            
            try:
                base = int(extra_area[0], 16)
            except ValueError:
                print("[!] Error: Invalid base address for extra area: %s" % extra_area[0])
                sys.exit(1)
            
            try:
                size = int(extra_area[1], 16)
            except ValueError:
                print("[!] Error: Invalid size for extra area: %s" % extra_area[1])
                sys.exit(1)

            try:    
                type = int(extra_area[2], 10)
            except ValueError:
                print("[!] Error: Invalid type for extra area: %s" % extra_area[2])
                sys.exit(1)

    
            if type != 0 and type != 1:
                print("[!] Error: Invalid type for extra area: %s" % extra_area[2])
                sys.exit(1)

            name = extra_area[3]

            content += "    {0x%08x, 0x%08x, %d, \"%s\"},\\\n" % (base, size, type, name)

        content += "};\n"
    return content

def __config_filters(filters):
    content = ""
    if filters and len(filters) > 0:
        content += "\n#define TARGET_FILTERS "
        content += "char* target_filters[] = {\\\n"

        for filter in filters:
            content += "    \"%s\",\\\n" % filter

        content += "};\n"

    return content

def generate_config_header_file(debug=False, \
                                enable_io=False, \
                                enable_default_io=False, \
                                enable_pci=False, \
                                enable_e_pci=False, \
                                enable_apic=False, \
                                enable_hpet=False, \
                                enable_auto_reboot=False, \
                                seed=None, \
                                io_brute_force_range=None, \
                                extra_areas=None, \
                                target_filters=None, \
                                disable_fuzzer=False, \
                                enable_vga=False):
    content = "#pragma once\n\n"

    if not enable_io and not enable_default_io and not enable_pci and not enable_e_pci and not enable_apic and not enable_hpet and not (extra_areas and len(extra_areas) > 0):
        print("[!] Error: No fuzzing target selected!")
        sys.exit(1)

    content += "#define ENABLE_TTY_SERIAL\n"
    content += "#define EARLY_BOOT_PRINTF\n"

    if debug:
        content += "#define DEBUG_PRINTF\n"

    if enable_io:
        content += "#define IO_FUZZING\n"
    if enable_default_io:
        content += "#define DEFAULT_IO_FUZZING\n"
    if enable_pci:
        content += "#define PCI_FUZZING\n"
    if enable_e_pci:
        content += "#define EXTENDED_PCI_FUZZING\n"
    if enable_apic:
        content += "#define APIC_FUZZING\n"
    if enable_hpet:
        content += "#define HPET_FUZZING\n"

    if enable_auto_reboot:
        content += "#define PANIC_AUTO_REBOOT\n"

    if seed:
        try:
            hex_value = int(seed, 16)
        except:
            print("[!] Error: Invalid seed value!")
            sys.exit(1)
        content += "#define SEED %s\n" % hex(hex_value)

    if io_brute_force_range:
        content += "#define IO_BRUTE_FORCE_RANGE %d\n" % io_brute_force_range

    content += __config_extra_areas(extra_areas)
    content += __config_filters(target_filters)

    if disable_fuzzer:
        content += "#define DISABLE_FUZZER\n"

    if enable_vga:
        content += "#define ENABLE_TTY_VGA\n"

    return content

def build_hypercube_image(config, path=None, uefi=False):

    if uefi:
        print("[*] Compiling hypercube image (UEFI)...")
    else:
        print("[*] Compiling hypercube image (BIOS)...")

    f = open("config.h", "w")
    f.write(config)
    f.close()

    cmd = "make"
    if uefi:
        cmd += " efi"

    proc = subprocess.Popen(cmd.split(" "), stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, cwd=(os.getcwd()+"/os"))
    _, error = proc.communicate()

    # Check if the program returned a non-zero exit code
    if proc.returncode != 0:
        # Print the standard error output
        print("Error:", error.decode('utf-8'))
        sys.exit(1)

    if path:
        # TODO: fix for EFI
        print("[*] Moving image to %s" % path)

        if uefi:
            shutil.move("os/iso/hypercube_os_efi.iso", path)
        else:
            shutil.move("os/iso/hypercube_os_bios.iso", path)

    print("[*] Image created successfully!")

def build_hypercube_payload_image(config, payload_blob_path, path=None, uefi=False):

    #print("[*] Compiling hypercube image...")

    f = open("config.h", "w")
    f.write(config)
    f.close()

    shutil.copyfile(payload_blob_path, "os/misc/crash.hexa")

    proc = subprocess.Popen(["make", "hypercube_os_crash.bin"], stdout=subprocess.DEVNULL, stderr=subprocess.PIPE, cwd=(os.getcwd()+"/os"))
    _, error = proc.communicate()

    # Check if the program returned a non-zero exit code
    if proc.returncode != 0:
        # Print the standard error output
        print("Error:", error.decode('utf-8'))
        sys.exit(1)

    if path:
        if uefi:
            shutil.move("os/iso/hypercube_os_efi_crash.iso", path)
        else:
            shutil.move("os/iso/hypercube_os_bios_crash.iso", path)

    #print("[*] Payload image created successfully!")