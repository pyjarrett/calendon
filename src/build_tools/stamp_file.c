#include <stdlib.h>
#include <stdio.h>

int main(int argc, char** argv) {
	if (argc != 2) {
		printf("Usage: %s filename\n", argv[0]);
		printf("Writes blank file to the given filename\n");
		return EXIT_FAILURE;
	}

	FILE* file = fopen(argv[1], "w");
	fclose(file);
	return EXIT_SUCCESS;
}
