#ifndef _REPLAY_CB_H_
#define _REPLAY_CB_H_
struct replay_cb {
	void *dma_base;
	void *sdhost_base;
};

extern struct replay_cb *g_replay_cb;
#endif
