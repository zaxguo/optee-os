#include "common.h"
static inline void req_read(void *base, int off, int expected) {
	u32 val = readl(base + off);
	/*printk("reading [%08x, %08x]:[%p:%d]\n", val, expected, base, off);*/
	if (val != (u32)expected) {
		/*printk("!!! divergence happened at %d!\n", __LINE__);*/
	}
}

static inline void reply_read(void *base, int off, int expected) {
	u32 val = readl(base + off);
	/*printk("reading [%08x, %08x]:[%p:%d]\n", val, expected, base, off);*/
	if (val != (u32)expected) {
		/*printk("!!! divergence happened at %d!\n", __LINE__);*/
	}

}

static inline void req_write(void *base, int off, int val) {
	/*printk("writing [%08x,%p:%d]\n", val, base, off);*/
	writel(val, base + off);
}

static inline void reply_write(void *base, int off, int val) {
	/*printk("writing [%08x,%p:%d]\n", val, base, off);*/
	writel(val, base + off);
}

extern void __flush_dcache_area(void *, int);

static dma_addr_t prepare_cb(void* teedev) {
	struct tee_context *ctx = kmalloc(sizeof(struct tee_context), GFP_KERNEL);
	ctx->teedev = teedev;
	INIT_LIST_HEAD(&ctx->list_shm);
	struct tee_shm *shm = tee_shm_alloc(ctx, 8 * sizeof(u32), TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
	struct tee_shm *data = tee_shm_alloc(ctx, 4096, TEE_SHM_MAPPED | TEE_SHM_DMA_BUF);
	if (IS_ERR(shm)) {
		printk("lwg:%s:%d: cannot get the shm of %d bytes!!!\n", __func__, __LINE__, 32);
		return 0;
	}
	if (IS_ERR(data)) {
		printk("lwg:%s:%d: cannot get the shm of %d bytes!!!\n", __func__, __LINE__, 4096);
		return 0;
	}
	memset(data->kaddr, 0xdd, 4096);
	wmb();

	u32 *cb = shm->kaddr;
	*cb = 0x000d0149;
	*(cb + 1) = data->paddr - DMA_OFF;
	*(cb + 2) = 0x7e202040;
	*(cb + 3) = 0x00001000;
	*(cb + 4) = 0x0;
	*(cb + 5) = 0x0;
	*(cb + 6) = 0x0;
	*(cb + 7) = 0x0;
	/*print_hex_dump(KERN_WARNING, "cb:", DUMP_PREFIX_OFFSET, 16, 4, cb, 32, 0);*/
	__flush_dcache_area(shm->kaddr, 32);
	return shm->paddr - DMA_OFF;
}

static void replay_dma_write(void *teedev, void *host) {
	unsigned long flags;

	if (replay_dma_chan == NULL || host == NULL) {
		printk("dma: %p or host: %p, invalid!\n", replay_dma_chan, host);
		return;
	}

	dma_addr_t cb = prepare_cb(teedev);
	/*printk("lwg:%s:%d: cb [%08x].\n", __func__, __LINE__, cb);*/
	/*spin_lock_irqsave(&replay_lock, flags);*/
	req_read(host, SDEDM, 0x00010801);
	req_read(host, SDCMD, 0x0000000d);
	req_read(host, SDHSTS, 0x00000000);
	req_write(host, SDHCFG, 0x0000040e);
	req_write(host, SDHBCT, 0x00000200);
	req_write(host, SDHBLC, 0x00000008);
	req_write(host, SDARG, 0x00000000);
	req_write(host, SDCMD, 0x00008099);
	req_write(replay_dma_chan, BCM2835_DMA_ADDR, cb);
	/*printk("%d:start... (debug = %08x, src = %08x)\n", __LINE__, readl(replay_dma_chan + BCM2835_DMA_DEBUG), readl(replay_dma_chan + BCM2835_DMA_SOURCE_AD));*/
	req_write(replay_dma_chan, BCM2835_DMA_CS, 0x00000001);
	req_read(replay_dma_chan, BCM2835_DMA_CS, 0x0000000b);
	req_read(host, SDCMD, 0x00000099);
	req_read(host, SDRSP0, 0x00000900);
	u32 val;
	do {
		cpu_relax();
		val = readl(replay_dma_chan + BCM2835_DMA_CS);
		/*printk("%d:poll... (val = %08x, debug = %08x, src = %08x)\n", __LINE__, val, readl(replay_dma_chan + BCM2835_DMA_DEBUG), readl(replay_dma_chan + BCM2835_DMA_SOURCE_AD));*/
		udelay(1);
	} while ((val &= 0x00000004) == 0);
	/* ack */
	reply_write(replay_dma_chan, BCM2835_DMA_CS, 0x00000004);
	/*printk("%d:ack ... (debug = %08x, src = %08x)\n", __LINE__, readl(replay_dma_chan + BCM2835_DMA_DEBUG), readl(replay_dma_chan + BCM2835_DMA_SOURCE_AD));*/
	reply_read(host, SDHSTS, 0x00000000);
	reply_read(host, SDCMD, 0x00000099);
	reply_read(host, SDEDM, 0x00010801);
	reply_write(host, SDHCFG, 0x0000040e);
	reply_read(host, SDCMD, 0x00000099);
	reply_read(host, SDHSTS, 0x00000000);
	reply_write(host, SDARG, 0x00000000);
	reply_write(host, SDCMD, 0x0000880c);
	/* poll */
#if 1
	do {
		val = readl(host + SDHSTS);
		/*printk("%d:poll... (val = %08x\n", __LINE__, val);*/
	} while (val != 0x00000400);
#endif
	/*reply_read(host, SDHSTS, 0x00000400);*/
	reply_write(host, SDHSTS, 0x00000701);
	reply_read(host, SDCMD, 0x0000080c);
	reply_read(host, SDRSP0, 0x00000c00);
	reply_read(host, SDHSTS, 0x00000000);

	req_read(host, SDEDM, 0x00010801);
	req_read(host, SDCMD, 0x0000080c);
	req_read(host, SDHSTS, 0x00000000);
	req_write(host, SDARG, 0x59b40000);
	req_write(host, SDCMD, 0x0000800d);
	req_read(host, SDCMD, 0x0000800d);
	req_read(host, SDCMD, 0x0000800d);
	req_read(host, SDCMD, 0x0000000d);
	req_read(host, SDRSP0, 0x00000900);
	/*spin_unlock_irqrestore(&replay_lock, flags);*/
}
