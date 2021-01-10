#include <tee/enigma.h>
#include <malloc.h>
#include <trace.h>

/* enigma control block, which stores btt, cipher, etc. */
struct enigma_cb enigma_cb;
int actual_id = 5;
uint64_t fs_size;



static int __look_up_block(btt_e *btt, btt_e vblock) {
	return 0;
}

static inline int has_enigma_cb(void) {
	return (enigma_cb.cipher != NULL);
}

static inline int has_btt_for_device(int lo_number) {
	return (enigma_cb.btt[lo_number] != NULL);
}

static inline btt_e *get_btt_for_device(int lo_number) {
	return enigma_cb.btt[lo_number];
}

static inline int pblk_allocated(btt_e pblk) {
	return pblk != NULL_BLK;
}



/* an extremely simple sequential block allocation strategy  */
static int alloc_block(int dev_id, btt_e vblock, btt_e *pblock) {
	if (!has_enigma_cb()) {
		EMSG("Enigma cb not found!\n");
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
	/* actual block allocation */
	*pblock = b_map->idx++;
	EMSG("allocating pblk [%x] for vblk[%x] for dev [%d]\n", *pblock, vblock, dev_id);
	/* TODO: suppose change it to alloc size */
	b_map->allocated += SECTOR_SIZE;
	return 0;
}

/* Given a SDB dev id and its virtual block, return a phyiscal disk block */
/* This is the core BTT logic
 * TODO: move this to normal world */
int look_up_block(int dev_id, btt_e vblock, btt_e *pblock) {
	if (!has_enigma_cb()) {
		EMSG("enigma cb is not initialized!\n");
		return -1;
	}
	/* enigma does not allocate block for actual fs, all linear mapped */
	if (dev_id == actual_id) {
		*pblock = vblock;
		EMSG("not allocating for actual..\n");
		return 0;
	}
	/* not allocated, try to allocate */
	if (!pblk_allocated(vblock)) {
			int err = alloc_block(dev_id, vblock, pblock);
			if (err) {
				/* TODO: debug info, etc.*/
				EMSG("alloc failed.., err = %d\n", err);
				*pblock = NULL_BLK;
				return LOOKUP_FAIL;
			}
			return 0;
	} else {
		EMSG("pblk has already been allocated!\n");
		*pblock = vblock;
		/* TODO: break */
		/* CoW needs to break the originla sharing... */
		return 0;
	}
	return 0;
}

btt_e *alloc_btt(int entries) {
	int i;
	btt_e *head = malloc(BTT_ENTRY_SIZE * 1);
	for (i = 0; i < entries; i++) {
		/**(head + i) = NULL_BLK;*/
	}
	return head;
}


static uint64_t get_sybil_start(void) {
	return fs_size;
}

static int init_block_map(uint32_t alloc_size, uint32_t block_size, uint64_t total_size, struct block_map *bmap) {
	bmap->block_size = block_size;
	bmap->allocated = alloc_size;
	bmap->total_size = total_size;
	bmap->idx = get_sybil_start();
	return 0;
}

int init_enigma_cb(void) {
	int i;
	enigma_cb.dev_count = SBD_NUM;
	enigma_cb.btt_size = BTT_SIZE;
#if 1
	/* not really useful */
	for (i = 0; i < enigma_cb.dev_count; i++) {
		btt_e *_btt = alloc_btt(1);
		enigma_cb.btt[i] = _btt;
	}
#endif
	enigma_cb.cipher = enigma_cb.btt[0];
	init_block_map(512, 512, 1024*14400, &enigma_cb.b_map);
	EMSG("enigma cb succussfully init..\n");
	return 0;
}





