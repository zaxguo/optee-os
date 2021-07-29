#include <types_ext.h>
#include <inttypes.h>
#include <trace.h>
#include <kernel/interrupt.h>
#include <kernel/misc.h>
#include <mm/core_memprot.h>
#include <mm/core_mmu.h>
#include <io.h>
#include <kernel/tee_time.h>

#define CHECK_DIVERGENCE() \
	if (val != expected) { \
		DMSG("%d:not as expected (val = %08x, expected = %08x)...divergence!\n", __LINE__, val, expected); \
	}\

#include "replay/wr_8.h"
#include "replay/rd_8.h"
#include "replay/replay_cb.h"
#include "replay/block.h"
#include "replay/select3.h"

struct replay_cb *replay_cb;


static void _bio(int rw, int sec, int cnt) {
	switch (cnt) {
	case 1:
	case 8:
		if (rw == READ) {
			rd_8(sec, replay_cb->dma_base, replay_cb->sdhost_base);
		} else {
			wr_8(sec, replay_cb->dma_base, replay_cb->sdhost_base);
		}
		break;
	case 32:
		if (rw == READ) {
			rd_32(sec, replay_cb->dma_base, replay_cb->sdhost_base);
		} else {
			wr_32(sec, replay_cb->dma_base, replay_cb->sdhost_base);
		}
		break;
	case 256:
		if (rw == READ) {
			rd_256(sec, replay_cb->dma_base, replay_cb->sdhost_base);
		} else {
			wr_256(sec, replay_cb->dma_base, replay_cb->sdhost_base);
		}
		break;
	default:
		EMSG("Unrecorded bio size!!!!! Abort!\n");
		while(1);
		break;
	}
}

/* simple strategy to issue the actual bio */
void bio(int rw, int sec, int cnt) {
	printk("%s, %d, %d\n", (rw == READ) ? "READ" : "WRITE", sec, cnt);
	/* transform the block ID to be 3-word aligned */
	if (sec & ((1 << 3) - 1)) {
		int old_sec = sec;
		sec = (sec >> 3) << 3;
		printk("adjust...%d => %d\n", old_sec, sec);
	}
	/* directly match */
	if (cnt == 1 || cnt == 8 || cnt == 32 || cnt == 256) {
		_bio(rw, sec, cnt);
		return;
	}
	/* greater than rw 100 sectors all go to 256
	 * Check: we dont have over 256 sector access */
	if (cnt > 100) {
		_bio(rw, sec, 256);
	/* fit using 32 */
	} else if (cnt > 8 && cnt < 100) {
		/* how many 32-sector read we need? */
		int need = (cnt - 1)/32 + 1;
		int i;
		for (i = 0; i < need; i++) {
			_bio(rw, sec + i * 32, 32);
		}
	} else if (cnt < 8) {
		_bio(rw, sec, 8);
	}
	return;
}

void replay_entry(struct replay_cb *cb) {
	void *sdhost = cb->sdhost_base;
	void *dma	 = cb->dma_base;
	int i = 0;
	int ms_diff;
	TEE_Time start, end;
	replay_cb = cb;
	tee_time_get_sys_time(&start);
	select3();
#if 0
	/* simple throughput test */
	for (; i < 10; i++) {
		/*wr_256(0, dma, sdhost);*/
		/*rd_8(0, dma, sdhost);*/
	}
#endif
	tee_time_get_sys_time(&end);
	ms_diff = (end.seconds - start.seconds)*1000 + (end.millis - start.millis);
	EMSG("replaying: %d KB, time = %d ms, tput = %d KB/s...\n",
			i * 4, ms_diff, (i*4)*1000/ms_diff);
}
