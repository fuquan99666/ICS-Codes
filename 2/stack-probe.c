#include<stdio.h>
#include<stdint.h>

// volatile make sure compiler don't optimize it 
void * volatile low;
void * volatile high;

void update_range(void *ptr){
	if (ptr < low) low = ptr;
	if (ptr > high) high = ptr;
}

void probe(int n) {
	update_range(&n);
	long sz = (uintptr_t)high - (uintptr_t)low;
	if (sz % 1024 < 32) {
		printf("Stack >= %ld KB\n", sz / 1024);
	}
	probe(0);
}

int main() {
	// no buffer for stdout , to make sure all stdout is printed before segmentation fault 
	setbuf(stdout, NULL);
	low = (void *) -1; // 0xFFFFFFFFFFFFFFFF
	high = (void *) 0; // 0x0000000000000000
	probe(0);
	return 0;
}
