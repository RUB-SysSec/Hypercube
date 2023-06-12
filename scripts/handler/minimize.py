import sys
import os
import glob
from scripts.executor.general import ExitReason
from scripts.executor.qemu import QEMUTargetExecutor
from scripts.compile import generate_config_header_file, build_hypercube_payload_image
from scripts.handler.coverage import build_hypercube_image
from scripts.handler.decompiler import run_decompiler
import subprocess
import random
import shutil
import re
import uuid

from scripts.config import TARGET_DEST

BLOCK_SIZE = 0x400
BITMAP_SIZE = 32
TOTAL_BLOCK_SIZE = BLOCK_SIZE + BITMAP_SIZE

def __remove_hexadecimal_addresses(input_str):
    hex_substrings = re.findall(r'0[xX][0-9a-fA-F]+', input_str)
    new_str = input_str
    for i in hex_substrings:
        new_str = new_str.replace(i, "********")

    return new_str

def __remove_tmp_files(temp_file_path):
    if os.path.exists(temp_file_path):
        os.remove(temp_file_path)

    if os.path.exists(temp_file_path + ".iso"):
        os.remove(temp_file_path + ".iso")

    if os.path.exists(temp_file_path + ".extra"):
        os.remove(temp_file_path + ".extra")

def get_number_of_opcodes(payload_blob_path):
    cmd = "./tesseract_tool count %s" % payload_blob_path
    proc = subprocess.Popen(cmd.split(" "), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, cwd=(os.getcwd()+"/os/tools/"))
    stdout, _ = proc.communicate()

    if "[!] opcodes: " in stdout.decode('utf-8').splitlines()[-1]:
        line = stdout.decode('utf-8').splitlines()[-1]
        number = int(line.split("[!] opcodes: ")[1])
        return int(number)
    
    raise Exception("Could not get number of opcodes")

def trim_payload(input_path, output_path, range_start, range_end, remove_empty_blocks):

    if remove_empty_blocks:
        cmd = "./tesseract_tool trim %s %s %d %d 1" % (input_path, output_path, range_start, range_end)
    else:
        cmd = "./tesseract_tool trim %s %s %d %d 0" % (input_path, output_path, range_start, range_end)
    proc = subprocess.Popen(cmd.split(" "), stdout=subprocess.PIPE, stderr=subprocess.PIPE, cwd=(os.getcwd()+"/os/tools/"))
    stdout, stderr = proc.communicate()

    #print(stdout.decode('utf-8'))
    #print(stderr.decode('utf-8'))

    if "[!] removing opcodes from range" in stdout.decode('utf-8').splitlines()[-1]:
        #line = stdout.decode('utf-8').splitlines()[-1]
        #print(line)
        return True
    
    raise Exception("Could not perform minimization step")

def check_minimization_step(runner, payload_blob_path, image_path, config, expected_exit_type=None, expected_exit_reason_str="", tmp_file_path="/tmp/minimization_payload.bin", memlimit=None, extra_file_path=None):

    if expected_exit_type == None:
        raise Exception("Expected exit type not set")

    # compile new image
    build_hypercube_payload_image(config, payload_blob_path, path=image_path)

    if extra_file_path:
        shutil.copyfile(tmp_file_path+".extra", extra_file_path)

    # perform dry run to output the configuration
    stdout_output, stderr_output, exit_reason_type, exit_reason_str, seed, run_time = runner.run_target(image_path, timeout_sec=10.0, memlimit=memlimit)

    #print("Exit reason: %s / %s" % (exit_reason_type, exit_reason_str))

    #return true if crash still occurs
    if exit_reason_type == expected_exit_type and __remove_hexadecimal_addresses(exit_reason_str) == expected_exit_reason_str:
        return True

    if exit_reason_type == expected_exit_type:
        print("[!] crash reason changed to: %s" % __remove_hexadecimal_addresses(exit_reason_str))
    #    return False
    
    #print("-> exit_reason_type: %s" % exit_reason_type)
    #print("Exit reason: %s / %s" % (exit_reason_type, exit_reason_str))
    return False

def run_minization_step(runner, payload_blob_path, image_path, config, expected_exit_type=None, expected_exit_reason_str="", number_of_attempts=10, tmp_file_path="/tmp/minimization_payload.bin", memlimit=None, extra_file_path=None):

    attempts = 0

    print("[*] starting minimization...")
    image_tmp_path = tmp_file_path+".iso"

    while True:
    
        if attempts == number_of_attempts:
            print("[*] minimization finished (number of max attempts reached)")
            return True

        number_of_opcodes = get_number_of_opcodes(payload_blob_path)

        if number_of_opcodes == 1:
            print("[*] minimization finished")
            return True

        while True:
            choice = random.randint(0, 10)
            if choice == 0:
                a = random.randint(0, number_of_opcodes)
                b = number_of_opcodes
            elif choice == 1:
                a = 0
                b = random.randint(0, number_of_opcodes)
            else:
                a = random.randint(0, number_of_opcodes)
                b = random.randint(0, number_of_opcodes)

            if a != b:
                break

            
        remove_empty_blocks = False
        if random.randint(0, 5) == 0:
            remove_empty_blocks = True

        file_size = os.stat(payload_blob_path).st_size
        print("\t[!] remaining opcodes: %d (size: %d bytes) / removing range: %d - %d (remove_empty_blocks: %d)" % (number_of_opcodes, file_size, min(a,b), max(a,b), remove_empty_blocks), end="\t")
        trim_payload(payload_blob_path, tmp_file_path, min(a,b), max(a,b), remove_empty_blocks)

        res = check_minimization_step(runner, tmp_file_path, image_tmp_path, config, expected_exit_type=expected_exit_type, expected_exit_reason_str=expected_exit_reason_str, tmp_file_path=tmp_file_path, memlimit=memlimit, extra_file_path=extra_file_path)
        
        #print(res)
        if res:
            print(" -> success")
            shutil.copyfile(tmp_file_path, payload_blob_path)
            shutil.copyfile(image_tmp_path, image_path)
            attempts = 0
        else:
            attempts += 1
            print(" -> failed (attempt %d / %d)" % (attempts, number_of_attempts))


def generate_payload(payload_blob_path, blocks, rng_state_a, rng_state_b, rng_state_c, rng_state_d):
    cmd = "./tesseract_tool gen_blocks %s %d %x %x %x %x" % (payload_blob_path, blocks, rng_state_a, rng_state_b, rng_state_c, rng_state_d)

    #print(cmd)

    proc = subprocess.Popen(cmd.split(" "), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, cwd=(os.getcwd()+"/os/tools/"))
    stdout, _ = proc.communicate()

    rng_state_a = None
    rng_state_b = None
    rng_state_c = None
    rng_state_d = None

    for line in stdout.decode('utf-8').splitlines():
        #print(line)
        if "[*] rng_state->a: " in line:
            rng_state_a = int(line.split("[*] rng_state->a: ")[1], 16)
        if "[*] rng_state->b: " in line:
            rng_state_b = int(line.split("[*] rng_state->b: ")[1], 16)
        if "[*] rng_state->c: " in line:
            rng_state_c = int(line.split("[*] rng_state->c: ")[1], 16)
        if "[*] rng_state->d: " in line:
            rng_state_d = int(line.split("[*] rng_state->d: ")[1], 16)

    #print("rng_state_a: %x" % rng_state_a)
    #print("rng_state_b: %x" % rng_state_b)
    #print("rng_state_c: %x" % rng_state_c)
    #print("rng_state_d: %x" % rng_state_d)

    if rng_state_a == None or rng_state_b == None or rng_state_c == None or rng_state_d == None:
        raise Exception("Could not generate payload")

    if "[!] Generated " in stdout.decode('utf-8'):
        return (rng_state_a, rng_state_b, rng_state_c, rng_state_d)
    
    raise Exception("Could not generate payload")

def get_rng_state(seed):
    cmd = "./tesseract_tool seed2state %s" % (seed)
    proc = subprocess.Popen(cmd.split(" "), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, cwd=(os.getcwd()+"/os/tools/"))
    stdout, _ = proc.communicate()

    rng_state_a = None
    rng_state_b = None
    rng_state_c = None
    rng_state_d = None

    for line in stdout.decode('utf-8').splitlines():
        if "[*] rng_state->a: " in line:
            rng_state_a = int(line.split("[*] rng_state->a: ")[1],16)
        if "[*] rng_state->b: " in line:
            rng_state_b = int(line.split("[*] rng_state->b: ")[1],16)
        if "[*] rng_state->c: " in line:
            rng_state_c = int(line.split("[*] rng_state->c: ")[1],16)
        if "[*] rng_state->d: " in line:
            rng_state_d = int(line.split("[*] rng_state->d: ")[1],16)

    if rng_state_a == None or rng_state_b == None or rng_state_c == None or rng_state_d == None:
        raise Exception("Could not translate seed to rng state")

    return (rng_state_a, rng_state_b, rng_state_c, rng_state_d)

def generate_payload_by_seed(payload_blob_path, seed, blocks):
    rng_state_a, rng_state_b, rng_state_c, rng_state_d = get_rng_state(seed)
    return generate_payload(payload_blob_path, blocks, rng_state_a, rng_state_b, rng_state_c, rng_state_d)

def find_minimal_reproducable_payload(runner, payload_blob_path, image_path, configuration, expected_exit_type=None, expected_exit_reason_str="", tmp_file_path="/tmp/hypercube_tmp_payload.bin", memlimit=None, extra_file_path=None):

    print("[*] trying to find minimal reproducable payload size...")

    image_tmp_path = tmp_file_path+".iso"
    file_size = os.path.getsize(payload_blob_path)
    assert(file_size % TOTAL_BLOCK_SIZE == 0 and file_size%2 == 0)

    raw_data = open(payload_blob_path, "rb").read()

    while len(raw_data) > TOTAL_BLOCK_SIZE:

        new_size = int(len(raw_data)/2)
        raw_data = raw_data[:new_size]
        f = open(tmp_file_path, "wb")
        f.write(raw_data)
        f.close()

        # huge payload files tend to take longer to execute
        if os.path.getsize(tmp_file_path) >= (16>>20):
            timeout = 30.0
        else:
            timeout = 10.0

        build_hypercube_payload_image(configuration, tmp_file_path, path=image_tmp_path)

        if extra_file_path:
            shutil.copy(tmp_file_path+".extra", extra_file_path)

        stdout_output, stderr_output, exit_reason_type, exit_reason_str, seed, run_time = runner.run_target(image_tmp_path, timeout_sec=timeout, memlimit=memlimit)
        
        if exit_reason_type == expected_exit_type and __remove_hexadecimal_addresses(exit_reason_str) == expected_exit_reason_str:
            print("\t[!] new size is %d bytes" % os.path.getsize(tmp_file_path))
            shutil.copy(tmp_file_path, payload_blob_path)
            shutil.copy(image_tmp_path, image_path)
        else:
            break

    print("\t[*] found minimal reproducer payload size: %d bytes" % os.path.getsize(payload_blob_path))


# Let's first try to find a payload that reproduces the crash
def generate_reproducable_payload(runner, payload_blob_path, image_path, seed, configuration, configured_areas, expected_exit_type=None, expected_exit_reason_str="", tmp_file_path="/tmp/hypercube_tmp_payload.bin", memlimit=None, extra_file_path=None):

    print("[*] trying to find a crash reproducer payload...")

    image_tmp_path = tmp_file_path+".iso"
    rng_state_a, rng_state_b, rng_state_c, rng_state_d = get_rng_state(seed)

    for size in reversed([2**i for i in range(15)]):
        rng_state_a, rng_state_b, rng_state_c, rng_state_d = get_rng_state(seed)
        for j in range(10):
            rng_state_a, rng_state_b, rng_state_c, rng_state_d = generate_payload(tmp_file_path, size, rng_state_a, rng_state_b, rng_state_c, rng_state_d)

            print("\t[!] trying to reproduce crash with %d blocks (%d bytes; offset #%d)" % (size, size*(1024+32), j), end=" => ")

            # huge payload files tend to take longer to execute
            if os.path.getsize(tmp_file_path) >= (16>>20):
                timeout = 30.0
            else:
                timeout = 10.0

            build_hypercube_payload_image(configuration, tmp_file_path, path=image_tmp_path)

            if extra_file_path:
                shutil.copy(tmp_file_path + ".extra", extra_file_path)

            stdout_output, stderr_output, exit_reason_type, exit_reason_str, seed, run_time = runner.run_target(image_tmp_path, timeout_sec=timeout, memlimit=memlimit)

            if exit_reason_type == expected_exit_type and __remove_hexadecimal_addresses(exit_reason_str) == expected_exit_reason_str:
                print("success!")
                shutil.copy(tmp_file_path, payload_blob_path)
                shutil.copy(image_tmp_path, image_path)
                find_minimal_reproducable_payload(runner, payload_blob_path, image_path, configuration, expected_exit_type, expected_exit_reason_str, tmp_file_path=tmp_file_path, memlimit=memlimit, extra_file_path=extra_file_path)
                return
            
            else:
                print("fail!")
                #shutil.copy(image_tmp_path, "/tmp/%d.iso"%j)

    print("[!] could not find a crash reproducer payload")
    __remove_tmp_files(tmp_file_path)
    sys.exit(0)

def handle_minimize(parser, args):

    print('[*] running input minimization...')

    temp_file_path = "/tmp/hypercube_payload_%s.bin" % uuid.uuid4()

    print("[*] tmp file path: \"%s\"" % temp_file_path)

    e_filter = None
    if args.e_filter is not None:
        print('[*] using error filter: %s' % args.e_filter)
        e_filter = args.e_filter

    e_type = None
    if args.e_type is not None:
        print('[*] using error type: %s' % args.e_type)
        try:
            e_type = ExitReason[args.e_type]
        except KeyError:
            print('[!] Error: invalid error type: %s' % args.e_type)
            print('[*] Valid error types are:')
            for k in ExitReason:
                print("\t" + str(k).split(".")[1])
            sys.exit(0)

    if args.seed is None and args.input is None:
        print('Error: either --seed or --input argument is required.')
        sys.exit(0)

    if args.seed is not None and args.input is not None:
        print('Error: only one of --seed or --input argument is allowed.')
        sys.exit(0)


    extra_file_path = None
    if args.extra_file_path is not None:

        if args.empty_extra_file_size is not None and args.copy_extra_file_path is not None:
            print('[!] Error: either --empty_extra_file_size or --copy_extra_file_path can be specified')
            sys.exit(0)

        if args.empty_extra_file_size is not None:
            path = temp_file_path + ".extra"
            if os.path.exists(path):
                os.remove(path)

            with open(path, "w") as file:
                file.truncate(args.empty_extra_file_size * 1024)

        if args.copy_extra_file_path is not None:
            shutil.copy(args.copy_extra_file_path, temp_file_path + ".extra")

        extra_file_path = args.extra_file_path

    target_path = ""
    if glob.glob(TARGET_DEST + "/%s/x86_64-softmmu/qemu-system-x86_64" % args.target):
        target_path = glob.glob(TARGET_DEST + "/%s/x86_64-softmmu/qemu-system-x86_64" % args.target)[0]
    elif glob.glob(TARGET_DEST + "/%s/build/x86_64-softmmu/qemu-system-x86_64" % args.target):
        target_path = glob.glob(TARGET_DEST + "/%s/build/x86_64-softmmu/qemu-system-x86_64" % args.target)[0]
    else:
        raise Exception("Target (%s) not found" % args.target)

    target_path = os.path.realpath(target_path)
    image_path = os.path.realpath(args.image)
    payload_path = os.path.realpath(args.output)


    # at this point we do different things depending on whether we have a seed or an input file
    if args.seed is not None:
        configuration = generate_config_header_file( \
            debug=False, \
            enable_io=args.enable_io, \
            enable_pci=args.enable_pci, \
            enable_e_pci=args.enable_e_pci, \
            enable_apic=args.enable_apic, \
            enable_hpet=args.enable_hpet, \
            enable_default_io=args.enable_default_io, \
            target_filters=args.filter, \
            seed=args.seed)
        
        #print(configuration)

        build_hypercube_image(configuration, path=image_path)
        #sys.exit(0)
    else: 
        # payload file provided
        configuration = generate_config_header_file( \
            debug=False, \
            enable_io=args.enable_io, \
            enable_pci=args.enable_pci, \
            enable_e_pci=args.enable_e_pci, \
            enable_apic=args.enable_apic, \
            enable_hpet=args.enable_hpet, \
            enable_default_io=args.enable_default_io, \
            target_filters=args.filter)
        build_hypercube_payload_image(configuration, args.input, path=image_path)

    extra_params = args.qemu_params

    memlimit = None
    if args.memlimit is not None:
        memlimit = args.memlimit * 1024 * 1024

    success = False
    print_once = False

    runner = QEMUTargetExecutor(target_path, extra_params)

    print("[*] trying to reproduce error...")

    for i in range(10): 

        print("\t[*] running target (%d/10)..."%(i+1))

        if extra_file_path:
            shutil.copy(temp_file_path+".extra", extra_file_path)

        # perform dry run to check if the seed results in a crash
        stdout_output, stderr_output, exit_reason_type, exit_reason_str, seed, run_time = runner.run_target(image_path, timeout_sec=10.0, memlimit=memlimit)

        if exit_reason_type == ExitReason.NO_FUZZING_REGIONS:
            print('\t[!] Error: no fuzzing regions found.')
            sys.exit(1)

        if args.seed is not None:
            if str(int(seed, 16)) != str(int(args.seed, 16)):
                print('\t[!] Error: configured seed does not match runtime seed (args.seed: %s, runtime seed: %s, ).' % (args.seed, seed))
                sys.exit(0)
        else:
            if print_once == False:
                print("\t[!] ignoring seed: %s" % seed)
                print_once = True

        expected_exit_type = exit_reason_type
        expected_exit_reason_str = __remove_hexadecimal_addresses(exit_reason_str)

        if e_type is None and e_filter is None:
            success = True
            break

        if e_type is not None and e_type != expected_exit_type:
            print('\t[?] Warning: seed or input does not trigger a crash of the expected type (expected: %s, actual: %s).' % (e_type, expected_exit_type))
            continue

        if e_filter is not None and e_filter not in expected_exit_reason_str:
            print('\t[?] Warning: seed or input does not trigger a crash of the expected type (expected: %s, actual: %s).' % (e_filter, expected_exit_reason_str))
            continue

        if exit_reason_type is None or exit_reason_type == ExitReason.TIMEOUT or exit_reason_type == ExitReason.NO_FUZZING_REGIONS or exit_reason_type == ExitReason.DEADLOCK:
            print('\t[!] Error: seed does not trigger a crash (exit_type: %s).' % exit_reason_type)
        else:
            success = True
            break

    if success == False:
        print('\t[!] Error: could not find a crash reproducer payload')
        sys.exit(0)

    print("[*] reproducer found!")

    if extra_file_path:
        shutil.copy(temp_file_path+".extra", extra_file_path)

    # perform dry run to output the configuration
    success, available_areas, configured_areas, stdout_output, stderr_output = runner.dry_run_target(image_path)
        
    if not success:
        print("Error: Dry run failed")
        if len(stderr_output) == 0:
            print(stdout_output)
        else:
            print(stderr_output)
        sys.exit()

    print("[*] Available areas:")
    for area in available_areas:
        print("\t -> %s" % area)
    
    if len(configured_areas) == 0:
        print("[!] Error: No configured areas")
        sys.exit()

    print("[*] Configured areas:")
    for area in configured_areas:
        print("\t -> %s" % area)

    print("[*] exit_reason_type: \"%s\"" % exit_reason_type)
    print("[*] exit_reason_str:  \"%s\"" % exit_reason_str)

    try:
        if args.seed is not None:
            # find minimal number of blocks to trigger the crash:
            generate_reproducable_payload(runner, payload_path, image_path, seed, configuration, configured_areas, expected_exit_type=expected_exit_type, expected_exit_reason_str=expected_exit_reason_str, tmp_file_path=temp_file_path, memlimit=memlimit, extra_file_path=extra_file_path)
        else:
            try:
                shutil.copy(args.input, payload_path)
            except shutil.SameFileError:
                print("[!] Warning: input file and payload file are the same")

        run_minization_step(runner, payload_path, image_path, configuration, expected_exit_type=expected_exit_type, expected_exit_reason_str=expected_exit_reason_str, number_of_attempts=20, tmp_file_path=temp_file_path, memlimit=memlimit, extra_file_path=extra_file_path)
        
        # optional: run decompiler if number of opcodes is below 100
        if get_number_of_opcodes(payload_path) < 100:
            run_decompiler(payload_path, configured_areas)

        print("[+] Done!")
        print("\t[+] Payload: %s" % payload_path)
        print("\t[+] Image: %s" % image_path)

        print("\t[+] Run the following command to reproduce the crash:")
        
        if extra_file_path:
            copy_cmd = "cp %s %s" % (payload_path, "/tmp/data.file")
            print("\t%s; %s\n" % (copy_cmd, runner.get_target_cmd_line(image_path, extra_params.replace(extra_file_path, "/tmp/data.file"))))
        else:
            print("\t%s\n" % runner.get_target_cmd_line(image_path))


    except KeyboardInterrupt:
        print("[-] Bye!")
        __remove_tmp_files(temp_file_path)
        sys.exit(0)

    __remove_tmp_files(temp_file_path)
    sys.exit(0)
