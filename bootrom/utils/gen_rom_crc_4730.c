/*
 * gen_rom_crc.c
 *
 * Generate CRC from the bin (JZ4730)
 *
 * Usage:
 *	modify ROM_LEN
 *	bin_to_32bit_bin  xxx.bin xxx.tmp
 *	gen_rom_crc_4730 xxx.tmp > xxx.crc
 *
 * Copyright (c) 2005-2007, Ingenic Semiconductor Inc.
 */

#include <stdio.h>

#define ROM_LEN 256      /* 1024-bytes */
#define CRC_16	0x8016
#define CRC_ITU	0x1021
#define CRC_SEL CRC_16

unsigned int rom_data[ROM_LEN];
char bin_buf[32];

/*
  convert binary string to binary value
 */
int str_to_binary(char * str_val)
{
	int i, val = 0;
	for (i = 0; i < 32; i++) {
		val = val<<1;
		if (str_val[i] == '1')
			val |= 1;
	}
	return val;
}

int main(int argc, char**argv)
{
	FILE *fp;
	unsigned int i, j, counter;
	unsigned short test=0, reverse=0;

	if (argc != 2) {
		perror("arguments error");
		return -1;
	}
	/* open original ROM srec */
	fp = fopen(argv[1], "rt");

	/* gen crc */
	if (fp) {
		for (i = 0; i < ROM_LEN-1; i++) {
			fscanf(fp, "%s", bin_buf);
			rom_data[i] = str_to_binary(bin_buf);
			for(j = 1, counter = 0; counter < 32; counter++) {
				if ((short)test < 0)
					test = ((test ^ CRC_SEL) & 0xfffe) | (((rom_data[i] & j)>>counter) ^ 0x00000001);
				else
					test = (test<<1) | ((rom_data[i] & j)>>counter);
				j = j<<1;
			}
		}
		// get final CRC
		printf("current test=%04x\n", test);
		reverse = test;
		for (i = 0; i < 0xffffffff; i++) {
			test = reverse;
			for (j = 1, counter = 0; counter < 32; counter++) {
				if ((short)test < 0)
					test = ((test ^ CRC_SEL) & 0xfffe) | (((i & j)>>counter) ^ 0x00000001);
				else
					test = (test<<1) | ((i & j)>>counter);
				j = j<<1;
			}
			if (test == 0) break;
		}
		printf ("CRC_16 0x%08x (", i);
		for (j = 0; j < 32; j++) {
			printf("%d", (i & (1 << (31 - j))) ? 1 : 0);
		}
		printf(")\n");
		fclose(fp);
		return 0;
	} else {
		perror("Can not open rom file");
		return 1;
	}
}
