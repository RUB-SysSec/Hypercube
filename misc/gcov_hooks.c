#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <dlfcn.h>

static void gcov_handler(int sig) {
    printf("gcov handler called (sig: %d).\n", sig);
    fflush(stdout);
    fprintf(stderr, "gcov handler called (sig: %d).\n", sig);
    fflush(stderr);
    exit(0); /* flush gcocv data */
}

void gcov_init() {
    signal(SIGSEGV, gcov_handler);
    signal(SIGILL, gcov_handler);
    signal(SIGSTOP, gcov_handler);
    signal(SIGABRT, gcov_handler);
    printf("gcov signal handler installed.\n");
    fprintf(stderr, "gcov signal handler installed.\n");
}

int __libc_start_main(
    int (*main)(int, char **, char **),
    int argc,
    char **argv,
    int (*init)(int, char **, char **),
    void (*fini)(void),
    void (*rtld_fini)(void),
    void *stack_end)
{
    typeof(&__libc_start_main) orig = dlsym(RTLD_NEXT, "__libc_start_main");

    gcov_init();
    return orig(main, argc, argv, init, fini, rtld_fini, stack_end);
}
