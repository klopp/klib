#if defined(__KERNEL__)

#include <linux/module.h>
#include <linux/slab.h>

void * _k_calloc(int size, int n) {
    void * data = kmalloc(size * n, GFP_KERNEL);
    if (data) {
        memset(data, 0, size * n);
    }
    return data;
}

char * _k_strdup(char * s) {
    int size = strlen(s);
    char * data = kmalloc(size + 1, GFP_KERNEL);
    if (data) {
        memcpy(data, s, size + 1);
    }
    return data;
}

#endif
