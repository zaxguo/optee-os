#include <tee/enigma.h>
#include <malloc.h>
#include <trace.h>

#define INIT_SYBIL_COUNT 2
/* enigma control block, which stores btt, cipher, etc. */
struct enigma_cb enigma_cb;
int actual_id = 5;
/* allocation start from sector 1 */
uint64_t fs_size = 1;



static int __look_up_block(sector_t *btt, sector_t vblock) {
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

static inline int is_filedata(sector_t pblk) {
	return (pblk == FILEDATA);
}

static inline int pblk_allocated(sector_t pblk) {
	return (pblk != NULL_BLK);
}

int inc_blk_ref(sector_t pblk) {
	btt_e *btt = get_btt_for_device(0);
	return ++btt[pblk];
}

int dec_blk_ref(sector_t pblk) {
	btt_e *btt =get_btt_for_device(0);
	return --btt[pblk];
}

int get_blk_ref(sector_t pblk) {
	btt_e *btt = get_btt_for_device(0);
	return btt[pblk];
}


/* an extremely simple sequential block allocation strategy  */
static int alloc_block(int dev_id, sector_t vblock, sector_t *pblock) {
	if (!has_enigma_cb()) {
		EMSG("Enigma cb not found!\n");
		return NULL_CB;
	}
	/* TODO: lock */
	struct enigma_cb *cb = &enigma_cb;
	struct block_map *b_map = &cb->b_map;
	uint64_t remaining = b_map->total_size - b_map->allocated;
	/* minimal alloc unit is 1 sector */
	if (remaining < SECTOR_SIZE) {
		return ALLOC_FAIL;
	}
	/* actual block allocation */
	*pblock = b_map->idx++;
	int cnt = inc_blk_ref(*pblock);
	/*EMSG("allocating pblk [%x] for vblk[%x] for dev [%d], ref = %d\n", *pblock, vblock, dev_id, cnt);*/
	/* TODO: suppose change it to alloc size */
	b_map->allocated += SECTOR_SIZE;
	return 0;
}

/* check the pblock passed from normal OS, allocate new blocks if necessary */
/* TODO: `vblock` is misleading. It is the pblock number _AFTER_ btt lookup
 * passed from OS
 * */
int look_up_block(int dev_id, sector_t vblock, sector_t *pblock) {
	if (!has_enigma_cb()) {
		EMSG("enigma cb is not initialized!\n");
		return -1;
	}
	/* enigma does not allocate block for actual fs, all linear mapped */
	if (dev_id == actual_id) {
		*pblock = vblock;
		/*EMSG("not allocating for actual.\n");*/
		return 0;
	}
	/* we donot allocate for filedata block */
	if (is_filedata(vblock)) {
		*pblock = vblock;
		EMSG("not allocating for filedata.\n");
		return 0;
	}
	/* not allocated metadata blk, try to allocate */
	if (!pblk_allocated(vblock)) {
			int err = alloc_block(dev_id, vblock, pblock);
			if (err) {
				/* TODO: debug info, etc.*/
				EMSG("alloc failed.., err = %d\n", err);
				*pblock = NULL_BLK;
				return LOOKUP_FAIL;
			}
			return 0;
	/* allocated, examine its ref count */
	} else {
		/* prepare to use the existing block if its ref count == 1 */
		*pblock = vblock;
		btt_e *btt = get_btt_for_device(0);
		/*EMSG("pblk [%x] has already been allocated (ref=%x). \n", vblock, btt[vblock]);*/
		/* break sharing: decrement the old ref count then alloc new block */
		if (get_blk_ref(*pblock) > 1) {
			dec_blk_ref(*pblock);
			int err = alloc_block(dev_id, vblock, pblock);
			if (err) {
					/* TODO: debug info, etc.*/
					EMSG("alloc failed.., err = %d\n", err);
					*pblock = NULL_BLK;
					return LOOKUP_FAIL;
			}
		}
		return 0;
	}
	return 0;
}


btt_e *alloc_btt(int entries) {
	int i;
	btt_e *head = malloc(BTT_ENTRY_SIZE * entries);
	/* ref count */
	for (i = 0; i < entries; i++) {
		*(head + i) = 0;
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
	for (i = 0; i < enigma_cb.dev_count; i++) {
		btt_e *_btt = alloc_btt(BTT_SIZE);
		enigma_cb.btt[i] = _btt;
		EMSG("btt[%d:%d] succussfully init..\n", i, BTT_SIZE);
	}
#endif
	enigma_cb.cipher = enigma_cb.btt[0];
	init_block_map(512, 512, 1024*14400, &enigma_cb.b_map);
	EMSG("enigma cb succussfully init...\n");
	EMSG("bmap_idx = %ld\n", enigma_cb.b_map.idx);
	return 0;
}





