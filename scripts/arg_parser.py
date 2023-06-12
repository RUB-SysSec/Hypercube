import sys
import argparse
import glob
from scripts.config import TARGET_DEST

class Argextrar(argparse.ArgumentParser):

    def show_banner(self):
        print('''- Hypercube ''')

    def error(self, message):
        self.show_banner()
        self.print_help()
        print('\033[91m[Error] %s\n\n\033[0m\n' % message)
        
        sys.exit(1)

def __add_extra_areas_options(subparser):
    subparser.add_argument('--extra_areas', action='append',nargs=4, metavar=('<base>','<length>', '<type>', '<name>') , help='add extra areas (base & length in hex, type: 0=PIO, 1=MMIO)')

def __add_build_options(subparser):
    subparser.add_argument('--enable_io', action='store_true', default=False, help='enable IO enumeration')
    subparser.add_argument('--enable_default_io', action='store_true', default=False, help='enable default IO enumeration')
    subparser.add_argument('--enable_pci', action='store_true', default=False, help='enable PCI enumeration')
    subparser.add_argument('--enable_e_pci', action='store_true', default=False, help='enable extended PCI')
    subparser.add_argument('--enable_apic', action='store_true', default=False, help='enable APIC')
    subparser.add_argument('--enable_hpet', action='store_true', default=False, help='enable HPET')
    __add_extra_areas_options(subparser)

def __add_filters_options(subparser):
    subparser.add_argument('--filter', action='append', metavar=('<Filter>') , help='set target filter')

def __add_special_run_options(subparser):
    subparser.add_argument('--extra_file_path', default=None, type=str, metavar="<path>", help='specify extra file path')
    subparser.add_argument('--empty_extra_file_size', type=int, default=None, metavar="<size>", help='specifiy size of empty extra file (in KB)')
    subparser.add_argument('--copy_extra_file_path', default=None, metavar="<path>", type=str, help='copy existing extra file')

    #subparser.add_argument('--extra_file_size', type=int, default=None, help='set size of extra file')
    subparser.add_argument('--extra_file_keep', action='store_true', default=False, help='do not recreate extra file after each execution')
    subparser.add_argument('--store_extra_file', action='store_true', default=False, help='store extra file in case of abnormal exit reasons')

def __add_run_options(subparser):
    subparser.add_argument('--qemu_params', default=None, metavar="<VAR>", type=str, help='add extra parameters to QEMU command line')
    subparser.add_argument('--memlimit', default=None, type=int, metavar="<mlimit>", help='set memlimit (in MB; does not work well with ASAN builds)')

def __add_special_minimize_options(subparser):
    subparser.add_argument('--extra_file_path', default=None, metavar="<path>", type=str, help='specify extra file path')
    subparser.add_argument('--empty_extra_file_size', type=int, default=None, metavar="<size>", help='specifiy size of empty extra file (in KB)')
    subparser.add_argument('--copy_extra_file_path', default=None, metavar="<path>", type=str, help='copy existing extra file')

def parse_args_generate(subparser):

    subparser.add_argument('-__parser_type__', type=str, default="generate", help=argparse.SUPPRESS)

    subparser.add_argument('image_dest', default=None, type=str, metavar="<image_dest>", help='specifiy Hypercube image destination')

    parser = subparser.add_argument_group("Build Options")

    __add_build_options(parser)
    __add_filters_options(parser)

    parser.add_argument('--seed', default=None, type=str, metavar="<Seed>", help='set seed')
    parser.add_argument('--input', default=None, type=str, metavar="<input>", help='path to input file')

    parser.add_argument('--set_io_bruteforce_range', type=int, default=None, help='set custom IO bruteforce range (default: 0xf)')

    parser.add_argument('--enable_auto_reboot', action='store_true', default=False, help='enable auto reboot after panic')

    parser.add_argument('--debug', action='store_true', default=False, help='enable debug mode (verbose output)')
    parser.add_argument('--disable_fuzzer', action='store_true', default=False, help='disable fuzzer (useful to display all enumerated devices)')
    parser.add_argument('--enable_vga', action='store_true', default=False, help='enable VGA output')
    parser.add_argument('--uefi', action='store_true', default=False, help='generate UEFI image (i386)')

    # add option to set up to 4 manual IO / MMIO regions


def parse_args_minimize(subparser):

    subparser.add_argument('-__parser_type__', type=str, default="minimize", help=argparse.SUPPRESS)

    # find available targets
    target_paths = []
    target_paths = target_paths + glob.glob(TARGET_DEST + "/*/x86_64-softmmu/qemu-system-x86_64")
    target_paths = target_paths + glob.glob(TARGET_DEST + "/*/build/x86_64-softmmu/qemu-system-x86_64")

    # filter out non gcov postfixed target_paths
    target_paths = [path for path in target_paths if not path.split(TARGET_DEST)[1].split("/")[0].endswith("-gcov")]

    targets = []
    for path in target_paths:
        targets.append(path.split(TARGET_DEST)[1].split("/")[0])

    target_help = ""
    for target in targets:
        target_help += "%s\n" % target

    subparser.add_argument('target', metavar='<Target>', choices=targets, help=target_help)
    subparser.add_argument('image', default=None, type=str, metavar="<image_path>", help='path to output Hypercube image')
    subparser.add_argument('output', default=None, type=str, metavar="<output_path>", help='path to output file')

    parser = subparser.add_argument_group("Minimizer Options")

    parser.add_argument('--seed', default=None, type=str, metavar="<Seed>", help='set seed')
    parser.add_argument('--input', default=None, type=str, metavar="<input>", help='path to input file')
    parser.add_argument('--e_filter', default=None, type=str, metavar="<reason>", help='filter for exit reason')
    parser.add_argument('--e_type', default=None, type=str, metavar="<type>", help='filter for exit type')

    __add_special_minimize_options(parser)

    parser = subparser.add_argument_group("Build Options")

    __add_build_options(parser)
    __add_filters_options(parser)

    exec_parser = subparser.add_argument_group("Execution Options")
    __add_run_options(exec_parser)

def parse_args_decompiler(subparser):

    subparser.add_argument('-__parser_type__', type=str, default="decompiler", help=argparse.SUPPRESS)

    # find available targets
    target_paths = []
    target_paths = target_paths + glob.glob(TARGET_DEST + "/*/x86_64-softmmu/qemu-system-x86_64")
    target_paths = target_paths + glob.glob(TARGET_DEST + "/*/build/x86_64-softmmu/qemu-system-x86_64")

    # filter out non gcov postfixed target_paths
    target_paths = [path for path in target_paths if not path.split(TARGET_DEST)[1].split("/")[0].endswith("-gcov")]

    targets = []
    for path in target_paths:
        targets.append(path.split(TARGET_DEST)[1].split("/")[0])

    target_help = ""
    for target in targets:
        target_help += "%s\n" % target

    subparser.add_argument('target', metavar='<Target>', choices=targets, help=target_help)
    subparser.add_argument('payload', default=None, type=str, metavar="<payload_path>", help='path to payload file')

    __add_build_options(subparser)
    __add_filters_options(subparser)

    exec_parser = subparser.add_argument_group("Execution Options")
    __add_run_options(exec_parser)


def parse_args_coverage(subparser):

    subparser.add_argument('-__parser_type__', type=str, default="coverage", help=argparse.SUPPRESS)

    target = ["AC97", "CS4231a", "ES1370", "Intel-HDA", "SoundBlaster", "Floppy", "Parallel", "Serial", "IDE-Core", "EEPro100", "E1000", "NE2000-PCI", "PCNET-PCI", "RTL8139", "SDHCI"]
    target_help =    'AC97\n' \
                    'CS4231a\n' \
                    'ES1370\n' \
                    'Intel-HDA\n' \
                    'SoundBlaster\n' \
                    'Floppy\n' \
                    'Parallel\n' \
                    'Serial\n' \
                    'IDE-Core\n' \
                    'EEPro100\n' \
                    'E1000\n' \
                    'NE2000-PCI\n' \
                    'PCNET-PCI\n' \
                    'RTL8139\n' \
                    'SDHCI\n' 

    subparser.add_argument('target', metavar='<Target>', choices=target, help=target_help)

    # find available targets
    target_paths = []
    target_paths = target_paths + glob.glob("./targets/*/x86_64-softmmu/qemu-system-x86_64")
    target_paths = target_paths + glob.glob("./targets/*/build/x86_64-softmmu/qemu-system-x86_64")

    # filter out non gcov postfixed target_paths
    target_paths = [path for path in target_paths if path.split("./targets/")[1].split("/")[0].endswith("-gcov")]

    targets = []
    for path in target_paths:
        targets.append(path.split("./targets/")[1].split("/")[0])

    target_help = ""
    for target in targets:
        target_help += "%s\n" % target

    subparser.add_argument('qemu', metavar='<QEMU>', choices=targets, help=target_help)

    #subparser.add_argument('--aaz', choices='XYZ', help='baz is another option')
    subparser.add_argument('--run_time', default=600.0, type=float, help='eval run time in seconds (default: 600.0 or 10mins)')
    subparser.add_argument('--timeout', default=10.0, type=float, help='periodic timeout value (default: 10.0 seconds)')

# TODO: debug -> delete me later
def parse_args_run(subparser):

    subparser.add_argument('-__parser_type__', type=str, default="run", help=argparse.SUPPRESS)

    # find available targets
    target_paths = []
    target_paths = target_paths + glob.glob(TARGET_DEST + "/*/x86_64-softmmu/qemu-system-x86_64")
    target_paths = target_paths + glob.glob(TARGET_DEST + "/*/build/x86_64-softmmu/qemu-system-x86_64")

    # filter gcov postfixed target_paths
    target_paths = [path for path in target_paths if not path.split(TARGET_DEST)[1].split("/")[0].endswith("-gcov")]

    targets = []
    for path in target_paths:
        targets.append(path.split(TARGET_DEST)[1].split("/")[0])

    target_help = ""
    for target in targets:
        target_help += "%s\n" % target

    subparser.add_argument('target', metavar='<Target>', choices=targets, help=target_help)
    subparser.add_argument('image', default=None, type=str, metavar="<image_path>", help='specifiy Hypercube image')

    exec_parser = subparser.add_argument_group("Run Options")
    exec_parser.add_argument('--disable_dry_run', action='store_true', default=False, help='disable dry run to output registered fuzzing regions')
    __add_special_run_options(exec_parser)


    exec_parser = subparser.add_argument_group("Execution Options")
    __add_run_options(exec_parser)
    exec_parser.add_argument('--timeout', default=10.0, type=float, help='timeout value (default: 10.0 seconds)')


def parse_args():

    parser = Argextrar(formatter_class=argparse.RawTextHelpFormatter)

    sub_parsers = parser.add_subparsers(help='sub-command help')

    subparser_generate = sub_parsers.add_parser('generate', help='generate new Hypercube OS image')
    parse_args_generate(subparser_generate)

    subparser_minimize = sub_parsers.add_parser('minimize', help='testcase minimizer')
    parse_args_minimize(subparser_minimize)

    subparser_coverage = sub_parsers.add_parser('coverage', help='run QEMU coverage evaluation')
    parse_args_coverage(subparser_coverage)

    subparser_decompiler = sub_parsers.add_parser('decompiler', help='decompile tesseract payload file')
    parse_args_decompiler(subparser_decompiler)

    subparser_run = sub_parsers.add_parser('run', help='run Hypercube OS images')
    parse_args_run(subparser_run)
    args = parser.parse_args()

    return parser, args
