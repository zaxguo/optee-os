#ifndef __COMMON_H_
#define __COMMON_H_

#include <stdint.h>
#include <malloc.h>
#include <trace.h>
#include <arm64.h>
#include <string.h>
#include <mm/core_mmu.h>
#include <tee/cache.h>
#include <kernel/delay.h>

#define DMA_OFF 0x40000000
#define SDCMD  0x00 /* Command to SD card              - 16 R/W */
#define SDARG  0x04 /* Argument to SD card             - 32 R/W */
#define SDTOUT 0x08 /* Start value for timeout counter - 32 R/W */
#define SDCDIV 0x0c /* Start value for clock divider   - 11 R/W */
#define SDRSP0 0x10 /* SD card response (31:0)         - 32 R   */
#define SDRSP1 0x14 /* SD card response (63:32)        - 32 R   */
#define SDRSP2 0x18 /* SD card response (95:64)        - 32 R   */
#define SDRSP3 0x1c /* SD card response (127:96)       - 32 R   */
#define SDHSTS 0x20 /* SD host status                  - 11 R   */
#define SDVDD  0x30 /* SD card power control           -  1 R/W */
#define SDEDM  0x34 /* Emergency Debug Mode            - 13 R/W */
#define SDHCFG 0x38 /* Host configuration              -  2 R/W */
#define SDHBCT 0x3c /* Host byte count (debug)         - 32 R/W */
#define SDDATA 0x40 /* Data to/from SD card            - 32 R/W */
#define SDHBLC 0x50 /* Host block count (SDIO/SDHC)    -  9 R/W */

#define SDEDM_FORCE_DATA_MODE   (1<<19)
#define SDEDM_CLOCK_PULSE       (1<<20)
#define SDEDM_BYPASS            (1<<21)

#define SDEDM_WRITE_THRESHOLD_SHIFT 9
#define SDEDM_READ_THRESHOLD_SHIFT 14
#define SDEDM_THRESHOLD_MASK     0x1f

#define SDEDM_FSM_MASK           0xf
#define SDEDM_FSM_IDENTMODE      0x0
#define SDEDM_FSM_DATAMODE       0x1
#define SDEDM_FSM_READDATA       0x2
#define SDEDM_FSM_WRITEDATA      0x3
#define SDEDM_FSM_READWAIT       0x4
#define SDEDM_FSM_READCRC        0x5
#define SDEDM_FSM_WRITECRC       0x6
#define SDEDM_FSM_WRITEWAIT1     0x7
#define SDEDM_FSM_POWERDOWN      0x8
#define SDEDM_FSM_POWERUP        0x9
#define SDEDM_FSM_WRITESTART1    0xa
#define SDEDM_FSM_WRITESTART2    0xb
#define SDEDM_FSM_GENPULSES      0xc
#define SDEDM_FSM_WRITEWAIT2     0xd
#define SDEDM_FSM_STARTPOWDOWN   0xf

#define SDDATA_FIFO_WORDS        16

#define FIFO_READ_THRESHOLD     4
#define FIFO_WRITE_THRESHOLD    4
#define ALLOW_CMD23_READ        1
#define ALLOW_CMD23_WRITE       0
#define ENABLE_LOG              1
#define SDDATA_FIFO_PIO_BURST   8
#define CMD_DALLY_US            1
#define SDCDIV_MAX_CDIV                 0x7ff

#define BCM2835_DMA_CS		0x00
#define BCM2835_DMA_ADDR	0x04
#define BCM2835_DMA_SOURCE_AD	0x0c
#define BCM2835_DMA_DEBUG	0x20

/* glue to Linux */
typedef unsigned int u32;
typedef u32 dma_addr_t;
#define FROM_DEV	0 // read
#define TO_DEV		1 // write
#define IGNORE_VAL	0xdeadbeef

#define readl(c) ({ u32 __v = io_read32(c); dsb(); __v; })
#define writel(val, c) ({ dsb(); io_write32(c, val); })
#define cpu_relax()  __asm__ __volatile__("": : :"memory")

//#define printk EMSG
#define printk(...) (void)(0);

static __always_inline void req_read(void *base, int off, u32 expected) {
	u32 val = readl(base + off);
	if (expected != IGNORE_VAL && val != expected) {
		//EMSG("reading [%08x%s%08x]:[%p:%x] (%d)\n", val, (val == expected) ? "==" : "!=", expected, base, off, __LINE__);
	}
}

static __always_inline void reply_read(void *base, int off, u32 expected) {
	u32 val = readl(base + off);
	if (expected != IGNORE_VAL && val != expected) {
		//EMSG("reading [%08x%s%08x]:[%p:%x] (%d)\n", val, (val == expected) ? "==" : "!=", expected, base, off, __LINE__);
	}
}

static inline void req_write(void *base, int off, u32 val) {
	printk("writing [%08x,%p:%x]\n", val, base, off);
	writel(val, base + off);
}

static inline void reply_write(void *base, int off, u32 val) {
	printk("writing [%08x,%p:%x]\n", val, base, off);
	writel(val, base + off);
}

static void poll(void *base, int off, u32 expected) {
	u32 val;
	u32 i, max_tries = 100;
	do {
		cpu_relax();
		val = readl(base + off);
		if (i++ > max_tries) {
			EMSG("poll execeeded max tries (val = %08x\n)", val);
			break;
		}
	} while (val != expected);
}

static void* align(void *old, int bits) {
	unsigned long tmp = (unsigned long)old;
	tmp = (tmp + (1 << bits)) & (~(unsigned long)((1<<bits) -1));
	return (void *)tmp;
}

static void *alloc_aligned(int size, int alignment) {
	u32 *ret = malloc(size + (1 << alignment) + sizeof(u32 *));
	if (!ret) {
		EMSG("lwg:%s:%d: cannot get mem of %d bytes!!!\n", __func__, __LINE__, size);
		return 0;
	}
	if ((u32)ret & ((1 << alignment) - 1)) {
		ret = align(ret, alignment);
		printk("cb @ %p:%08x\n", ret, virt_to_phys(ret));
	}
	return ret;
}

static void *alloc(int size, int alignment) {
	u32 alloc_size = size + (1 << alignment) + sizeof(u32 *);
	u32 *ret = malloc(alloc_size);
	return ret;
}

static void dump_cb(u32 *cb) {
	EMSG("cb[%08x]:%08x %08x %08x %08x %08x %08x %08x %08x", virt_to_phys(cb),*cb, *(cb + 1), *(cb + 2), *(cb + 3), *(cb + 4), *(cb + 5), *(cb + 6), *(cb + 7));
}


static void cleanup_mem(int count, u32 **cbs, u32 **pgs) {
	u32 i = 0;
	u32 n = (count - 1)/8 + 1;
	u32 **cb_list, **pg_list;
	cb_list = cbs;
	pg_list = pgs;
	cache_operation(TEE_CACHEINVALIDATE, pg_list[i], 4096);
	for (i = 0; i < n; i++) {
#if 0
		/* verify data */
		if (i == 0 || i == 20 || i == 31) {
			u32 *data = pg_list[i];
			data = align(data, 8);
			EMSG("[%d] data = %08x %08x %08x %08x\n", i, *data, *(data + 1), *(data + 2), *(data + 3));
		}
#endif
		free(cb_list[i]);
		free(pg_list[i]);
	}
}

static dma_addr_t prepare_cb(int dir, int count, u32 ***cbs, u32 ***pgs) {
	u32 i = 0;
	u32 alignment = 8;
	u32 n_cb = (count - 1)/8 + 1;
	u32 **cb_list = malloc(n_cb * sizeof(u32*));
	u32 **pg_list = malloc(n_cb * sizeof(u32*));
	*cbs  = cb_list;
	*pgs  = pg_list;
	dma_addr_t ret;
	for (i = 0; i < n_cb; i++) {
		u32 *cb = alloc(128, alignment);
		u32 *pg = alloc(4096, alignment);
		//u32 *cb = alloc_aligned(128, alignment);
		//u32 *pg = alloc_aligned(4096, alignment);
		*(cb_list + i) = cb;
		*(pg_list + i) = pg;
		printk("cb @ %p:%08x\n", cb, virt_to_phys(cb));
		printk("data @ %p:%08x\n", pg, virt_to_phys(pg));
	}
	for (i = 0; i < n_cb; i++) {
		u32 *cb = cb_list[i];
		u32 *data = pg_list[i];
		cb = align(cb, alignment);
		data = align(data, alignment);
		memset(cb, 0, 128);
		/* XXX:generated data for write */
		if (dir == TO_DEV) {
			memset(data, i, 4096);
		} else {
			memset(data, 0, 4096);
		}
		if (dir == TO_DEV) {
			/* src */
			*cb = 0x000d0148;
			*(cb + 1) = virt_to_phys(data) - DMA_OFF;
			/* dst */
			*(cb + 2) = 0x7e202040;
		} else if (dir == FROM_DEV) {
			*cb = 0x000d0418;
			/* src */
			*(cb + 1) = 0x7e202040;
			/* dst */
			*(cb + 2) = virt_to_phys(data) - DMA_OFF;
		}
		/* length */
		*(cb + 3) = 0x00001000;
		/* next cb */
		if (i < (n_cb - 1)) {
			u32 *next = cb_list[i + 1];
			next = align(next, alignment);
			*(cb + 5) = virt_to_phys(next) - DMA_OFF;
		}
		/* last chained cb */
		if (i == (n_cb - 1)) {
			/* flip active bit */
			*cb |= 0x1;
			/* read due to silicon bug */
			if (dir == FROM_DEV)
				*(cb + 3) = 0x00000ff4;
			*(cb + 5) = 0x0;
		}
		//DHEXDUMP(cb, 32);
		//dump_cb(cb);
		cache_operation(TEE_CACHEINVALIDATE, cb, 128);
		cache_operation(TEE_CACHEINVALIDATE, data, 4096);
		if (i == 0) {
			ret = virt_to_phys(cb) - DMA_OFF;
		}
	}
	return ret;
}

static inline void bcm2835_sdhost_write(void *base, int val, int off) {
	writel(val, base + off);
}

static inline int bcm2835_sdhost_read(void *base, int off) {
	return readl(base + off);
}

static void reset_sdhost(void *host) {
	bcm2835_sdhost_write(host, 0, SDCMD);
	bcm2835_sdhost_write(host, 0, SDARG);
	bcm2835_sdhost_write(host, 0xf00000, SDTOUT);
	bcm2835_sdhost_write(host, 0, SDCDIV);
	bcm2835_sdhost_write(host, 0x7f8, SDHSTS); /* Write 1s to clear */
	bcm2835_sdhost_write(host, 0, SDHCFG);
	bcm2835_sdhost_write(host, 0, SDHBCT);
	bcm2835_sdhost_write(host, 0, SDHBLC);

	/* Limit fifo usage due to silicon bug */
	int temp = bcm2835_sdhost_read(host, SDEDM);
	temp &= ~((SDEDM_THRESHOLD_MASK<<SDEDM_READ_THRESHOLD_SHIFT) |
		  (SDEDM_THRESHOLD_MASK<<SDEDM_WRITE_THRESHOLD_SHIFT));
	temp |= (FIFO_READ_THRESHOLD << SDEDM_READ_THRESHOLD_SHIFT) |
		(FIFO_WRITE_THRESHOLD << SDEDM_WRITE_THRESHOLD_SHIFT);
	bcm2835_sdhost_write(host, temp, SDEDM);
	mdelay(10);
	//bcm2835_sdhost_set_power(host, true);
	//mdelay(10);
	//bcm2835_sdhost_write(host, host->hcfg, SDHCFG);
	//bcm2835_sdhost_write(host, SDCDIV_MAX_CDIV, SDCDIV);
}


#endif

