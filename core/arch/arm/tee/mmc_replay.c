#include <types_ext.h>
#include <inttypes.h>
#include <trace.h>

typedef uint32_t u32;

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

#define CHECK_DIVERGENCE() \
	if (val != expected) { \
		DMSG("%d:not as expected (val = %08x, expected = %08x)...divergence!\n", __LINE__, val, expected); \
	}\

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
		expected = 0x00000051;
	} else {
		expected = 0x00000098;
	}
	val = bcm2835_sdhost_read(host, SDCMD);
	CHECK_DIVERGENCE();

	expected = 0x00000900;
	val = bcm2835_sdhost_read(host, SDRSP0);
	CHECK_DIVERGENCE();
	return;
}
