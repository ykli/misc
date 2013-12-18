/*
 * bin_to_8bit_hex.c
 *
 * Convert binary format to hex format (string format) 8bit a line.
 *
 * Usage:	MEM_SIZE : The outfile hex file size in memory we want,
 *			   the final srec file size is 3*FILE_SIZE
 *		bin_to_8bit_hex xxx.bin xxx.srec (xxx.srec little-endian one line 8bit)
 *
 * Copyright (c) 2005-2007, Ingenic Semiconductor Inc.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>

#define MEM_SIZE 28672  /* 28KB */
int main(int argc, char *argv[])
{
	FILE *fdr, *fdw;
	struct stat fst;
	char *from, *to, *buf;
	int size, ret, i, j;
	char chars[16] = {'0', '1', '2','3','4','5','6','7','8','9','a','b','c','d','e','f'};
	char *newline = "\n";

	if (argc < 3) {
		printf("%s from-file to-file\n", argv[0]);
		exit(1);
	}

	from = argv[1];
	to = argv[2];

	if ((fdr = fopen(from, "rb")) == NULL) {
		printf("Cannot open file %s (%s)\n", from, strerror(errno));
		exit(1);
	}

	if (stat(from, &fst) == -1) {
		printf("Cannot get fstat %s (%s)\n", from, strerror(errno));
		exit(1);
	}

	size = fst.st_size;

	if (size > MEM_SIZE) {
		printf("file size is bigger than ROM_SIZE\n");
		exit(1);
	}

	buf = malloc(MEM_SIZE);
	if (!buf) {
		printf("No enough memory\n");
		exit(1);
	}
	memset(buf, 0, MEM_SIZE);

	if ((ret = fread(buf, sizeof(char), size, fdr)) != size) {
		printf("Cannot read %s (%s)\n", from, strerror(errno));
		exit(1);
	}

	fclose(fdr);

	if ((fdw = fopen(to, "w+")) == NULL) {
		printf("Cannot open file %s\n", to);
		exit(1);
	}

	for (i = 0; i < MEM_SIZE; i += 1) {
		char val = *(unsigned char *)(buf + i);
		for (j = 1; j >= 0; j--) {
			if (ret = fwrite(&chars[(val >> (4 * j)) & 0xf],
						sizeof(char),
						1,
						fdw) != 1) {
				printf("Cannot write to file %s\n",to);
				exit(1);
			}
		}

		if ((ret = fwrite(newline, sizeof(char), 1, fdw)) != 1) {
			printf("Cannot write to file %s\n", to);
			exit(1);
		}
	}

	fclose(fdw);

	return 0;
}
