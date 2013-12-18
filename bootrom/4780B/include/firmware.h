#define RESET_LEN 6
unsigned int reset_handler[RESET_LEN] = {
	0x241a0000,		// li k0,0
	0x409a6000,		// mtc0 k0, c0_status
	0x00000000,		// nop
	0x42000020,		// wait
	0x1000fffe,		// b
	0x00000000		// nop
};
