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

void replay_entry(struct replay_cb *cb) {
	void *sdhost = cb->sdhost_base;
	void *dma	 = cb->dma_base;
	int i = 0;
	int ms_diff;
	TEE_Time start, end;
	tee_time_get_sys_time(&start);
	for (; i < 10; i++) {
		wr_256(0, dma, sdhost);
		/*rd_8(0, dma, sdhost);*/
	}
	tee_time_get_sys_time(&end);
	ms_diff = (end.seconds - start.seconds)*1000 + (end.millis - start.millis);
	EMSG("replaying: %d KB, time = %d ms, tput = %d KB/s...\n",
			i * 4, ms_diff, (i*4)*1000/ms_diff);
}

#if 0
static inline u32 bcm2835_sdhost_read(void *host, u32 val) {
	return *(u32 *)(host + val);
}

static inline void bcm2835_sdhost_write(void *host, u32 val, u32 reg) {
	*(u32 *)(host + reg) = val;
}

void replay_read_single_block(void *host, u32 block, int rw) {
	u32 expected, val;

	expected = 0x00010801;
	val= bcm2835_sdhost_read(host, SDEDM);
	CHECK_DIVERGENCE();

	expected = 0x0000000d;
	val = bcm2835_sdhost_read(host, SDCMD);
	CHECK_DIVERGENCE();

	expected = 0x00000000;
	val = bcm2835_sdhost_read(host, SDHSTS);
	CHECK_DIVERGENCE();

	val = 0x0000041e;
	bcm2835_sdhost_write(host, val, SDHCFG);

	val = 0x00000200;
	bcm2835_sdhost_write(host, val, SDHBCT);

	val = 0x00000001;
	bcm2835_sdhost_write(host, val, SDHBLC);

	val = 0x00000800;
	bcm2835_sdhost_write(host, val, SDARG);

	if (rw == 0) {
		val = 0x00008051;
		expected = 0x00008051;
	} else {
		val = 0x00008098;
		expected = val;
	}
	bcm2835_sdhost_write(host, val, SDCMD);

	val = bcm2835_sdhost_read(host, SDCMD);
	CHECK_DIVERGENCE();

	if (rw == 0) {
		expected = 0x00008051;
	} else {
		expected = 0x00008098;
	}
	val = bcm2835_sdhost_read(host, SDCMD);
	CHECK_DIVERGENCE();

	expected = 0x00000900;
	val = bcm2835_sdhost_read(host, SDRSP0);
	CHECK_DIVERGENCE();
	return;
}

void replay_irq(void *host) {
	u32 val, expected, word, i, j;

	while((bcm2835_sdhost_read(host, SDHSTS) & SDHSTS_DATA_FLAG) != 0x1) {
		DMSG("polling..\n");
	}

	expected = 0x0000001;
	val = bcm2835_sdhost_read(host, SDHSTS);
	CHECK_DIVERGENCE();

	val = 0x00000701;
	bcm2835_sdhost_write(host, val, SDHSTS);

	word = 16;
	i = 0;
	while(i < 128/word) {
		j = 0;
		expected = 0x00010902;
		val = bcm2835_sdhost_read(host, SDEDM);
		CHECK_DIVERGENCE();
		while (j < word) {
			val = bcm2835_sdhost_read(host, SDDATA);
			DMSG("reading %08x...\n", val);
			j++;
		}
		i++;
	}
	expected = 0x00000000;
	val = bcm2835_sdhost_read(host, SDHSTS);
	CHECK_DIVERGENCE();

	val = 0x0000040e;
	bcm2835_sdhost_write(host, val, SDHCFG);

	expected = 0x00010801;
	val = bcm2835_sdhost_read(host, SDEDM);
	CHECK_DIVERGENCE();

	expected = 0x00000000;
	val = bcm2835_sdhost_read(host, SDHSTS);
	CHECK_DIVERGENCE();
}


void itr_core_handler(void) {
	uint32_t val;
	DMSG("handling irq by replaying...\n");
	void *irq_base = phys_to_virt_io(0x3f00b200);
	void *local_irq_base = phys_to_virt_io(0x40000000);
	void *host = phys_to_virt_io(0x3f202000);
	val = io_read32(irq_base);
	DMSG("pend 0 = %08x\n", val);
	val = io_read32(irq_base + 0x4);
	DMSG("pend 1 = %08x\n", val);
	val = io_read32(irq_base + 0x8);
	DMSG("pend 2 = %08x\n", val);

	thread_mask_exceptions(THREAD_EXCP_FOREIGN_INTR);
	int cpu = get_core_pos();
	val = io_read32(local_irq_base + 0xc + 4 * cpu);
	DMSG("pending bit = %08x...\n", val);

	replay_irq(host);

}
#endif
