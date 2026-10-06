#include <stdio.h>
#include <string.h>

void target() {
    printf("target called\n");
}

int main() {
    // 拷贝 target 开头 16 字节到缓冲区
    volatile unsigned char code_copy[16];
    memcpy((void*)code_copy, (void*)target, 16);
    
    // 打印出来看看
    for (int i = 0; i < 16; i++) {
        printf("%02x ", code_copy[i]);
    }
    printf("\n");
    
    target();
    return 0;
}
