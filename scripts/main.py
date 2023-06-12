import sys

from scripts.arg_parser import parse_args
from scripts.compile import generate_config_header_file, build_hypercube_image, build_hypercube_payload_image


from scripts.handler.run import handle_run
from scripts.handler.coverage import handle_coverage
from scripts.handler.minimize import handle_minimize
from scripts.handler.decompiler import handle_decompiler

def handle_generate(parser, args):

    if args.input and args.seed:
        print("[!] Error: Cannot specify both input and seed")
        sys.exit(1)

    configuration = generate_config_header_file( \
        debug=args.debug, \
        enable_io=args.enable_io, \
        enable_default_io=args.enable_default_io, \
        enable_pci=args.enable_pci, \
        enable_e_pci=args.enable_e_pci, \
        enable_apic=args.enable_apic, \
        enable_hpet=args.enable_hpet, \
        enable_auto_reboot=args.enable_auto_reboot, \
        seed=args.seed, \
        extra_areas=args.extra_areas, \
        io_brute_force_range=args.set_io_bruteforce_range, \
        target_filters=args.filter, \
        disable_fuzzer=args.disable_fuzzer, \
        enable_vga=args.enable_vga)
    

    print("[*] Generated configuration:")
    print("--------------------------------------------------")
    print(configuration, end="")
    print("--------------------------------------------------")
    
    if not args.input:
        build_hypercube_image(configuration, path=args.image_dest, uefi=args.uefi)
    else:
        build_hypercube_payload_image(configuration, args.input, path=args.image_dest, uefi=args.uefi)
    sys.exit(0)

# main
def main():
    
    parser, args = parse_args()

    if '__parser_type__' in vars(args):
        if args.__parser_type__ == 'generate':
            handle_generate(parser, args)
        elif args.__parser_type__ == 'minimize':
            handle_minimize(parser, args)
        elif args.__parser_type__ == 'coverage':
            handle_coverage(parser, args)
        elif args.__parser_type__ == 'run':
            handle_run(parser, args)
        elif args.__parser_type__ == 'decompiler':
            handle_decompiler(parser, args)
        else:
            parser.print_help()
            sys.exit(0)
    else:
        parser.print_help()
