#include "common.h"

static void wr_8(int sec, void *replay_dma_chan, void *host) {
	if (replay_dma_chan == NULL || host == NULL) {
		EMSG("dma: %p or host: %p, invalid!\n", replay_dma_chan, host);
		return;
	}
	dma_addr_t cb = prepare_cb(TO_DEV);
	req_read(host, SDEDM, 0x00010801);
	req_read(host, SDCMD, 0x0000000d);
	req_read(host, SDHSTS, 0x00000000);
	req_write(host, SDHCFG, 0x0000040e);
	req_write(host, SDHBCT, 0x00000200);
	req_write(host, SDHBLC, 0x00000008);
	/*lwg: below sector number, parameterized */
	req_write(host, SDARG, sec);
	req_write(host, SDCMD, 0x00008099);
	req_write(replay_dma_chan, BCM2835_DMA_ADDR, cb);
	printk("%d:start... (debug = %08x, src = %08x)\n", __LINE__, readl(replay_dma_chan + BCM2835_DMA_DEBUG), readl(replay_dma_chan + BCM2835_DMA_SOURCE_AD));
	req_write(replay_dma_chan, BCM2835_DMA_CS, 0x00000001);
	req_read(replay_dma_chan, BCM2835_DMA_CS, 0x0000000b);
	req_read(host, SDCMD, 0x00000099);
	req_read(host, SDRSP0, 0x00000900);
	u32 val;
	do {
		cpu_relax();
		val = readl(replay_dma_chan + BCM2835_DMA_CS);
		printk("%d:poll... (val = %08x, debug = %08x, src = %08x)\n", __LINE__, val, readl(replay_dma_chan + BCM2835_DMA_DEBUG), readl(replay_dma_chan + BCM2835_DMA_SOURCE_AD));
		//udelay(1);
	} while ((val &= 0x00000004) == 0);
	/* ack */
	reply_write(replay_dma_chan, BCM2835_DMA_CS, 0x00000004);
	printk("%d:ack ... (debug = %08x, src = %08x)\n", __LINE__, readl(replay_dma_chan + BCM2835_DMA_DEBUG), readl(replay_dma_chan + BCM2835_DMA_SOURCE_AD));
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
}


