// The standard hanoi problem implementation with recursion


#include <stdio.h>

void hanoi(int n, char source, char target, char via) {
    if (n == 1) {
        printf("Move disk 1 from %c to %c\n", source, target);
        return;
    }
    hanoi(n - 1, source, via, target); // first move n-1 disks from source to via 
    printf("Move disk %d from %c to %c\n", n, source, target); // now we can move the last disk
    hanoi(n - 1, via, target, source); // finally move the n-1 disks from via to target 
}

int main() {
    int n;
    scanf("%d", &n);
    hanoi(n, 'A', 'C', 'B');
}