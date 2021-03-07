#ifndef _ENIGMA_H
#define _ENIGMA_H

#include <tee_api_types.h>
#include "enigma_types.h"

//#define BTT_SIZE (6001)
//#define BTT_SIZE (80001)
#define BTT_SIZE (200001)
#define BTT_ENTRY_SIZE (sizeof(btt_e))
#define SECTOR_SIZE (512)
//#define SBD_NUM 10
#define SBD_NUM 1

#define BTT_ENC 0
#define BTT_DEC	1

//typedef uint64_t btt_e;
/* lwg: maybe 32b is enough, which can squeeze into 32b reg during world switch */
typedef uint32_t sector_t;
typedef uint8_t	 btt_e;
typedef btt_e crypto_skcipher;


struct block_map {
	uint32_t alloc_size;
	uint32_t block_size;
	/* stats */
	uint64_t allocated;
	uint64_t total_size;
	/* next to be allocated sector */
	uint64_t idx;
};

struct enigma_cb {
	int dev_count;
	int btt_size;
	btt_e *btt[SBD_NUM];
	/* TODO: change to OPTEE crypto interface */
	crypto_skcipher *cipher;
	struct block_map b_map;

};

struct lookup_result {
	bool shared;
	sector_t block;
};

extern struct enigma_cb enigma_cb;

int init_btt_for_device(int lo_number);
btt_e *alloc_btt(int size);
int free_btt(btt_e *btt);
int init_enigma_cb(void);
int decrypt_btt_entry(sector_t *btt);
int look_up_block(int dev_id, sector_t vblock, sector_t *pblock);
int inc_blk_ref(sector_t pblk);
int get_blk_ref(sector_t pblk);
int dec_blk_ref(sector_t pblk);

#endif

