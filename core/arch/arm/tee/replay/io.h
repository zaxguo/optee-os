#include "block.h"
static void io (void)
{
	bio(READ, 213552, 32);
	bio(READ, 216224, 256);
	bio(READ, 213584, 256);
	bio(READ, 213840, 256);
	bio(READ, 215528, 256);
	bio(READ, 214952, 256);
	bio(READ, 215784, 152);
	bio(READ, 214240, 256);
	bio(READ, 214496, 224);
	bio(READ, 214096, 144);
	bio(READ, 214808, 144);
	bio(READ, 4859, 5);
	bio(READ, 6080, 1);
	bio(READ, 6081, 2);
	bio(READ, 8488, 1);
	bio(READ, 8489, 2);
	bio(READ, 9368, 32);
	bio(READ, 214720, 88);
	bio(READ, 215936, 256);
	bio(READ, 2608, 2);
	bio(READ, 2610, 6);
	bio(READ, 3864, 1);
	bio(READ, 3865, 1);
	bio(READ, 3866, 6);
	bio(READ, 4856, 1);
	bio(READ, 4857, 2);
	bio(READ, 7600, 1);
	bio(READ, 7601, 1);
	bio(READ, 7602, 6);
	bio(READ, 8491, 5);
	bio(READ, 236296, 32);
	bio(READ, 7728, 6);
	bio(READ, 27616, 32);
	bio(READ, 6083, 5);
	bio(READ, 9400, 5);
	bio(READ, 236328, 64);
	bio(READ, 236392, 49);
	bio(READ, 215216, 256);
	bio(READ, 215208, 8);
	bio(READ, 215472, 56);
	bio(READ, 27648, 11);
	bio(READ, 2177, 1);
	bio(READ, 2178, 1);
	bio(READ, 2179, 1);
	bio(READ, 238304, 1);
	bio(WRITE, 248128, 1);
	bio(WRITE, 2175, 5);
	bio(WRITE, 2431, 5);
	bio(WRITE, 2568, 1);
	bio(WRITE, 249240, 8);
	bio(WRITE, 251552, 1008);
	bio(WRITE, 253576, 1008);
	bio(WRITE, 2568, 1);
	bio(WRITE, 249240, 1);
}
