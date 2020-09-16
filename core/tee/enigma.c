#include <tee/enigma.h>

/* enigma control block, which stores btt, cipher, etc. */
struct enigma_cb;

/* Given a SDB dev id and its virtual block, return a phyiscal disk block */
/* This is the core BTT logic */
int look_up_block(int dev_id, btt_e vblock, btt_e *pblock) {
	if (!has_enigma_cb()) {
		return -1;
	}
	btt_e *btt = get_btt_for_device(dev_id);
	*pblock = btt[vblock];
	return 0;
}

int alloc_btt_entries(int dev_id, btt_e vblock, btt_e *pblock) {

}
