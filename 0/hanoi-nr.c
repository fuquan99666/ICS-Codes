// The non-recursive hanoi problem implementation using a stack from jyy's os class
// Here we don't use recursion in C, but we simulate the recursion with a stack of frames
// We save the state of each recursive call in a frame(its pc, n, source, target, via) and push it onto the stack

#include <stdio.h>
#include <assert.h>

typedef struct {
    int pc, n; // pc: program counter, n: number of disks
    char source, target, via; 
} Frame;

#define call(...) ({ *(++top) = (Frame) { .pc = 0, __VA_ARGS__ }; })
#define ret() ({ top--; })
#define goto(loc) ({ f->pc = (loc) - 1;})

void hanoi(int n, char source, char target, char via) {

    Frame stack[100]; // stack to hold frames

    Frame *top = stack-1; // initialize top of stack 
    call(n, source, target, via);

    for (Frame *f; ( f = top) >= stack; f->pc++) { // loop until stack is empty (no more frames to run)
        n = f->n;
        source = f->source;
        target = f->target;
        via = f->via;

        // for each frame(small hanoi), just cv the logic from hanoi.c
        // but instead of calling hanoi recursively, we push a new frame onto the stack
        switch (f->pc) {
            case 0:  if (n == 1) { printf("%c -> %c\n", source, target); goto(4); } break;
            case 1:  call(n - 1, source, via, target); break;
            case 2:  call(    1, source, target, via); break;
            case 3:  call(n - 1, via, target, source); break;
            case 4:  ret(); break;
            default: assert(0); // should never reach here
        }
    }
}

int main() {
    int n;
    scanf("%d", &n);
    hanoi(n, 'A', 'C', 'B');
}