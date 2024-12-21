#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include <signal.h>

static void* (*real_malloc)(size_t) = NULL;

void* _malloc(size_t size) {
    if (!real_malloc) {
        real_malloc = (void* (*)(size_t))dlsym(RTLD_NEXT, "malloc");
        if (!real_malloc) {
            fprintf(stderr, "Error in dlsym: %s\n", dlerror());
            exit(EXIT_FAILURE);
        }
    }

    void* ptr = real_malloc(size);
    return ptr;
}