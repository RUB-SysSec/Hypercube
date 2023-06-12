import glob, os, sys, subprocess, uuid
from scripts.compile import generate_config_header_file, build_hypercube_payload_image
from scripts.executor.general import ExitReason
from scripts.executor.qemu import QEMUTargetExecutor
from scripts.config import TARGET_DEST

def run_decompiler(payload_blob_path, areas):

    cmd_areas = ""
    for area in areas:
        print(area)
        if area.split(" ")[2] == "MMIO":
            mmio_area = True
        elif area.split(" ")[2] == "PIO":
            mmio_area = False
        else:
            raise Exception("Unknown area type")

        area_base = int(area.split(" ")[0].split("base:")[1],16)
        area_size = int(area.split(" ")[1].split("size:")[1],16)

        cmd_areas += " %d %x %x" % (mmio_area, area_base, area_size)

    #print(cmd_areas)
    cmd = "./tesseract_tool decompile %s%s" % (payload_blob_path, cmd_areas)
    #print(cmd)
    proc = subprocess.Popen(cmd.split(" "), stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, cwd=(os.getcwd()+"/os/tools/"))
    stdout, _ = proc.communicate()

    print(stdout.decode('utf-8').split("===========================================")[1])

def handle_decompiler(parser, args):

    print('[*] running input decompiler...')

    target_path = ""
    if glob.glob(TARGET_DEST + "/%s/x86_64-softmmu/qemu-system-x86_64" % args.target):
        target_path = glob.glob(TARGET_DEST + "/%s/x86_64-softmmu/qemu-system-x86_64" % args.target)[0]
    elif glob.glob(TARGET_DEST + "/%s/build/x86_64-softmmu/qemu-system-x86_64" % args.target):
        target_path = glob.glob(TARGET_DEST + "/%s/build/x86_64-softmmu/qemu-system-x86_64" % args.target)[0]
    else:
        raise Exception("Target (%s) not found" % args.target)

    temp_file_path = "/tmp/hypercube_payload_%s.bin" % uuid.uuid4()

    print("[*] tmp file path: \"%s\"" % temp_file_path)

    target_path = os.path.realpath(target_path)
    payload_path = os.path.realpath(args.payload)

    configuration = generate_config_header_file( \
            debug=False, \
            enable_io=args.enable_io, \
            enable_pci=args.enable_pci, \
            enable_e_pci=args.enable_e_pci, \
            enable_apic=args.enable_apic, \
            enable_hpet=args.enable_hpet, \
            enable_default_io=args.enable_default_io, \
            target_filters=args.filter)
    
    build_hypercube_payload_image(configuration, payload_path, path=temp_file_path)

    extra_params = args.qemu_params
    runner = QEMUTargetExecutor(target_path, extra_params)

    # perform dry run to output the configuration
    success, available_areas, configured_areas, stdout_output, stderr_output = runner.dry_run_target(temp_file_path)
        
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

    run_decompiler(payload_path, configured_areas)
    os.remove(temp_file_path)
    print("[-] Bye!")
    
    sys.exit(0)
