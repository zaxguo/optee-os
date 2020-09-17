#include <tee/enigma.h>

/* enigma control block, which stores btt, cipher, etc. */
struct enigma_cb;

static int __look_up_block(btt_e *btt, btt_e vblock) {
	return 0;
}

/* TODO: currently only support single block alloc range alloc?? */
static int alloc_block(int dev_id, btt_e vblock, btt_e *pblock) {
	if (!has_enigma_cb()) {
		return NULL_CB;
	}
	/* TODO: lock */
	struct enigma_cb *cb = &enigma_cb;
	struct block_map *b_map = &cb->b_map;
	btt_e *btt = get_btt_for_device(dev_id);
	uint64_t remaining = b_map->total_size - b_map->allocated;
	/* minimal alloc unit is 1 sector */
	if (remaining < SECTOR_SIZE) {
		return ALLOC_FAIL;
	}
	/* exceed the size of btt */
	if (vblock > BTT_SIZE) {
		return ALLOC_FAIL;
	}
	/* actual block allocation */
	*pblock = b_map->idx++;
	btt[vblock] = *pblock;
	b_map->allocated += SECTOR_SIZE;
	return 0;
}

/* Given a SDB dev id and its virtual block, return a phyiscal disk block */
/* This is the core BTT logic */
int look_up_block(int dev_id, btt_e vblock, btt_e *pblock) {
	if (!has_enigma_cb()) {
		return -1;
	}
	btt_e *btt = get_btt_for_device(dev_id);
	*pblock = btt[vblock];
	/* not allocated, try to allocate */
	if (!pblk_allocated(*pblock)) {
			int err = alloc_block(dev_id, vblock, pblock);
			if (err == ALLOC_FAIL) {
				/* TODO: debug info, etc.*/
				*pblock = NULL_BLK;
				return LOOKUP_FAIL;
			}
			return 0;
	}
	return 0;
}


