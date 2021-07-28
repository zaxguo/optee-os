#include "common.h"

static void rd_template(int sec, int count, void *replay_dma_chan, void *host)
{
	dma_addr_t cb = prepare_cb(FROM_DEV, count);
	req_read(host, SDEDM, 0x00010801);
	req_read(host, SDCMD, 0x00000052);
	req_read(host, SDHSTS, 0x00000000);
	req_write(host, SDARG, 0x00000008);
	req_write(host, SDCMD, 0x00008017);
	// poll for 0x8000 flag to turn off
	//req_read(host, SDCMD, 0x00008017);
	//req_read(host, SDCMD, 0x00000017);
	u32 val;
	do {
		udelay(10);
		val = readl(host + SDCMD);
		cpu_relax();
	} while (val & 0x8000);
	//poll(host, SDCMD, 0x00000017);

	req_read(host, SDRSP0, 0x00000900);
	req_read(host, SDCMD, 0x00000017);
	req_read(host, SDHSTS, 0x00000000);
	req_write(host, SDHCFG, 0x0000040e);
	req_write(host, SDHBCT, 0x00000200);
	req_write(host, SDHBLC, 0x00000008);
	/* lwg: srs sector parameterize */
	req_write(host, SDARG, sec);
	req_write(host, SDCMD, 0x00008052);
	req_write(replay_dma_chan, BCM2835_DMA_ADDR, cb);
	req_write(replay_dma_chan, BCM2835_DMA_CS, 0x00000001);
	req_read(replay_dma_chan, BCM2835_DMA_CS, 0x00000003);
	req_read(host, SDCMD, 0x00000052);
	req_read(host, SDRSP0, 0x00000900);
	/* poll irq */
	do {
		val = readl(replay_dma_chan + BCM2835_DMA_CS);
		cpu_relax();
	} while((val &= 0x00000004) == 0);
	reply_write(replay_dma_chan, BCM2835_DMA_CS, 0x00000004);
	reply_read(host, SDHSTS, 0x00000001);
	reply_read(host, SDCMD, 0x00000052);
	reply_read(host, SDEDM, 0x00010834);
	reply_read(host, SDEDM, 0x00010834);
	reply_read(host, SDDATA, IGNORE_VAL);
	reply_read(host, SDEDM, 0x00010824);
	reply_read(host, SDDATA, IGNORE_VAL);
	reply_read(host, SDEDM, 0x00010814);
	reply_read(host, SDDATA, IGNORE_VAL);
	reply_write(host, SDHCFG, 0x0000040e);
	reply_read(host, SDEDM, 0x00010804);
	reply_write(host, SDEDM, 0x00090804);
}
static void rd_8(int sec, void *replay_dma_chan, void *host) {
	rd_template(sec, 8, replay_dma_chan, host);
}
