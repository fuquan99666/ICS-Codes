// Some basic functions that using stdarg.h, e.g. printf 

// 对于这个标准stdarg.h, 你可以点进去看看它的具体内容，然后你会惊讶的发现
// C标准库不负责这部分的实现，为什么呢？结合我们对ABI的理解，这种与ABI强相关的细节实现，
// 由编译器来实现是最合理的（一般是某builtin函数）。C标准库只负责提供一个接口，供你调用而已。
//#include <stdarg.h>

#include "stdarg.h"
#include <stdio.h>

// a help function that convert integer to string 
char *itoa(int value, char *str, int base) {
  // assume base is 10 ?
  char *p = str;
  if (value == 0) {
    str[0] = '0';
    str[1] = '\0';
    return str;
  }
  if (value < 0) {
    *p++ = '-';
    value = -value;
  }
  char *start = p;
  while (value) {
    *p++ = (value % base) + '0';
    value /= base;
  }
  // reverse the string
  char *end = p - 1;
  while (start < end) {
    char temp = *start;
    *start++ = *end;
    *end-- = temp;
  }
  *p = '\0';
  return str;
}

// the fundamental function : putch
// which means 它比库函数printf更基础，往往会根据不同机器架构有不同的实现
// 这里我们就模拟凑合一下
void putch(char a){
	printf("%c", a);
}

// 同理，凑合一下
void panic(char *s){
  fprintf(stderr, "%s", s);
}

// A toy implementation of printf 
// 通过这个例子，你应该会知道，“最基础”的库函数依旧是由一些更基础的函数组成 （一般称之为系统调用?）
int printf_1(const char *fmt, ...) {

	va_list ap;
	int d;
	char *s;

	va_start(ap, fmt);

	while(*fmt) {
    if (*fmt == '%') {
      fmt++;
      switch (*fmt++)
      {
      case 's':
        s = va_arg(ap, char *);
        for (; *s; s++) {
          putch(*s);
        }
        break;
      case 'd':
        d = va_arg(ap, int);
        char buf[32];
        itoa(d, buf, 10);
        for (char *q = buf; *q; q++) {
          putch(*q);
        }
        break;
      default:
        panic("Not implemented");
        break;
      }
    } else {
      putch(*fmt++);
    }
  }
  va_end(ap);
  return 0;
}

int main() {
	int a = 666;
	char b[100] = "暂时无法给你明确的答复";
	printf_1("%s : %d\n", b, a);
	return 0;
}
