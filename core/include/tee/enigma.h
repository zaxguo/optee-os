#ifndef _ENIGMA_H
#define _ENIGMA_H

#include <tee_api_types.h>
#include "enigma_types.h"

#define BTT_SIZE (30001)
#define BTT_ENTRY_SIZE (sizeof(btt_e))
#define SECTOR_SIZE (512)
#define SBD_NUM 10

#define BTT_ENC 0
#define BTT_DEC	1

typedef uint64_t btt_e;
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
	btt_e *btt[SBD_NUM];
	/* TODO: change to OPTEE crypto interface */
	crypto_skcipher *cipher;
	struct block_map b_map;

};

struct lookup_result {
	bool shared;
	btt_e block;
};

extern struct enigma_cb enigma_cb;

int init_btt_for_device(int lo_number);
btt_e *alloc_btt(unsigned long size);
int free_btt(btt_e *btt);
int init_enigma_cb(void);
int decrypt_btt_entry(btt_e *btt);
int look_up_block(int dev_id, btt_e vblock, btt_e *pblock);


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
	return pblk != (btt_e)NULL_BLK;
}

#endif

