// 这是一个“最小手写版”变参实现，只针对当前这个 32 位 x86 / gcc 场景。
// 为什么这样写：这里的 printf_1 只需要支持 int / char * 这类 4 字节参数，
// 所以可以直接按栈上传参顺序去取值，不需要完整的标准库 stdarg 实现。
// 之前的问题：把 &last 当成可变参数起点时，拿到的是局部变量的地址，
// 不是调用者压栈后的真实实参区；在当前编译结果下会导致参数错位。
// 现在的写法：直接从 ebp 计算出参数区起点，再按 sizeof(type) 逐个前进。

// 这个 ifndef xxx endif 写法 大家可以学习一下，如何保证一个头文件只被包含一次
// 这在复杂项目中非常重要！
#ifndef MY_STDARG_H
#define MY_STDARG_H

typedef char *va_list;

// 无奈，只能强制硬编码了
#define va_start(ap, last)  do { \
   void *__fp; \
   __asm__ volatile ("movl %%ebp, %0" : "=r"(__fp)); \
   (ap) = (char *)__fp + 12; \
} while (0)

// 我一开始是想用下面的写法，然而编译器会把 fmt 复制一份到 callee 的栈帧里，
// 导致 &last 不是调用者的实参区起点，而是 callee 的局部变量地址 。。。
// #define va_start(ap, last)  (ap = (char *)&last + sizeof(last))

#define va_arg(ap, type)    (*(type *)((ap += sizeof(type)) - sizeof(type)))
#define va_end(ap)          (ap = (char *)0)

#endif
