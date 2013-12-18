/*
 * gen_rom_crc.c
 *
 * Generate CRC from the bin (4740 and later)
 *
 * Usage:
 *	modify SREC_LEN
 *	bin_to_32bit_bin  xxx.bin xxx.tmp
 *	gen_rom_crc xxx.tmp > xxx.crc
 *
 * Copyright (c) 2013-2015, Ingenic Semiconductor Inc.
 */

#include <stdio.h>

#define SREC_LEN 8192 /* 32KB size */
#define CRC_16	0x8005
#define CRC_ITU	0x1021
#define CRC_SEL CRC_16

unsigned int source_data[SREC_LEN];
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
	int i, j, counter;
	short test = 0, reverse = 0;

	if (argc != 2) {
		perror("arguments error");
		return -1;
	}
	/* open original ROM srec */
	fp = fopen(argv[1], "rt");

	/* gen crc */
	if (fp) {
		for (i=0; i<SREC_LEN-1; i++) {
			fscanf(fp, "%s", bin_buf);
			source_data[i] = str_to_binary(bin_buf);
			for(j=1, counter=0; counter<32; counter++) {
				if (test<0)
					test = ( (test<<1) | ((source_data[i] & j)>>counter) ) ^ CRC_SEL;
				else
					test = (test<<1) | ((source_data[i] & j)>>counter);
				j=j<<1;
			}
		}
		// get final CRC
		fscanf(fp, "%s", bin_buf);
		source_data[i] = str_to_binary(bin_buf);
		for(j=1, counter=0; counter<16; counter++) {
			if (test<0)
				test = ( (test<<1) | ((source_data[i] & j)>>counter) ) ^ CRC_SEL;
			else
				test = (test<<1) | ((source_data[i] & j)>>counter);
			j=j<<1;
		}
		for(counter=0; counter<16; counter++) {
			reverse = (reverse<<1) | (test>>counter) & 1;
		}
		if (CRC_SEL == CRC_16)
			printf ("CRC_16 0x%04x, 0x%04x(rev)\n", (unsigned short)test, (unsigned short)reverse);
		else
			printf ("CRC_ITU_T 0x%04x, 0x%04x(rev)\n", (unsigned short)test, (unsigned short)reverse);
		fclose(fp);
		return 0;
	} else {
		perror("Can not open rom file");
		return 1;
	}
}
