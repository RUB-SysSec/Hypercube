import sys
import glob
import os
import subprocess
from scripts.compile import generate_config_header_file, build_hypercube_image
from scripts.executor.general import ExitReason
from scripts.executor.qemu import QEMUTargetExecutor
from scripts.config import TARGET_DEST

def __find_qemu_executable(target):
    print("[*] Searching for qemu executable path for %s" % target)
    target_path = ""
    if glob.glob(TARGET_DEST + "/%s/x86_64-softmmu/qemu-system-x86_64" % target):
        target_path = glob.glob(TARGET_DEST + "/%s/x86_64-softmmu/qemu-system-x86_64" % target)[0]
    elif glob.glob(TARGET_DEST + "/%s/build/x86_64-softmmu/qemu-system-x86_64" % target):
        target_path = glob.glob(TARGET_DEST + "/%s/build/x86_64-softmmu/qemu-system-x86_64" % target)[0]
    else:
        raise Exception("Target (%s) not found" % target)
    return target_path

def __target_dir(target):
    if target == "AC97" or target == "CS4231a" or target == "ES1370" or target == "Intel-HDA" or target == "SoundBlaster":
        return "/hw/audio/"
    elif target == "IDE-Core":
        return "/hw/ide/"
    elif target == "PCNET-PCI" or target == "NE2000-PCI" or target == "RTL8139" or target == "EEPro100" or target == "E1000":
        return "/hw/net/"
    elif target == "Floppy":
        return "/hw/block/"
    elif target == "Serial" or target == "Parallel":
        return "/hw/char/"
    elif target == "SDHCI":
        return "/hw/sd/"
    else:
        raise Exception("Target (%s) not found" % target)

def __target_file(target):

    if target == "AC97":
        return "ac97.c"
    elif target == "CS4231a":
        return "cs4231a.c"
    elif target == "ES1370":
        return "es1370.c"
    elif target == "Intel-HDA":
        return "intel-hda.c"
    elif target == "SoundBlaster":
        return "sb16.c"
    elif target == "IDE-Core":
        return "core.c"
    elif target == "EEPro100":
        return "eepro100.c"
    elif target == "PCNET-PCI":
        return "pcnet.c"
    elif target == "NE2000-PCI":
        return "ne2000.c"
    elif target == "RTL8139":
        return "rtl8139.c"
    elif target == "E1000":
        return "e1000.c"
    elif target == "Floppy":
        return "fdc.c"
    elif target == "Serial":
        return "serial.c"
    elif target == "Parallel":
        return "parallel.c"
    elif target == "SDHCI":
        return "sdhci.c"
    else:
        raise Exception("Target (%s) not found" % target)

def __reset_gcov(qemu_path, target):

    print("[*] Resetting gcov for %s" % target)

    qemu_directory = qemu_path.split("/x86_64-softmmu/qemu-system-x86_64")[0]

    for file in glob.glob("%s/%s/*.gcda" % (qemu_directory, __target_dir(target))):
        print("[!] Removing %s" % file)
        os.remove(file)

def __generate_eval_configuration(target):

    if target == "AC97" or target == "ES1370" or target == "Intel-HDA":
        configuration = generate_config_header_file( \
            debug=False, \
            enable_pci=True, \
            io_brute_force_range=0xf, \
            target_filters=["MEDIA"])
    elif target == "CS4231a":
        configuration = generate_config_header_file( \
            debug=False, \
            enable_default_io=True, \
            io_brute_force_range=0xf, \
            target_filters=["cs4231a"])
    elif target == "SoundBlaster":
        configuration = generate_config_header_file( \
            debug=False, \
            enable_default_io=True, \
            io_brute_force_range=0xf, \
            target_filters=["sb16"])
    elif target == "IDE-Core":
        configuration = generate_config_header_file( \
            debug=False, \
            enable_default_io=True, \
            enable_pci=True, \
            io_brute_force_range=0xf, \
            target_filters=["IDE"])
    elif target == "Floppy":
        configuration = generate_config_header_file( \
            debug=False, \
            enable_default_io=True, \
            io_brute_force_range=0xf, \
            target_filters=["Floppy"])
    elif target == "Serial":
        configuration = generate_config_header_file( \
            debug=False, \
            enable_default_io=True, \
            io_brute_force_range=0xf, \
            target_filters=["SERIAL"])
    elif target == "Parallel":
        configuration = generate_config_header_file( \
            debug=False, \
            enable_default_io=True, \
            io_brute_force_range=0xf, \
            target_filters=["Parallel"])
    elif target == "PCNET-PCI" or target == "NE2000-PCI" or target == "RTL8139" or target == "EEPro100" or target == "E1000":
        configuration = generate_config_header_file( \
            debug=False, \
            enable_pci=True, \
            io_brute_force_range=0xf, \
            target_filters=["Ethernet"])
    elif target == "SDHCI":
        configuration = generate_config_header_file( \
            debug=False, \
            enable_pci=True, \
            io_brute_force_range=0xf, \
            target_filters=["Unknown"])
    else:
        raise Exception("Target (%s) not found" % target)
    
    return configuration

def target_specific_parameters(target):

    if target == "AC97":
        return "-device AC97"
    elif target == "CS4231a":
        return "-device cs4231a"
    elif target == "ES1370":
        return "-device ES1370"
    elif target == "Intel-HDA":
        return "-device intel-hda -device hda-duplex"
    elif target == "SoundBlaster":
        return "-device sb16"
    elif target == "IDE-Core":
        os.system("qemu-img create /tmp/hdd.img 8K")
        return "-hda /tmp/hdd.img"
    elif target == "PCNET-PCI":
        return "-device pcnet,netdev=net0 -netdev user,id=net0"
    elif target == "NE2000-PCI":
        return " -device ne2k_pci"
    elif target == "RTL8139":
        return "-device rtl8139" # ,netdev=net0 -netdev user,id=net0"
    elif target == "EEPro100":
        return "-device i82550" #,netdev=net0 -netdev user,id=net0"
    elif target == "E1000":
        return "-device e1000,netdev=net0 -netdev user,id=net0"
    elif target == "Floppy":
        f = open("/tmp/floppy.img", "wb")
        f.write(b'\x00' * 1440 * 1024)
        f.close()
        f = open("/tmp/floppy2.img", "wb")
        f.write(b'\x00' * 1440 * 1024)
        f.close()
        return "-fda /tmp/floppy.img"
    elif target == "Serial":
        if os.path.exists("/tmp/A"):
            os.remove("/tmp/A")
        return "-serial file:/tmp/A"
    elif target == "Parallel":
        if os.path.exists("/tmp/A"):
            os.remove("/tmp/A")
        return "-parallel file:/tmp/A"
    elif target == "SDHCI":
        os.system("qemu-img create sd-card.img 10M")
        return "-device sdhci-pci,sd-spec-version=3,uhs=1,capareg=9223372033633091583 -drive format=raw,file=sd-card.img,if=none,id=disk,cache=writeback,discard=unmap -device sd-card,drive=disk"
    else:
        raise Exception("Target (%s) not found" % target)


def __run_gcov_process(dir, cmd, line_coverage=False):
    proc = subprocess.Popen(cmd.split(), stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=dir)

    return_code = proc.wait()

    if line_coverage:
        if return_code != 0:
            err_str = proc.stderr.read().decode('utf-8').strip()
            print("gcov error: %s" % return_code)
            print(err_str)
            sys.exit(0)

        out_str = proc.stdout.read().decode('utf-8').strip()

        return out_str.split("Lines executed:")[1].split("\n")[0].strip().split(" of ")[0].strip()
    else:
        if return_code != 0:
            err_str = proc.stderr.read().decode('utf-8').strip()
            print("gcov error: %s" % return_code)
            print(err_str)
            sys.exit(0)

        out_str = proc.stdout.read().decode('utf-8').strip()

        try:
            return out_str.split("Branches executed:")[1].split("\n")[0].strip().split(" of ")[0].strip()
        except:
            if not os.path.exists("misc/gcov_hooks.so"):
                print("[ ] error: misc/gcov_hooks.so does not exist" )
                sys.exit(1)
            print("[?] no branch coverage?")
            print(out_str)
            sys.exit(1)

def __run_gcov(qemu_dir, target, line_coverage=False):
    target_dir = __target_dir(target)
    target_file = __target_file(target)
    #qemu_dir = __find_qemu_executable(target).split("/x86_64-softmmu/qemu-system-x86_64")[0]

    if line_coverage:
        return __run_gcov_process("%s/%s" % (qemu_dir, target_dir), "gcov %s" % (target_file), line_coverage=True)
    else:
        return __run_gcov_process("%s/%s" % (qemu_dir, target_dir), "gcov -b %s" % (target_file), line_coverage=False)

def handle_coverage(parser, args):
    #print('coverage')

    executable_path = __find_qemu_executable(args.qemu)

    __reset_gcov(executable_path, args.target)


    # generate config header file
    configuration = __generate_eval_configuration(args.target)

    #print(configuration)

    # compile hypercube ISO image
    build_hypercube_image(configuration, path="/tmp/hypercube.iso")

    # run target
    executable_path = os.path.realpath(executable_path)
    image_path = os.path.realpath("/tmp/hypercube.iso")

    extra = target_specific_parameters(args.target)

    runner = QEMUTargetExecutor(executable_path, extra)

    remaining_run_time_sec = args.run_time
    print("[!] Coverage run time: %s" % "{:.2f}".format(remaining_run_time_sec))


    print("[!] Running target: %s" % runner.get_target_cmd_line(image_path))
    #get_target_qemu_cmd_line(executable_path, image_path, extra))

    while remaining_run_time_sec > 0:
        stdout_output, stderr_output, exit_reason_type, exit_reason_str, seed, run_time = runner.run_target( \
            image_path, \
            gcov_mode=True, \
            timeout_sec= min(args.timeout, remaining_run_time_sec), \
            debug=False, deadlock_watchdog=False)

        remaining_run_time_sec -= run_time
        print("[%s sec]\tseed: %s\t %f\t exit_reason: %s - %s" % ("{:.2f}".format(remaining_run_time_sec), seed, run_time, exit_reason_type, exit_reason_str))

        result = __run_gcov(executable_path.split("/x86_64-softmmu/qemu-system-x86_64")[0], args.target)

        print("[!] Coverage (%s): %s" % (args.target, result))

        #print(stdout_output)

        #print(stderr_output)

    # collect gcov data
    result = __run_gcov(executable_path.split("/x86_64-softmmu/qemu-system-x86_64")[0], args.target)



    # print results
    print("[!] Coverage (%s): %s" % (args.target, result))

    return result

    #sys.exit(0)
