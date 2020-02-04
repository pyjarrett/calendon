/**
 * A very simple tool to overwrite a file with an empty file.  This is used as
 * part of the knell build step on Windows to invalidate a dummy file to ensure
 * post build steps run and therefore runtime DLLs get copied into the runtime
 * directory of knell, to ensure fresh copies of knell-lib are provided.
 */
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
