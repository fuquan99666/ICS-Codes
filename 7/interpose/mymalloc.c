/* 
 * mymalloc.c - Examples of run-time, link-time, and compile-time
 *              library interpositioning.
 */

/*
 * Run-time interpositioning of malloc and free based 
 * on the dynamic linker's (ld-linux.so) LD_PRELOAD mechanism
 * 
 * Example (Assume a.out calls malloc and free):
 *   linux> gcc -Wall -DRUNTIME -shared -fpic -o mymalloc.so mymalloc.c -ldl
 *
 *   bash> (LD_PRELOAD="./mymalloc.so" ./a.out)	
 *   ...or 
 *   tcsh> (setenv LD_PRELOAD "./mymalloc.so"; ./a.out; unsetenv LD_PRELOAD)
 */
/* $begin interposer */
#ifdef RUNTIME
#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>

// Using frpintf(stderr, ...) instead of printf(...) to avoid recursion problem (printf uses malloc) !
// Below is the example from jyy , but in my archlinux desktop(maybe different glibc implementation?), it failed because of the recursion problem (free and dlsym)
//void *malloc(size_t size) {
    //static void *(*real_malloc)(size_t) = NULL;
		//if (!real_malloc) {
			//real_malloc = dlsym(RTLD_NEXT, "malloc");
		//}

    //void *ptr = real_malloc(size);
    //fprintf(stderr, "malloc(%zu) = %p\n", size, ptr);
    
    //return ptr;
//}

//void free(void *ptr) {
    //if (!ptr) return;
    //static void (*real_free)(void*) = NULL;
		//if (!real_free) {
			//real_free = dlsym(RTLD_NEXT, "free");
		//}
    
    //fprintf(stderr, "free(%p)\n", ptr);
    //real_free(ptr);
//}

// A safer version by using constructor to get real malloc and free
static void *(*real_malloc)(size_t) = NULL;
static void (*real_free)(void *) = NULL;

// 2. 初始化函数：在动态库被加载时，立刻执行一次
__attribute__((constructor))
static void init_mymalloc(void) {
    // 此时程序刚启动，不会使用你的 malloc，所以 dlsym 是绝对安全的
    real_malloc = dlsym(RTLD_NEXT, "malloc");
    real_free = dlsym(RTLD_NEXT, "free");
}

void *malloc(size_t size) {
    // 3. 兜底：极少数情况下初始化函数没执行完就被调用了
    if (!real_malloc) {
        return NULL; 
    }
    
    void *ptr = real_malloc(size);
    
    fprintf(stderr, "malloc(%zu) = %p\n", size, ptr);
    return ptr;
}

void free(void *ptr) {
    if (!ptr) return;
    if (!real_free) return; // 兜底保护
    
    real_free(ptr);
    
    fprintf(stderr, "free(%p)\n", ptr);
}
#endif
/* $end interposer */

/* 
 * Link-time interposition of malloc and free using the static
 * linker's (ld) "--wrap symbol" flag.
 * 
 * Compile the executable using "-Wl,--wrap,malloc -Wl,--wrap,free".
 * This tells the linker to resolve references to malloc as
 * __wrap_malloc, free as __wrap_free, __real_malloc as malloc, and
 * __real_free as free.
 */
/* $begin interposel */
#ifdef LINKTIME
#include <stdio.h>

void *__real_malloc(size_t size);
void __real_free(void *ptr);

/* malloc wrapper function */
void *__wrap_malloc(size_t size)
{
    void *ptr = __real_malloc(size); /* Call libc malloc */
    printf("malloc(%d) = %p\n", (int)size, ptr);
    return ptr;
}

/* free wrapper function */
void __wrap_free(void *ptr)
{
    __real_free(ptr); /* Call libc free */
    printf("free(%p)\n", ptr);
}
#endif
/* $end interposel */

/*
 * Compile-time interpositioning of malloc and free using the C
 * preprocessor. A local malloc.h file defines malloc and free as
 * wrappers mymalloc and myfree respectively.
 */
/* $begin interposec */
#ifdef COMPILETIME
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

/* prevent the local malloc.h macro from recursively wrapping us */
#undef malloc
#undef free

/* malloc wrapper function */
void *mymalloc(size_t size)
{
    void *ptr = malloc(size);
    printf("malloc(%d)=%p\n", (int)size, ptr);
    return ptr;
}

/* free wrapper function */
void myfree(void *ptr)
{
    printf("free(%p)\n", ptr);
    free(ptr);
}
#endif
/* $end interposec */
