#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main() {
	// use setbuf to change the mode of stream buffer 
  setvbuf(stdout, NULL, _IONBF, 0);
	printf("May not be printed twice");
	pid_t p = fork();

	if (p < 0) {
		fprintf(stderr, "fork failed\n");
	}

	if (p == 0) {
		// child 
		printf("\nChild Yep!\n");
		exit(0);
	} else {
		// parent 
		printf("\nParent Yep!\n");
	}
	return 0;
}

