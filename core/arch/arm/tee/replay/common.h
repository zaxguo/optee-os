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

#define printk EMSG
//#define printk(...) (void)(0);

static __always_inline void req_read(void *base, int off, u32 expected) {
	u32 val = readl(base + off);
	printk("reading [%08x%s%08x]:[%p:%x] (%d)\n", val, (val == expected) ? "==" : "!=", expected, base, off, __LINE__);
}

static __always_inline void reply_read(void *base, int off, u32 expected) {
	u32 val = readl(base + off);
	printk("reading [%08x%s%08x]:[%p:%x] (%d)\n", val, (val == expected) ? "==" : "!=", expected, base, off, __LINE__);

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

static dma_addr_t prepare_cb(int dir) {
	u32 alignment = 8;
	u32 *cb = malloc(4096);
	u32 *data = malloc(4096 + (1 << alignment) + 8);
	if (!cb) {
		printk("lwg:%s:%d: cannot get the shm of %d bytes!!!\n", __func__, __LINE__, 32);
		return 0;
	}
	if (!data) {
		printk("lwg:%s:%d: cannot get the shm of %d bytes!!!\n", __func__, __LINE__, 4096);
		return 0;
	}
	printk("cb @ %p:%08x\n", cb, virt_to_phys(cb));
	printk("data @ %p:%08x\n", data, virt_to_phys(data));
	/* ascii 8 */
	if ((u32)cb & ((1 << alignment) - 1)) {
		cb = align(cb, alignment);
		printk("cb @ %p:%08x\n", cb, virt_to_phys(cb));
	}
	if ((u32)data & ((1 << alignment) -1)) {
		data = align(data, alignment);
	}
	memset(data, 0x39, 4096);
	dsb();
	if (dir == TO_DEV) {
		/* src */
		*cb = 0x000d0149;
		*(cb + 1) = virt_to_phys(data) - DMA_OFF;
		/* dst */
		*(cb + 2) = 0x7e202040;
		/* length */
		*(cb + 3) = 0x00001000;
	} else if (dir == FROM_DEV) {
		*cb = 0x000d0419;
		/* src */
		*(cb + 1) = 0x7e202040;
		/* dst */
		*(cb + 2) = virt_to_phys(data) - DMA_OFF;
		/* length */
		*(cb + 3) = 0x00000ff4;

	}
	*(cb + 4) = 0x0;
	*(cb + 5) = 0x0;
	*(cb + 6) = 0x0;
	*(cb + 7) = 0x0;
	/*print_hex_dump(KERN_WARNING, "cb:", DUMP_PREFIX_OFFSET, 16, 4, cb, 32, 0);*/
	//DHEXDUMP(cb, 32);
	//__flush_dcache_area(cb, 32);
	cache_operation(TEE_CACHEINVALIDATE, cb, 4096);
	cache_operation(TEE_CACHEINVALIDATE, data, 4096);
	return virt_to_phys(cb) - DMA_OFF;
}



#endif

