import sys
import glob
import os

from scripts.executor.general import ExitReason
from scripts.executor.qemu import QEMUTargetExecutor

from scripts.config import TARGET_DEST
import shutil

def run_thread_pool(target_path, image_path, timeout=5):
    pass

EXTRA_FILE_TMP_PATH = "/tmp/.hypercube_extra_file.tmp"

def __is_exit_reason_interesting(exit_reason_type):
    if exit_reason_type == ExitReason.SIGSEGV or \
        exit_reason_type == ExitReason.SIGBUS or \
        exit_reason_type == ExitReason.SIGILL or \
        exit_reason_type == ExitReason.SIGFPE or \
        exit_reason_type == ExitReason.ASAN or \
        exit_reason_type == ExitReason.ABORT or \
        exit_reason_type == ExitReason.DEADLOCK or \
        exit_reason_type == ExitReason.SIGKILL:
        return True
    return False

def handle_run(parser, args):

    extra_file_path = None
    if args.extra_file_path is not None:

        if args.empty_extra_file_size is not None and args.copy_extra_file_path is not None:
            print('[!] Error: either --empty_extra_file_size or --copy_extra_file_path can be specified')
            sys.exit(0)

        if args.empty_extra_file_size is not None:
            path = EXTRA_FILE_TMP_PATH
            if os.path.exists(path):
                os.remove(path)

            with open(path, "w") as file:
                file.truncate(args.empty_extra_file_size * 1024)

        if args.copy_extra_file_path is not None:
            shutil.copy(args.copy_extra_file_path, EXTRA_FILE_TMP_PATH)

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

    extra_params = args.qemu_params
    runner = QEMUTargetExecutor(target_path, extra_params)

    if not args.disable_dry_run:

        if extra_file_path:
            if os.path.exists(extra_file_path):
                os.remove(extra_file_path)
            shutil.copy(EXTRA_FILE_TMP_PATH, extra_file_path)

        success, available_areas, configured_areas, stdout_output, stderr_output = runner.dry_run_target(image_path)
        
        if not success:
            print("Error: Dry run failed")
            if len(stderr_output) == 0:
                print(stdout_output.decode('utf-8'))
            else:
                print(stderr_output.decode('utf-8'))
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

    memlimit = None
    if args.memlimit is not None:
        memlimit = args.memlimit * 1024 * 1024

    print("[*] Running target:")
    run_id = 0
    while True:

        if extra_file_path:
            if os.path.exists(extra_file_path):
                os.remove(extra_file_path)
            shutil.copy(EXTRA_FILE_TMP_PATH, extra_file_path)

        #if args.extra_file_path is not None and args.extra_file_size is not None:
        #    __create_extra_file(args.extra_file_path, args.extra_file_size, keep=args.extra_file_keep)

        stdout_output, stderr_output, exit_reason_type, exit_reason_str, seed, run_time = runner.run_target(image_path, timeout_sec=args.timeout, memlimit=memlimit)
        print("[%d]\tseed: %s\t %f\t exit_reason: %s - %s" % (run_id, seed, run_time, exit_reason_type, exit_reason_str))
        #print(stdout_output)
        #print(stderr_output)
        run_id += 1

        if __is_exit_reason_interesting(exit_reason_type):
            if extra_file_path and args.store_extra_file: # args.extra_file_path is not None and args.extra_file_size is not None and args.store_extra_file:
                saved_extra_file_location = "/tmp/hypercube_extra_%s" % (seed)
                print("[*] Storing extra file: %s" % saved_extra_file_location)
                os.rename(args.extra_file_path, saved_extra_file_location)

    sys.exit(0)
