/*-
 *   BSD LICENSE
 *
 *   Copyright (c) Intel Corporation.
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * This is a simple example of a virtual block device module that passes IO
 * down to a bdev (or bdevs) that its configured to attach to.
 */

#include "spdk/stdinc.h"
#include "spdk/bdev.h"
#include "spdk/event.h"

#include "vbdev_rout.h"
#include "spdk/rpc.h"
#include "spdk/env.h"
#include "spdk/endian.h"
#include "spdk/string.h"
#include "spdk/thread.h"
#include "spdk/util.h"

#include "spdk/bdev_module.h"
#include "spdk/log.h"

FILE *pFILE = NULL;
int filefile = 0;
struct spdk_io_channel *ch1=NULL;
struct spdk_io_channel *ch2=NULL;
struct spdk_io_channel *ch3=NULL;
struct spdk_io_channel *ch4=NULL;
struct spdk_io_channel *ch5=NULL;
struct spdk_io_channel *ch6=NULL;
struct spdk_io_channel *ch7=NULL;
struct spdk_io_channel *ch8=NULL;
struct spdk_io_channel *ch9=NULL;
struct spdk_io_channel *ch10=NULL;
struct spdk_io_channel *ch11=NULL;

struct spdk_thread *th1 = NULL;
struct spdk_thread *th2 = NULL;
struct spdk_thread *th3 = NULL;
struct spdk_thread *th4 = NULL;
struct spdk_thread *th5 = NULL;
struct spdk_thread *th6 = NULL;
struct spdk_thread *th7 = NULL;
struct spdk_thread *th8 = NULL;
struct spdk_thread *th9 = NULL;
struct spdk_thread *th10 = NULL;
struct spdk_thread *th11 = NULL;

struct spdk_io_channel *nc1 = NULL;
struct spdk_io_channel *nc2 = NULL;
struct spdk_io_channel *nc3 = NULL;
struct spdk_io_channel *nc4 = NULL;
struct spdk_io_channel *nc5 = NULL;
struct spdk_io_channel *ncc1 = NULL;
struct spdk_io_channel *ncc2 = NULL;

struct spdk_thread *be1 = NULL;
struct spdk_thread *be2 = NULL;
struct spdk_thread *be3 = NULL;
struct spdk_thread *be4 = NULL;
struct spdk_thread *be5 = NULL;
struct spdk_thread *be6 = NULL;
struct spdk_thread *be7 = NULL;
struct spdk_thread *be8 = NULL;
struct spdk_thread *be9 = NULL;

int nnn = 0;
int ga = 0;
int ggd = 1;
int tot = 0;
int zeone = 0;
int zeone1 = 0;
struct spdk_io_channel *ch_all=NULL;
struct spdk_io_channel *ch_all1=NULL;
struct spdk_io_channel *ch_all2=NULL;
struct spdk_io_channel *ch_all3=NULL;
struct spdk_io_channel *ch_all4=NULL;
struct spdk_io_channel *ch_all5=NULL;
struct spdk_io_channel *ch_all6=NULL;
struct spdk_io_channel *ch_all7=NULL;
struct spdk_io_channel *ch_all8=NULL;
struct spdk_io_channel *ch_all9=NULL;
struct spdk_io_channel *ch_all10=NULL;

struct spdk_io_channel *ch_b = NULL;

struct spdk_io_channel *f_ch=NULL;
struct spdk_io_channel *n_ch=NULL;

int g_cnt = 0;
int g_cnt1 = 0;
int g_cnt2 = 0;
int g_cnt3 = 0;
int g_cnt4 = 0;
int g_cnt5 = 0;
int g_cnt6 = 0;
int g_cnt7 = 0;
int g_cnt8 = 0;
int g_cnt9 = 0;
int g_cnt10 = 0;
int g_1 = 0;

int ttt = 0;
	

char *buff;
int buc = 0;
int sec = 0;

int co1=0;
int co2=0;
int co3=0;
int co4=0;
int four00 = 0;


struct spdk_bdev_desc *bdev_desc;
char *buff;

int totall = 0;
#define SPDK_MAX_THREAD_NAME_LEN 256

  enum spdk_thread_state {
      SPDK_THREAD_STATE_RUNNING,
      SPDK_THREAD_STATE_EXITING,
      SPDK_THREAD_STATE_EXITED,
   };
 
  struct spdk_thread {  
	  uint64_t            tsc_last;
	  struct spdk_thread_stats    stats;
	  /*
	   * Contains pollers actively running on this thread.  Pollers
       *  are run round-robin. The thread takes one poller from the head
       *  of the ring, executes it, then puts it back at the tail of
       *  the ring.
       */
      TAILQ_HEAD(active_pollers_head, spdk_poller)    active_pollers;
      /**
       * Contains pollers running on this thread with a periodic timer.
       */
      RB_HEAD(timed_pollers_tree, spdk_poller)    timed_pollers;
      struct spdk_poller              *first_timed_poller;
      /*
       * Contains paused pollers.  Pollers on this queue are waiting until
       * they are resumed (in which case they're put onto the active/timer
       * queues) or unregistered.
       */
      TAILQ_HEAD(paused_pollers_head, spdk_poller)    paused_pollers;
      struct spdk_ring        *messages;
      int             msg_fd;
      SLIST_HEAD(, spdk_msg)      msg_cache;
      size_t              msg_cache_count;
      spdk_msg_fn         critical_msg;
      uint64_t            id;
      enum spdk_thread_state      state;
      int             pending_unregister_count;
 
      RB_HEAD(io_channel_tree, spdk_io_channel)   io_channels;
      TAILQ_ENTRY(spdk_thread)            tailq;
 
      char                name[SPDK_MAX_THREAD_NAME_LEN + 1];
      struct spdk_cpuset      cpumask;
      uint64_t            exit_timeout_tsc;
 
      /* Indicates whether this spdk_thread currently runs in interrupt. */
      bool                in_interrupt;
      struct spdk_fd_group        *fgrp;
 
      /* User context allocated at the end */
      uint8_t             ctx[0];
 };
 
 struct spdk_io_channel {
	 struct spdk_thread *thread;
	 struct io_device *dev;
	 uint32_t ref;
	 uint32_t destroy_ref;
	 RB_ENTRY(spdk_io_channel) node;
	 spdk_io_channel_destroy_cb destroy_cb;
     uint8_t _padding[40];
 };
struct spdk_bdev_channel {

	struct spdk_bdev	*bdev;

	/* The channel for the underlying device */
	struct spdk_io_channel	*channel;

	/* Per io_device per thread data */
	struct spdk_bdev_shared_resource *shared_resource;

	struct spdk_bdev_io_stat stat;

	/*
	 * Count of I/O submitted to the underlying dev module through this channel
	 * and waiting for completion.
	 */
	uint64_t		io_outstanding;

	/*
	 * List of all submitted I/Os including I/O that are generated via splitting.
	 */
	bdev_io_tailq_t		io_submitted;

	/*
	 * List of spdk_bdev_io that are currently queued because they write to a locked
	 * LBA range.
	 */
	bdev_io_tailq_t		io_locked;

	uint32_t		flags;

	struct spdk_histogram_data *histogram;

#ifdef SPDK_CONFIG_VTUNE
	uint64_t		start_tsc;
	uint64_t		interval_tsc;
	__itt_string_handle	*handle;
	struct spdk_bdev_io_stat prev_stat;
#endif

	bdev_io_tailq_t		queued_resets;

	lba_range_tailq_t	locked_ranges;
};


static int vbdev_rout_init(void);
static int vbdev_rout_get_ctx_size(void);
static void vbdev_rout_examine(struct spdk_bdev *bdev);
static void vbdev_rout_finish(void);
static int vbdev_rout_config_json(struct spdk_json_write_ctx *w);

static struct spdk_bdev_module rout_if = {
	.name = "rout_external",
	.module_init = vbdev_rout_init,
	.get_ctx_size = vbdev_rout_get_ctx_size,
	.examine_config = vbdev_rout_examine,
	.module_fini = vbdev_rout_finish,
	.config_json = vbdev_rout_config_json
};

SPDK_BDEV_MODULE_REGISTER(ext_rout, &rout_if)

/* List of pt_bdev names and their base bdevs via configuration file.
 * Used so we can parse the conf once at init and use this list in examine().
 */
struct bdev_names {
	char			*vbdev_name;
	char			*bdev_name;
	TAILQ_ENTRY(bdev_names)	link;
};
static TAILQ_HEAD(, bdev_names) g_bdev_names = TAILQ_HEAD_INITIALIZER(g_bdev_names);

/* List of virtual bdevs and associated info for each. */
struct vbdev_rout {
	struct spdk_bdev		*base_bdev; /* the thing we're attaching to */
	struct spdk_bdev_desc		*base_desc; /* its descriptor we get from open */
	struct spdk_bdev		pt_bdev;    /* the PT virtual bdev */
	TAILQ_ENTRY(vbdev_rout)	link;
	struct spdk_thread		*thread;    /* thread where base device is opened */
};
static TAILQ_HEAD(, vbdev_rout) g_pt_nodes = TAILQ_HEAD_INITIALIZER(g_pt_nodes);

/* The pt vbdev channel struct. It is allocated and freed on my behalf by the io channel code.
 * If this vbdev needed to implement a poller or a queue for IO, this is where those things
 * would be defined. This rout bdev doesn't actually need to allocate a channel, it could
 * simply pass back the channel of the bdev underneath it but for example purposes we will
 * present its own to the upper layers.
 */
struct pt_io_channel {
	struct spdk_io_channel	*base_ch; /* IO channel of base device */
};

struct bdev_rout_io {
	struct spdk_thread *s_thread;
	enum spdk_bdev_io_status status;
	size_t total_len;
};


/* Just for fun, this pt_bdev module doesn't need it but this is essentially a per IO
 * context that we get handed by the bdev layer.
 */
struct rout_bdev_io {
	uint8_t test;

	/* bdev related */
	struct spdk_io_channel *ch;

	struct spdk_thread *thread;

	/* for bdev_io_wait */
	struct spdk_bdev_io_wait_entry bdev_io_wait;
};

static void
vbdev_rout_submit_request(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io);


/* Callback for unregistering the IO device. */

static void
_device_unregister_cb(void *io_device)
{
	struct vbdev_rout *pt_node  = io_device;

	/* Done with this pt_node. */
	free(pt_node->pt_bdev.name);
	free(pt_node);
}

/* Wrapper for the bdev close operation. */
static void
_vbdev_rout_destruct(void *ctx)
{
	struct spdk_bdev_desc *desc = ctx;

	spdk_bdev_close(desc);
}

/* Called after we've unregistered following a hot remove callback.
 * Our finish entry point will be called next.
 */
static int
vbdev_rout_destruct(void *ctx)
{
	struct vbdev_rout *pt_node = (struct vbdev_rout *)ctx;

	/* It is important to follow this exact sequence of steps for destroying
	 * a vbdev...
	 */

	TAILQ_REMOVE(&g_pt_nodes, pt_node, link);

	/* Unclaim the underlying bdev. */
	spdk_bdev_module_release_bdev(pt_node->base_bdev);

	/* Close the underlying bdev on its same opened thread. */
	if (pt_node->thread && pt_node->thread != spdk_get_thread()) {
		spdk_thread_send_msg(pt_node->thread, _vbdev_rout_destruct, pt_node->base_desc);
	} else {
		spdk_bdev_close(pt_node->base_desc);
	}

	/* Unregister the io_device. */
	spdk_io_device_unregister(pt_node, _device_unregister_cb);

	return 0;
}

/* Completion callback for IO that were issued from this bdev. The original bdev_io
 * is passed in as an arg so we'll complete that one with the appropriate status
 * and then free the one that this module issued.
 */
static void
_pt_complete_io(struct spdk_bdev_io *bdev_io, bool success, void *cb_arg)
{
	struct spdk_bdev_io *orig_io = cb_arg;
	int status = success ? SPDK_BDEV_IO_STATUS_SUCCESS : SPDK_BDEV_IO_STATUS_FAILED;
	struct rout_bdev_io *io_ctx = (struct rout_bdev_io *)orig_io->driver_ctx;
	struct spdk_thread *thread_now = spdk_get_thread();

	/* We setup this value in the submission routine, just showing here that it is
	 * passed back to us.
	 */
	if (io_ctx->test != 0x5a) {
		SPDK_ERRLOG("Error, original IO device_ctx is wrong! 0x%x\n",
			    io_ctx->test);
	}

	/* Complete the original IO and then free the one that we created here
	 * as a result of issuing an IO via submit_request.
	 */
	if(ggd % 10000 == 0){
		//SPDK_NOTICELOG("%d\n",ggd);
	}
	ggd++;
	
	time_t tntt;
	struct tm *lltt;
	struct timeval ttvv;
	tntt = gettimeofday(&ttvv, NULL);
	lltt = localtime(&ttvv.tv_sec);

	//bdev_io->u.nvme_passthru.cdw10 = llt->tm_sec;
	
	if(orig_io->u.nvme_passthru.cmd.opc == 0xC1){
		int timee = 0;
		timee = ttvv.tv_usec - orig_io->u.nvme_passthru.cmd.cdw10;
	//	SPDK_NOTICELOG("%d-%d = %d\n",ttvv.tv_usec,orig_io->u.nvme_passthru.cmd.cdw10, timee);
		if (timee > 0){
			//SPDK_NOTICELOG("1\n");
		switch(thread_now->id){
		case 2:
			//if(++co1 == 200){
			if(++four00 < 300){
				co1 = 0;
				fprintf(pFILE,"%d, %d\n",timee, thread_now->id);
			}
			break;
		case 3:
			if(++four00 < 300){
			//if(++co2 == 200){
				co2 = 0;
				fprintf(pFILE,"%d, %d\n",timee, thread_now->id);
			}
			break;
		case 4:
			if(++four00 < 300){
			//if(++co3 == 200){
				co3 = 0;
				fprintf(pFILE,"%d, %d\n",timee, thread_now->id);
			}
			break;
		case 5:
			if(++four00 < 300){
			//if(++co4 == 200){
				co4 = 0;
				fprintf(pFILE,"%d, %d\n",timee, thread_now->id);
			}
			break;
			//fprintf(pFILE,"%d, %d\n",timee,thread_now->id);
		}
		}
	}
	spdk_bdev_io_complete(orig_io, status);
	spdk_bdev_free_io(bdev_io);
	//SPDK_NOTICELOG("D1. rout back thread:%d\n",thread_now->id);
	//SPDK_NOTICELOG("thread_pt_complete_io:%d\n",thread_now->id);
	//msg_queue_run_batch(thread_now,1);
	//msg_queue_run_batch(thread_now,1);

	//SPDK_NOTICELOG("C1. rout complete opc:0x%x\n",orig_io->u.nvme_passthru.cmd.opc);
}

static void
_c_io(void *ctx)
{
	struct spdk_bdev_io *orig_io = ctx;
	struct spdk_thread *thread = spdk_get_thread();

	if(ggd % 10000 == 0){
	//	SPDK_NOTICELOG("%d\n",ggd);
	}
	ggd++;
	//SPDK_NOTICELOG("_c_io-thread:%d\n",thread->id);
	spdk_bdev_io_complete(orig_io, orig_io->internal.status);
}



static void
_back_complete_io(struct spdk_bdev_io *bdev_io, bool success, void *cb_arg)
{
	struct spdk_bdev_io *orig_io = cb_arg;
	int status = success ? SPDK_BDEV_IO_STATUS_SUCCESS : SPDK_BDEV_IO_STATUS_FAILED;
	struct rout_bdev_io *io_ctx = (struct rout_bdev_io *)orig_io->driver_ctx;
	struct spdk_thread *thread_now = spdk_get_thread();
	struct vbdev_rout *pt_node = SPDK_CONTAINEROF(bdev_io->bdev, struct vbdev_rout, pt_bdev);
		
	if(orig_io->internal.ch != bdev_io->internal.ch){
		struct spdk_thread *thread_b = spdk_bdev_io_get_thread(orig_io);
		//SPDK_NOTICELOG("2. in back complete_thread_now :%d -> thread %d\n",thread_now->id,thread_b->id);
		
		//SPDK_NOTICELOG("fin now thread:%d\n",thread_now->id);
		orig_io->internal.status = status;
		//spdk_bdev_io_complete(orig_io, status);
		spdk_bdev_free_io(bdev_io);
		spdk_thread_send_msg(thread_b,_c_io, orig_io);
	}
	/*
	struct spdk_bdev_channel *bdev_ch = bdev_io->internal.ch;
	struct spdk_io_channel *ch = bdev_ch->channel;*/
	/*
	if (io_ctx->test != 0x5a) {
		SPDK_NOTICELOG("HI!: Error, original IO device_ctx is wrong! 0x%x\n",
				    io_ctx->test);
	}*/
	else{
		//SPDK_NOTICELOG("D1. rout back thread:%d\n",thread_now->id);

	
	//spdk_bdev_free_io(orig_io); // will diff thread free bdev_io that allocated by othr thrd??? do not know
	//SPDK_NOTICELOG("2. rout back thread:%d\n",thread_now->id);
		spdk_bdev_free_io(bdev_io);
	}
	//free(orig_io);
}

static void
_back_io(struct spdk_bdev_io *bdev_io, bool success, void *cb_arg)
{
	char* buff = cb_arg;
	struct spdk_thread *thread = spdk_get_thread();
	//SPDK_NOTICELOG("1. _back_io:%d\n",thread->id);
	if(ggd % 10000 == 0){
		SPDK_NOTICELOG("%d\n",ggd);
	}
	ggd++;
	spdk_bdev_free_io(bdev_io);
}

static void
_pt_complete_zcopy_io(struct spdk_bdev_io *bdev_io, bool success, void *cb_arg)
{
	struct spdk_bdev_io *orig_io = cb_arg;
	int status = success ? SPDK_BDEV_IO_STATUS_SUCCESS : SPDK_BDEV_IO_STATUS_FAILED;
	struct rout_bdev_io *io_ctx = (struct rout_bdev_io *)orig_io->driver_ctx;

	/* We setup this value in the submission routine, just showing here that it is
	 * passed back to us.
	 */
	if (io_ctx->test != 0x5a) {
		SPDK_ERRLOG("Error, original IO device_ctx is wrong! 0x%x\n",
			    io_ctx->test);
	}

	/* Complete the original IO and then free the one that we created here
	 * as a result of issuing an IO via submit_request.
	 */
	spdk_bdev_io_set_buf(orig_io, bdev_io->u.bdev.iovs[0].iov_base, bdev_io->u.bdev.iovs[0].iov_len);
	spdk_bdev_io_complete(orig_io, status);
	spdk_bdev_free_io(bdev_io);
}

static void
vbdev_rout_resubmit_io(void *arg)
{
	struct spdk_bdev_io *bdev_io = (struct spdk_bdev_io *)arg;
	struct rout_bdev_io *io_ctx = (struct rout_bdev_io *)bdev_io->driver_ctx;

	vbdev_rout_submit_request(io_ctx->ch, bdev_io);
}

static void
vbdev_rout_queue_io(struct spdk_bdev_io *bdev_io)
{
	struct rout_bdev_io *io_ctx = (struct rout_bdev_io *)bdev_io->driver_ctx;
	struct pt_io_channel *pt_ch = spdk_io_channel_get_ctx(io_ctx->ch);
	int rc;

	io_ctx->bdev_io_wait.bdev = bdev_io->bdev;
	io_ctx->bdev_io_wait.cb_fn = vbdev_rout_resubmit_io;
	io_ctx->bdev_io_wait.cb_arg = bdev_io;

	/* Queue the IO using the channel of the base device. */
	rc = spdk_bdev_queue_io_wait(bdev_io->bdev, pt_ch->base_ch, &io_ctx->bdev_io_wait);
	if (rc != 0) {
		SPDK_ERRLOG("Queue io failed in vbdev_rout_queue_io, rc=%d.\n", rc);
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
	}
}

/* Callback for getting a buf from the bdev pool in the event that the caller passed
 * in NULL, we need to own the buffer so it doesn't get freed by another vbdev module
 * beneath us before we're done with it. That won't happen in this example but it could
 * if this example were used as a template for something more complex.
 */
static void
pt_read_get_buf_cb(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io, bool success)
{
	struct vbdev_rout *pt_node = SPDK_CONTAINEROF(bdev_io->bdev, struct vbdev_rout,
					 pt_bdev);
	struct pt_io_channel *pt_ch = spdk_io_channel_get_ctx(ch);
	struct rout_bdev_io *io_ctx = (struct rout_bdev_io *)bdev_io->driver_ctx;
	int rc;

	if (!success) {
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
		return;
	}

	if (bdev_io->u.bdev.md_buf == NULL) {
		rc = spdk_bdev_readv_blocks(pt_node->base_desc, pt_ch->base_ch, bdev_io->u.bdev.iovs,
					    bdev_io->u.bdev.iovcnt, bdev_io->u.bdev.offset_blocks,
					    bdev_io->u.bdev.num_blocks, _pt_complete_io,
					    bdev_io);
	} else {
		rc = spdk_bdev_readv_blocks_with_md(pt_node->base_desc, pt_ch->base_ch,
						    bdev_io->u.bdev.iovs, bdev_io->u.bdev.iovcnt,
						    bdev_io->u.bdev.md_buf,
						    bdev_io->u.bdev.offset_blocks,
						    bdev_io->u.bdev.num_blocks,
						    _pt_complete_io, bdev_io);
	}

	if (rc != 0) {
		if (rc == -ENOMEM) {
			SPDK_ERRLOG("No memory, start to queue io for rout.\n");
			io_ctx->ch = ch;
			vbdev_rout_queue_io(bdev_io);
		} else {
			SPDK_ERRLOG("ERROR on bdev_io submission!\n");
			spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
		}
	}
}
static void
rout_submit_request(void *ctx){
	struct spdk_bdev_io *bdev_io = ctx;
	struct vbdev_rout *pt_node = SPDK_CONTAINEROF(bdev_io->bdev, struct vbdev_rout, pt_bdev);
	struct spdk_io_channel *ch;
	struct spdk_thread *thread = spdk_get_thread();
	switch(thread->id){
		case 1:
			ch = ch1;
			break;
		case 2:
			ch = ch2;
			break;
		case 3:
			ch = ch3;
			break;
		case 4:
			ch = ch4;
			break;
		case 5:
			ch = ch5;
			break;
		case 6:
			ch = ch6;
			break;
		case 7:
			ch = ch7;
			break;
		case 8:
			ch = ch8;
			break;
		case 9:
			ch = ch9;
			break;
		case 10:
			ch = ch10;
			break;
	}
	/*
	if(ch_all->thread->id == thread->id){
		ch = ch_all;
	}
	else if(ch_all2->thread->id == thread->id){
		ch = ch_all2;
	}
	else if(ch_all3->thread->id == thread->id){
		ch = ch_all3;
	}*/
	struct pt_io_channel *pt_ch = spdk_io_channel_get_ctx(ch);
	struct spdk_bdev_channel *channel = spdk_io_channel_get_ctx(ch);	

	//SPDK_NOTICELOG("rout_sub:%d\n",ch->thread->id);
	
	//bdev_io->internal.ch = channel;
	//struct rout_bdev_io *io_ctx2 = (struct rout_bdev_io *)bdev_io->driver_ctx;
	//io_ctx2->test = 0x5a;
	spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
		bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _back_complete_io, bdev_io);
}

static void
rout_submit_request2(int ctx){
	
	//struct spdk_bdev_io *bdev_io = ctx;
	int id = ctx;
	//struct vbdev_rout *pt_node = SPDK_CONTAINEROF(bdev_io->bdev, struct vbdev_rout, pt_bdev);
	
	struct spdk_io_channel *ch;
	struct spdk_thread *thread = spdk_get_thread();
	switch(thread->id){
		case 1:
			ch = ch1;
			break;
		case 2:
			ch = ch2;
			break;
		case 3:
			ch = ch3;
			break;
		case 4:
			ch = ch4;
			break;
		case 5:
			ch = ch5;
			break;
		case 6:
			ch = ch6;
			break;
		case 7:
			ch = ch7;
			break;
		case 8:
			ch = ch8;
			break;
		case 9:
			ch = ch9;
			break;
		case 10:
			ch = ch10;
			break;
	}
	/*
	if(ch_all->thread->id == thread->id){
		ch = ch_all;
	}
	else if(ch_all2->thread->id == thread->id){
		ch = ch_all2;
	}
	else if(ch_all3->thread->id == thread->id){
		ch = ch_all3;
	}*/
	//SPDK_NOTICELOG("thread:%d\n",ch->thread->id);
	struct pt_io_channel *pt_ch = spdk_io_channel_get_ctx(ch);
	struct spdk_bdev_channel *channel = spdk_io_channel_get_ctx(ch);	

//	SPDK_NOTICELOG("rout_sub2\n");
	uint32_t blk_size, buf_align;
	blk_size = 512;
	if(buc == 0){
		buc++;
		buff = spdk_dma_zmalloc(512,512,NULL);
		snprintf(buff, blk_size, "%s", "Hello World\n");
	}
	
	spdk_bdev_nvme_io_passthru2(bdev_desc, pt_ch->base_ch, 
		buff, 1024, _back_io, buff);
}

/* Called when someone above submits IO to this pt vbdev. We're simply passing it on here
 * via SPDK IO calls which in turn allocate another bdev IO and call our cpl callback provided
 * below along with the original bdev_io so that we can complete it once this IO completes.
 */
static void
vbdev_rout_submit_request(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	struct vbdev_rout *pt_node = SPDK_CONTAINEROF(bdev_io->bdev, struct vbdev_rout, pt_bdev);
	struct pt_io_channel *pt_ch = spdk_io_channel_get_ctx(ch);
	struct pt_io_channel *pt_ch2;
	struct rout_bdev_io *io_ctx = (struct rout_bdev_io *)bdev_io->driver_ctx;
	struct spdk_bdev_io *bdev_io_D;
	int rc = 0;

	io_ctx->test = 0x5a;
	io_ctx->ch = ch;

	time_t t;
	struct tm *lt;
	struct timeval tv;

	if (filefile == 0){
		filefile = 1;
		pFILE = fopen("log2.txt","w");
	}
	
	/* Setup a per IO context value; we don't do anything with it in the vbdev other
	 * than confirm we get the same thing back in the completion callback just to
	 * demonstrate.
	 */
	
	bdev_desc = pt_node->base_desc;
	/* for background I/O 여기 해제해야*/
	switch (ch->thread->id){
		case 1:
			if(th1 == NULL){
				th1 = ch->thread;
				ch1 = ch;
				SPDK_NOTICELOG("th1:%d\n",th1->id);
			}
			break;
		case 2:
			if(th2 == NULL){
				th2 = ch->thread;
				ch2 = ch;
				ch_b = ch;
				SPDK_NOTICELOG("2222:th2:%d,ch_b:%d\n",th2->id,ch_b->thread->id);
			}
			break;
		case 3:
			if(th3 == NULL){
				th3 = ch->thread;
				ch3 = ch;
				if(ch_b == NULL){
					ch_b = ch;
				}
				else if(ch_b->thread->id > ch->thread->id)
					ch_b = ch;
				SPDK_NOTICELOG("3333:th3:%d,ch_b:%d\n",th3->id,ch_b->thread->id);
			}
			break;
		case 4:
			if(th4 == NULL){
				th4 = ch->thread;
				ch4 = ch;
				if(ch_b == NULL){
					ch_b = ch;
				}
				else if(ch_b->thread->id > ch->thread->id)
					ch_b = ch;
				SPDK_NOTICELOG("4444:th4:%d,ch_b:%d\n",th4->id,ch_b->thread->id);
			}
			break;
		case 5:
			if(th5 == NULL){
				th5 = ch->thread;
				ch5 = ch;
				if(ch_b == NULL){
					ch_b = ch;
				}
				else if(ch_b->thread->id > ch->thread->id)
					ch_b = ch;
				SPDK_NOTICELOG("5555:th5:%d,ch_b:%d\n",th5->id,ch_b->thread->id);
			}
			break;
		case 6:
			if(th6 == NULL){
				th6 = ch->thread;
				ch6 = ch;
				if(ch_b == NULL){
					ch_b = ch;
				}
				else if(ch_b->thread->id > ch->thread->id)
					ch_b = ch;
				SPDK_NOTICELOG("6666:th6:%d,ch_b:%d\n",th6->id,ch_b->thread->id);
			}
			break;
		case 7:
			if(th7 == NULL){
				th7 = ch->thread;
				ch7 = ch;
				if(ch_b == NULL){
					ch_b = ch;
				}
				else if(ch_b->thread->id > ch->thread->id)
					ch_b = ch;
				SPDK_NOTICELOG("7777:th7:%d,ch_b:%d\n",th7->id,ch_b->thread->id);
			}
			break;
		case 8:
			if(th8 == NULL){
				th8 = ch->thread;
				ch8 = ch;
				if(ch_b == NULL){
					ch_b = ch;
				}
				else if(ch_b->thread->id > ch->thread->id)
					ch_b = ch;
				SPDK_NOTICELOG("8888:th8:%d,ch_b:%d\n",th8->id,ch_b->thread->id);
			}
			break;
		case 9:
			if(th9 == NULL){
				th9 = ch->thread;
				ch9 = ch;
				if(ch_b == NULL){
					ch_b = ch;
				}
				else if(ch_b->thread->id > ch->thread->id)
					ch_b = ch;
				SPDK_NOTICELOG("9999:th9:%d,ch_b:%d\n",th9->id,ch_b->thread->id);
			}
			break;
		case 10:
			if(th10 == NULL){
				th10 = ch->thread;
				ch10 = ch;
				if(ch_b == NULL){
					ch_b = ch;
				}
				else if(ch_b->thread->id > ch->thread->id)
					ch_b = ch;

				SPDK_NOTICELOG("101010:th10:%d,ch_b:%d\n",th10->id,ch_b->thread->id);
			}
			break;
		default:
			if(th11 == NULL){
				th11 = ch->thread;
				ch11 = ch;
				if(ch_b == NULL){
					ch_b = ch;
				}
				else if(ch_b->thread->id > ch->thread->id)
					ch_b = ch;				
				SPDK_NOTICELOG("111111:else th11:%d,ch_b:%d\n",th11->id,ch_b->thread->id);
			}
			break;
	}
	/*
	if(ch_b != NULL){
		SPDK_NOTICELOG("ch_b:%d\n",ch_b->thread->id);
	}*/

	switch (bdev_io->type) {
	case SPDK_BDEV_IO_TYPE_READ:
		spdk_bdev_io_get_buf(bdev_io, pt_read_get_buf_cb,
				     bdev_io->u.bdev.num_blocks * bdev_io->bdev->blocklen);
		break;
	case SPDK_BDEV_IO_TYPE_WRITE:
		if (bdev_io->u.bdev.md_buf == NULL) {
			rc = spdk_bdev_writev_blocks(pt_node->base_desc, pt_ch->base_ch, bdev_io->u.bdev.iovs,
						     bdev_io->u.bdev.iovcnt, bdev_io->u.bdev.offset_blocks,
						     bdev_io->u.bdev.num_blocks, _pt_complete_io,
						     bdev_io);
		} else {
			rc = spdk_bdev_writev_blocks_with_md(pt_node->base_desc, pt_ch->base_ch,
							     bdev_io->u.bdev.iovs, bdev_io->u.bdev.iovcnt,
							     bdev_io->u.bdev.md_buf,
							     bdev_io->u.bdev.offset_blocks,
							     bdev_io->u.bdev.num_blocks,
							     _pt_complete_io, bdev_io);
		}
		break;
	case SPDK_BDEV_IO_TYPE_WRITE_ZEROES:
		rc = spdk_bdev_write_zeroes_blocks(pt_node->base_desc, pt_ch->base_ch,
						   bdev_io->u.bdev.offset_blocks,
						   bdev_io->u.bdev.num_blocks,
						   _pt_complete_io, bdev_io);
		break;
	case SPDK_BDEV_IO_TYPE_UNMAP:
		rc = spdk_bdev_unmap_blocks(pt_node->base_desc, pt_ch->base_ch,
					    bdev_io->u.bdev.offset_blocks,
					    bdev_io->u.bdev.num_blocks,
					    _pt_complete_io, bdev_io);
		break;
	case SPDK_BDEV_IO_TYPE_FLUSH:
		rc = spdk_bdev_flush_blocks(pt_node->base_desc, pt_ch->base_ch,
					    bdev_io->u.bdev.offset_blocks,
					    bdev_io->u.bdev.num_blocks,
					    _pt_complete_io, bdev_io);
		break;
	case SPDK_BDEV_IO_TYPE_RESET:
		rc = spdk_bdev_reset(pt_node->base_desc, pt_ch->base_ch,
				     _pt_complete_io, bdev_io);
		break;
	case SPDK_BDEV_IO_TYPE_ZCOPY:
		rc = spdk_bdev_zcopy_start(pt_node->base_desc, pt_ch->base_ch, bdev_io->u.bdev.iovs,
					   bdev_io->u.bdev.iovcnt, bdev_io->u.bdev.offset_blocks,
					   bdev_io->u.bdev.num_blocks, bdev_io->u.bdev.zcopy.populate,
					   _pt_complete_zcopy_io, bdev_io);
		break;
	case SPDK_BDEV_IO_TYPE_ABORT:
		rc = spdk_bdev_abort(pt_node->base_desc, pt_ch->base_ch, bdev_io->u.abort.bio_to_abort,
				     _pt_complete_io, bdev_io);
		break;
	case SPDK_BDEV_IO_TYPE_NVME_IO:
		if(bdev_io->u.nvme_passthru.cmd.opc==0xC2){
			SPDK_NOTICELOG("in rout\n");
			break;
		}
		if(bdev_io->u.nvme_passthru.cmd.opc==0xC1){
			//fputs(,pFILE);
			//tot++;
			//SPDK_NOTICELOG("sta\n");
				
			time_t tnt;
			struct tm *llt;
			struct timeval ttv;
			tnt = gettimeofday(&ttv, NULL);
			llt = localtime(&ttv.tv_sec);
			if (llt->tm_sec - sec >= 1){
				sec = llt->tm_sec;
				four00 = 0;
				if(sec == 60){
					sec = 0;
				}
				fprintf(pFILE,"\n\n");
			}
			//SPDK_NOTICELOG("time:%02d.%06d\n",llt->tm_sec, ttv.tv_usec);
			bdev_io->u.nvme_passthru.cmd.cdw10 = ttv.tv_usec;
			//SPDK_NOTICELOG(":%06d\n",bdev_io->u.nvme_passthru.cmd.cdw10);
	//printf("sta : %06d\n",tv.tv_usec);
	//printf("sta시간 : %04d-%02d-%02d %02d:%02d:%02d.%06d\n",
	//		lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
	//		lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec);

			int te = 0;
			/* for background I/O 지금 어떤 thread인지 기록 여기 해제해야함.

			if(ga == 0){
				nc1 = ch;
				nc2 = ch;
				nc3 = ch;
				nc4 = ch;
				nc5 = ch;
				be1 = ch->thread;
				be2 = ch->thread;
				be3 = ch->thread;
				be4 = ch->thread;
				be5 = ch->thread;
				ga = 1;
			}I/ 
			//SPDK_NOTICELOG("1. nc1:%d,nc2:%d,nc3:%d,ch:%d\n",nc1->thread->id, nc2->thread->id, nc3->thread->id,ch->thread->id);

			for background I/O 2번 해제할거*여기도 해제해야
			if(ch->thread->id != nc1->thread->id && ch->thread->id != nc2->thread->id && ch->thread->id != nc3->thread->id){
					//&& nc4->thread->id != ch->thread->id){
			if(nnn % 3 == 0 ){
				nnn++;
					nc1 = ch;
			}
			else if(nnn %3 == 1){
				nnn++;
					nc2 = ch;
			}
			else{
				nnn++;
					nc3 = ch;
			}
			}*/
			
			/*for background 여기 해제해야함
			nc5 = nc4;
			nc4 = nc3;
			nc3 = nc2;
			nc2 = nc1;
			nc1 = ch;*/
			//t = gettimeofday(&tv, NULL);

			//lt = localtime(&tv.tv_sec);
			//printf("fin : %06d\n",tv.tv_usec);

			//SPDK_NOTICELOG("fin\n");
			//printf("fin시간 : %04d-%02d-%02d %02d:%02d:%02d.%06d\n",
			//lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
			//lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec);
			/*사용중인 쓰레드 두개 중 하나가 바뀌었으면 변경 요청 
			if(ch->thread->id != nc1->thread->id && ch->thread->id != nc2->thread->id){
				if(nnn % 2 == 0){
					nnn++;
					nc1 = ch;
				}
				else{
					nnn++;
					nc2 = ch;
				}
			}*/
			/* thread 1개일 때 
			if(ch->thread->id != nc1->thread->id){
				nc1 = ch;
			}*/

			
			//SPDK_NOTICELOG("2. nc1:%d,nc2:%d,nc3:%d,ch:%d\n",nc1->thread->id, nc2->thread->id, nc3->thread->id,ch->thread->id);

			if(nnn > 30001){
				nnn = 0;
			}
			/*	
			if(ch2 != NULL){
				//SPDK_NOTICELOG("ch2:%d\n",ch->thread->id);
				ch_b = ch2;
			}
			else if(ch3 != NULL){
				ch_b = ch3;
				//SPDK_NOTICELOG("ch2:%d,ch_b:%d\n",ch->thread->id,ch_b->thread->id);
			}
			else if(ch4 != NULL){
				ch_b = ch4;
			}
			else if(ch5 != NULL){
				ch_b = ch5;
			}
			else if(ch6 != NULL){
				ch_b = ch6;
			}
			else{
				ch_b = ch;
			}*/
			/***************************** 백그라운드용 반드시 여기 해제하기**/
			/*****background I/O
				
			
			switch(ch_b->thread->id){
				case 2:
					//if(th2 != NULL){
					//if(th2 != NULL && be1 != th2 && be2 != th2){
					//if(th2 != NULL && be1 != th2 && be2 != th2 && be3 != th2 && be4 != th2){
					if(th2 != NULL && be1 != th2){
						//if(nc1->thread->id != th2->id){ 
						//if(nc1->thread->id != th2->id && nc2->thread->id != th2->id){ 
						//if(nc1->thread->id != th2->id && nc2->thread->id != th2->id && nc3->thread->id != th2->id){ 
						if(nc1->thread->id != th2->id && nc2->thread->id != th2->id && nc3->thread->id != th2->id && nc4->thread->id != th2->id){ 
						//be4 = be3;
						//be3 = be2;
						//be2 = be1;
						be1 = th2;
						spdk_thread_send_msg(th2,rout_submit_request2, th2->id);
						break;
						}
					}
				case 3:
					//if(th3 != NULL){
					if(th3 != NULL && be1 != th3){
					//if(th3 != NULL && be1 != th3 && be2 != th3){
					//if(th3 != NULL && be1 != th3 && be2 != th3 && be3 != th3 && be4 != th3){
						//if(nc1->thread->id != th3->id){ 
						//if(nc1->thread->id != th3->id && nc2->thread->id != th3->id){ 
						//if(nc1->thread->id != th3->id && nc2->thread->id != th3->id && nc3->thread->id != th3->id){ 
						if(nc1->thread->id != th3->id && nc2->thread->id != th3->id && nc3->thread->id != th3->id && nc4->thread->id != th3->id){ 
						//be4 = be3;
						//be3 = be2;
						//be2 = be1;
						be1 = th3;
						spdk_thread_send_msg(th3,rout_submit_request2, th3->id);
						break;
						}
					}
				case 4:
					//if(th4 != NULL){
					if(th4 != NULL && be1 != th4){
					//if(th4 != NULL && be1 != th4 && be2 != th4){
					//if(th4 != NULL && be1 != th4 && be2 != th4 && be3 != th4 && be4 != th4){
						//if(nc1->thread->id != th4->id){ 
						//if(nc1->thread->id != th4->id && nc2->thread->id != th4->id){ 
						//if(nc1->thread->id != th4->id && nc2->thread->id != th4->id && nc3->thread->id != th4->id){ 
						if(nc1->thread->id != th4->id && nc2->thread->id != th4->id && nc3->thread->id != th4->id && nc4->thread->id != th4->id){ 
						//be4 = be3;
						//be3 = be2;
						//be2 = be1;
						be1 = th4;
						spdk_thread_send_msg(th4,rout_submit_request2, th4->id);
						break;
						}
					}
				case 5:
					//if(th5 != NULL){
					if(th5 != NULL && be1 != th5){
					//if(th5 != NULL && be1 != th5 && be2 != th5){
					//if(th5 != NULL && be1 != th5 && be2 != th5 && be3 != th5 && be4 != th5){
						//if(nc1->thread->id != th5->id){ 
						//if(nc1->thread->id != th5->id && nc2->thread->id != th5->id){ 
						//if(nc1->thread->id != th5->id && nc2->thread->id != th5->id && nc3->thread->id != th5->id){ 
						if(nc1->thread->id != th5->id && nc2->thread->id != th5->id && nc3->thread->id != th5->id && nc4->thread->id != th5->id){ 
						//be4 = be3;
						//be3 = be2;
						//be2 = be1;
						be1 = th5;
						spdk_thread_send_msg(th5,rout_submit_request2, th5->id);
						break;
						}
					}
				case 6:
					//if(th6 != NULL){
					if(th6 != NULL && be1 != th6){
					//if(th6 != NULL && be1 != th6 && be2 != th6){
					//if(th6 != NULL && be1 != th6 && be2 != th6 && be3 != th6 && be4 != th6){
						//if(nc1->thread->id != th6->id){ 
						//if(nc1->thread->id != th6->id && nc2->thread->id != th6->id){ 
						//if(nc1->thread->id != th6->id && nc2->thread->id != th6->id && nc3->thread->id != th6->id){ 
						if(nc1->thread->id != th6->id && nc2->thread->id != th6->id && nc3->thread->id != th6->id && nc4->thread->id != th6->id){ 
						//be4 = be3;
						//be3 = be2;
						//be2 = be1;
						be1 = th6;
						spdk_thread_send_msg(th6,rout_submit_request2, th6->id);
						break;
						}
					}
				case 7:
					//if(th7 != NULL){
					if(th7 != NULL && be1 != th7){
					//if(th7 != NULL && be1 != th7 && be2 != th7){
					//if(th7 != NULL && be1 != th7 && be2 != th7 && be3 != th7 && be4 != th7){
						//if(nc1->thread->id != th7->id){ 
						//if(nc1->thread->id != th7->id && nc2->thread->id != th7->id){ 
						//if(nc1->thread->id != th7->id && nc2->thread->id != th7->id && nc3->thread->id != th7->id){ 
						if(nc1->thread->id != th7->id && nc2->thread->id != th7->id && nc3->thread->id != th7->id && nc4->thread->id != th7->id){ 
						//be4 = be3;
						//be3 = be2;
						//be2 = be1;
						be1 = th7;
						spdk_thread_send_msg(th7,rout_submit_request2, th7->id);
						break;
						}
					}
				case 8:
					//if(th8 != NULL){
					if(th8 != NULL && be1 != th8){
					//if(th8 != NULL && be1 != th8 && be2 != th8){
					//if(th8 != NULL && be1 != th8 && be2 != th8 && be3 != th8 && be4 != th8){
						//if(nc1->thread->id != th8->id){ 
						//if(nc1->thread->id != th8->id && nc2->thread->id != th8->id){ 
						//if(nc1->thread->id != th8->id && nc2->thread->id != th8->id && nc3->thread->id != th8->id){ 
						if(nc1->thread->id != th8->id && nc2->thread->id != th8->id && nc3->thread->id != th8->id && nc4->thread->id != th8->id){ 
						//be4 = be3;
						//be3 = be2;
						//be2 = be1;
						be1 = th8;
						spdk_thread_send_msg(th8,rout_submit_request2, th8->id);
						break;
						}
					}
				case 9:
					//if(th9 != NULL){
					if(th9 != NULL && be1 != th9){
					//if(th9 != NULL && be1 != th9 && be2 != th9){
					//if(th9 != NULL && be1 != th9 && be2 != th9 && be3 != th9 && be4 != th9){
						//if(nc1->thread->id != th9->id){ 
						//if(nc1->thread->id != th9->id && nc2->thread->id != th9->id){ 
						//if(nc1->thread->id != th9->id && nc2->thread->id != th9->id && nc3->thread->id != th9->id){ 
						if(nc1->thread->id != th9->id && nc2->thread->id != th9->id && nc3->thread->id != th9->id && nc4->thread->id != th9->id){ 
					//	be4 = be3;
					//	be3 = be2;
					//	be2 = be1;
						be1 = th9;
						spdk_thread_send_msg(th9,rout_submit_request2, th9->id);
						break;
						}
					}
				case 10:
					//if(th10 != NULL){
					if(th10 != NULL && be1 != th10){
					//if(th10 != NULL && be1 != th10 && be2 != th10){
				//	if(th10 != NULL && be1 != th10 && be2 != th10 && be3 != th10 && be4 != th10){
						if(nc1->thread->id != th10->id && nc2->thread->id != th10->id && nc3->thread->id != th10->id && nc4->thread->id != th10->id){ 
					//	be4 = be3;
					//	be3 = be2;
					//	be2 = be1;
						be1 = th10;
						spdk_thread_send_msg(th10,rout_submit_request2, th10->id);
						break;
						}
					}
			}*/
			/*
			switch(ch_b->thread->id){
				case 2:
					//SPDK_NOTICELOG("case 2\n");
					if(th2 != NULL && be1 != th2 && be2 != th2 && be3 != th2 ){
							//&& be4 != th2){
							//&& be5 != th2){
						if(nc1->thread->id != th2->id && nc2->thread->id != th2->id && nc3->thread->id != th2->id){ 
								//&& nc4->thread->id != th2->id){
						//SPDK_NOTICELOG("1. back2_send:%d->%d __ size:%d\n",ch->thread->id, th2->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						//ch_b = th2;
				//		be5 = be4;
				//		be4 = be3;
						be3 = be2;
						be2 = be1;
						be1 = th2;
						spdk_thread_send_msg(th2, rout_submit_request2, th2->id);
						break;
						}
					}
				case 3:
					//SPDK_NOTICELOG("case 3\n");
					if(th3 != NULL && be1 != th3 && be2 != th3 && be3 != th3 ){
							//&& be4 != th3){
							//&& be5 != th3){
						if(nc1->thread->id != th3->id && nc2->thread->id != th3->id && nc3->thread->id != th3->id){
								//&& nc4->thread->id != th3->id){
						//SPDK_NOTICELOG("1. back3_send:%d->%d __ size:%d\n",ch->thread->id, th3->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						//ch_b = th3;
					//	be5 = be4;
					//	be4 = be3;
						be3 = be2;
						be2 = be1;
						be1 = th3;
						spdk_thread_send_msg(th3, rout_submit_request2, th3->id);
						break;
						}
					}
				case 4:
					//SPDK_NOTICELOG("case 4\n");
					if(th4 != NULL && be1 != th4 && be2 != th4 && be3 != th4){
							//&& be4 != th4){
					  //&& be5 != th4){
						if(nc1->thread->id != th4->id && nc2->thread->id != th4->id && nc3->thread->id != th4->id){ 
								//&& nc4->thread->id != th4->id){
						//SPDK_NOTICELOG("1. back4_send:%d->%d __ size:%d\n",ch->thread->id, th4->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						//ch_b = th4;
					//	be5 = be4;
					//	be4 = be3;
						be3 = be2;
						be2 = be1;
						be1 = th4;
						spdk_thread_send_msg(th4, rout_submit_request2, th4->id);
						break;
						}
					}
				case 5:
				//	SPDK_NOTICELOG("case 5\n");
					if(th5 != NULL && be1 != th5 && be2 != th5 && be3 != th5 ){
							//&& be4 != th5){
							//&& be5 != th5){
						if(nc1->thread->id != th5->id && nc2->thread->id != th5->id && nc3->thread->id != th5->id){ 
								//&& nc4->thread->id != th5->id){
					//	SPDK_NOTICELOG("1. back5_send:%d->%d __ size:%d\n",ch->thread->id, th5->id, bdev_io->u.nvme_passthru.cmd.cdw12);
					//	ch_b = th5;
					//	be5 = be4;
					//	be4 = be3;
						be3 = be2;
						be2 = be1;
						be1 = th5;
						spdk_thread_send_msg(th5, rout_submit_request2, th5->id);
						break;
						}
					}
				case 6:
				//	SPDK_NOTICELOG("case 6\n");
					if(th6 != NULL && be1 != th6 && be2 != th6 && be3 != th6 ){
							//&& be4 != th6){
						//	&& be5 != th6){
						if(nc1->thread->id != th6->id && nc2->thread->id != th6->id && nc3->thread->id != th6->id){ 
								//&& nc4->thread->id != th6->id){
				//		SPDK_NOTICELOG("1. back6_send:%d->%d __ size:%d\n",ch->thread->id, th6->id, bdev_io->u.nvme_passthru.cmd.cdw12);
					//	ch_b = th6;
					//	be5 = be4;
					//	be4 = be3;
						be3 = be2;
						be2 = be1;
						be1 = th6;
						spdk_thread_send_msg(th6, rout_submit_request2, th6->id);
						break;
						}
					}
				case 7:
				//	SPDK_NOTICELOG("case 7\n");
					if(th7 != NULL && be1 != th7 && be2 != th7 && be3 != th7 ){
							//&& be4 != th7){
						//	&& be5 != th7){
						if(nc1->thread->id != th7->id && nc2->thread->id != th7->id && nc3->thread->id != th7->id){
								//&& nc4->thread->id != th7->id){
				///		SPDK_NOTICELOG("1. back7_send:%d->%d __ size:%d\n",ch->thread->id, th7->id, bdev_io->u.nvme_passthru.cmd.cdw12);
					//	ch_b = th7;
					//	be5 = be4;
					//	be4 = be3;
						be3 = be2;
						be2 = be1;
						be1 = th7;
						spdk_thread_send_msg(th7, rout_submit_request2, th7->id);
						break;
						}
					}
				case 8:
				//	SPDK_NOTICELOG("case 8\n");
					if(th8 != NULL && be1 != th8 && be2 != th8 && be3 != th8 && be4 != th8){
						//	&& be5 != th8){
						if(nc1->thread->id != th8->id && nc2->thread->id != th8->id && nc3->thread->id != th8->id && nc4->thread->id != th8->id){
				//		SPDK_NOTICELOG("1. back8_send:%d->%d __ size:%d\n",ch->thread->id, th8->id, bdev_io->u.nvme_passthru.cmd.cdw12);
					//	ch_b = th8;
					//	be5 = be4;
						be4 = be3;
						be3 = be2;
						be2 = be1;
						be1 = th8;
						spdk_thread_send_msg(th8, rout_submit_request2, th8->id);
						break;
						}
					}
				case 9:
				//	SPDK_NOTICELOG("case 9\n");
					if(th9 != NULL && be1 != th9 && be2 != th9 && be3 != th9 ){
							//&& be4 != th9 ){
						//	&& be5 != th9){
						if(nc1->thread->id != th9->id && nc2->thread->id != th9->id && nc3->thread->id != th9->id){
							//&& nc4->thread->id != th9->id){
					//	SPDK_NOTICELOG("1. back9_send:%d->%d __ size:%d\n",ch->thread->id, th9->id, bdev_io->u.nvme_passthru.cmd.cdw12);
				//		ch_b = th9;
				//		be5 = be4;
				//		be4 = be3;
						be3 = be2;
						be2 = be1;
						be1 = th9;
						spdk_thread_send_msg(th9, rout_submit_request2, th9->id);
						break;
						}
					}
				default:
			//		SPDK_NOTICELOG("case 10\n");
					if(th10 != NULL){
			//		if(th10 != NULL && be1 != th10 && be2 != th10 && be3 != th10){
							//&& be4 != th10){
						//	&& be5 != th10 ){
			//			if(nc1->thread->id != th10->id && nc2->thread->id != th10->id && nc3->thread->id != th10->id ){
								//&& nc4->thread->id != th10->id){
				//		SPDK_NOTICELOG("1. back10_send:%d->%d __ size:%d\n",ch->thread->id, th10->id, bdev_io->u.nvme_passthru.cmd.cdw12);
					//	ch_b = th10;
				//		be5 = be4;
				//		be4 = be3;
						be3 = be2;
						be2 = be1;
						be1 = th10;
						spdk_thread_send_msg(th10, rout_submit_request2, th10->id);
						break;
			//			}
			//		}
					}
			}*/
			/*******************/
			/***********************************/
			/*****foreground I/O*****/
			struct spdk_thread *thread_now = spdk_get_thread();
			//	SPDK_NOTICELOG("2. no move:thread:%d:size-%d\n",thread_now->id,bdev_io->u.nvme_passthru.cmd.cdw12);				
			//가장 기본값은 여기 해제!!!!!!!!!!!!!
				
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
				bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			/****background 같은 CPU에서 실행**/
			/*********************
			
			uint32_t blk_size, buf_align;
			blk_size = 512;
			if(buc == 0){
				buc++;
				buff = spdk_dma_zmalloc(512,512,NULL);
				snprintf(buff, blk_size, "%s", "Hello World\n");
			}
			spdk_bdev_nvme_io_passthru2(bdev_desc, pt_ch->base_ch, buff, 1024, _back_io, buff);
			*/
			//SPDK_NOTICELOG("now th_id:%d\n",ch->thread->id);
			int pone = zeone1;

			////////////////////
			////////////////////
			////////////////////
			/*background I/O를 6, 7, 8에 몰아
			if(th6 != NULL && th7 != NULL && th8 != NULL){
			//if(th2 != NULL && ch->thread->id != 3 && ch->thread->id != 2 && ch->thread->id == 4  && pone == 0){
			if(ch->thread->id == 6 || ch->thread->id == 7 || ch->thread->id == 8){
			//	SPDK_NOTICELOG("thread:%d\n",ch->thread->id);
			uint32_t blk_size, buf_align;
			blk_size = 512;
			if(buc == 0){
				buc++;
				buff = spdk_dma_zmalloc(512,512,NULL);
				snprintf(buff, blk_size, "%s", "Hello World\n");
			}
			spdk_bdev_nvme_io_passthru2(bdev_desc, pt_ch->base_ch, buff, 1024, _back_io, buff);
			}
			else if(ch->thread->id != 6 && ch->thread->id != 7 && ch->thread->id != 8  && pone == 0){
					//SPDK_NOTICELOG("p:%d, thread %d->send to th6\n",pone, ch->thread->id);
					spdk_thread_send_msg(th6, rout_submit_request2, th6->id);
				zeone1=1;
			}
			//else if (th3 != NULL && ch->thread->id != 2 && ch->thread->id != 3 && ch->thread->id != 4 && pone == 1){
			else if (ch->thread->id != 6 && ch->thread->id != 7 && ch->thread->id != 8 && pone == 1){
					//SPDK_NOTICELOG("p:%d, thread %d->send to th7\n",pone, ch->thread->id);
					spdk_thread_send_msg(th7, rout_submit_request2, th7->id);
				zeone1=2;
			}
			//else if (th4 !=NULL && ch->thread->id != 3 && ch->thread->id != 2 && ch->thread->id != 4 &&	pone == 2){
			//else if (ch->thread->id != 3 && ch->thread->id != 2 && ch->thread->id != 4 &&	pone == 2){
			else{

					//SPDK_NOTICELOG("#### p:%d, thread %d->send to th8\n",pone, ch->thread->id);
					spdk_thread_send_msg(th8, rout_submit_request2, th8->id);
					zeone1=0;
			}
			}

			else{
				SPDK_NOTICELOG("Bout_____p:%d,now th_id:%d\n",pone,ch->thread->id);
				//spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
				//		bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);

			}*/
			
			/* 4096은 여기서 처리 */
			//if(ch->thread->id == 1|| ch->thread->id == 2 || ch->thread->id == 3){
			//SPDK_NOTICELOG("%d\n",ch->thread->id);
			//}
			//SPDK_NOTICELOG("%d\n",bdev_io->u.nvme_passthru.cmd.cdw12);
			
			///////////////////1. 16384B 데이터를 th5에서 처리하는 방법////////////////////
			/*		
			if(bdev_io->u.nvme_passthru.cmd.cdw12 == 2){
			
			if(th6 != NULL){
				if(ch->thread->id == 6){
				//	SPDK_NOTICELOG("thread:%d\n",ch->thread->id);
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
				}
				else{
				//	SPDK_NOTICELOG("thread:%d->7\n",ch->thread->id);
					spdk_thread_send_msg(th6,rout_submit_request,bdev_io);
				}
			}
			else{
			//	SPDK_NOTICELOG("thread:%d->7\n",ch->thread->id);
					//spdk_thread_send_msg(th3,rout_submit_request,bdev_io);
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}
			}
			else{
				//SPDK_NOTICELOG("else\n");
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}*/
			//////////////*********************************************///////////////////
			

			///////////////////2. 16384B 데이터를 th5,th6에서 처리하는 방법//////////////////// 
			/*	
			if(bdev_io->u.nvme_passthru.cmd.cdw12 == 3){
			if(th3 != NULL && ch->thread->id == 3){
			if(th5 != NULL && th6 != NULL){
				if(ch->thread->id == 5 || ch->thread->id == 6){
				//	SPDK_NOTICELOG("thread:%d\n",ch->thread->id);
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
				}
				else if(pone == 0){
				//	SPDK_NOTICELOG("thread:%d->7\n",ch->thread->id);
					spdk_thread_send_msg(th5,rout_submit_request,bdev_io);
					zeone1=1;
				}
				else{
					spdk_thread_send_msg(th6,rout_submit_request,bdev_io);
					zeone1=0;
				}
			
			}
			}
			else{
			//	SPDK_NOTICELOG("thread:%d->7\n",ch->thread->id);
					//spdk_thread_send_msg(th3,rout_submit_request,bdev_io);
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}
			}
			else{
				//SPDK_NOTICELOG("else\n");
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}
			*/	
			//////////////*********************************************///////////////////
			

			///////////////////3. 16384B 데이터를 th2,th5,th6에서 처리하는 방법//////////////////// 
			/*	
			if(bdev_io->u.nvme_passthru.cmd.cdw12 == 3){
			
			if(th5 != NULL && th6 != NULL && th4 != NULL){
				if(ch->thread->id == 5 || ch->thread->id == 6 || ch->thread->id == 4){
				//	SPDK_NOTICELOG("thread:%d\n",ch->thread->id);
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
				}
				else if(pone == 0){
				//	SPDK_NOTICELOG("thread:%d->7\n",ch->thread->id);
					spdk_thread_send_msg(th4,rout_submit_request,bdev_io);
					zeone1=1;
				}
				else if(pone == 1){
					spdk_thread_send_msg(th5,rout_submit_request,bdev_io);
					zeone1=2;
				}
				else{
					spdk_thread_send_msg(th6,rout_submit_request,bdev_io);
					zeone1=0;
				}
			}
			else{
			//	SPDK_NOTICELOG("thread:%d->7\n",ch->thread->id);
					//spdk_thread_send_msg(th3,rout_submit_request,bdev_io);
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}
			}
			else{
				//SPDK_NOTICELOG("else\n");
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}
			*/
			//////////////*********************************************///////////////////

			//SPDK_NOTICELOG("3_____%d\n",bdev_io->u.nvme_passthru.cmd.cdw12);
			//pone = zeone1;
			//if(th6 != NULL && th7 != NULL && th8 != NULL){
			/*
			if(th7 != NULL && th8 != NULL){
			//if(th2 != NULL && ch->thread->id != 3 && ch->thread->id != 2 && ch->thread->id == 4  && pone == 0){
			//if(ch->thread->id == 6 || ch->thread->id == 7 || ch->thread->id == 8){
			if(ch->thread->id == 7 || ch->thread->id == 8){
			//	SPDK_NOTICELOG("0_thread:%d\n",ch->thread->id);
				spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
				bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}
			else if(ch->thread->id != 7 && ch->thread->id != 8  && pone == 0){
			//else if(ch->thread->id != 6 && ch->thread->id != 7 && ch->thread->id != 8  && pone == 0){
			//		SPDK_NOTICELOG("0_p:%d, thread %d->send to th7\n",pone, ch->thread->id);
					//spdk_thread_send_msg(th6,rout_submit_request,bdev_io);
					spdk_thread_send_msg(th7,rout_submit_request,bdev_io);
				zeone1=1;
			}
			//else if (th3 != NULL && ch->thread->id != 2 && ch->thread->id != 3 && ch->thread->id != 4 && pone == 1){
			///else if (ch->thread->id != 6 && ch->thread->id != 7 && ch->thread->id != 8 && pone == 1){
				//	SPDK_NOTICELOG("0_p:%d, thread %d->send to th7\n",pone, ch->thread->id);
			///			spdk_thread_send_msg(th7, rout_submit_request,bdev_io);
			///		zeone1=2;
			///}
			//else if (th4 !=NULL && ch->thread->id != 3 && ch->thread->id != 2 && ch->thread->id != 4 &&	pone == 2){
			//else if (ch->thread->id != 3 && ch->thread->id != 2 && ch->thread->id != 4 &&	pone == 2){
			else{

				//	SPDK_NOTICELOG("0_#### p:%d, thread %d->send to th8\n",pone, ch->thread->id);
					spdk_thread_send_msg(th8, rout_submit_request,bdev_io);
					zeone1=0;
			}
			}
			else{
				SPDK_NOTICELOG("0_out_____p:%d,now th_id:%d\n",pone,ch->thread->id);
				spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);

			}*/
			/*
			}
			else{
				//SPDK_NOTICELOG("else\n");
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}*/
			
			/*
			else{

			//SPDK_NOTICELOG("0_____%d\n",bdev_io->u.nvme_passthru.cmd.cdw12);

			//여기해제 
			pone = zeone;
			//if(th5 != NULL && th3 != NULL && th4 != NULL){
			if(th2 != NULL && th5 != NULL && th3 != NULL && th4 != NULL && th6 != NULL && th9 != NULL){
			//if(th2 != NULL && ch->thread->id != 3 && ch->thread->id != 2 && ch->thread->id == 4  && pone == 0){
			if(ch->thread->id == 2 || ch-> thread->id == 9 || ch->thread->id == 5 || ch->thread->id == 3 || ch->thread->id == 4 || ch->thread->id == 6){
				//SPDK_NOTICELOG("0_1_thread:%d\n",ch->thread->id);
				spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
				bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}
			else if(ch->thread->id != 2 && ch->thread->id != 9 && ch->thread->id != 3 && ch->thread->id != 5 && ch->thread->id != 4 && ch->thread->id != 6 && pone == 0){
				//	SPDK_NOTICELOG("5_p:%d, thread %d->send to th5\n",pone, ch->thread->id);
					spdk_thread_send_msg(th5,rout_submit_request,bdev_io);
				zeone=1;
			}
			//else if (th3 != NULL && ch->thread->id != 2 && ch->thread->id != 3 && ch->thread->id != 4 && pone == 1){
			else if(ch->thread->id != 2 && ch->thread->id != 9 && ch->thread->id != 3 && ch->thread->id != 5 && ch->thread->id != 4 && ch->thread->id != 6 && pone == 1){
			//else if (ch->thread->id != 5 && ch->thread->id != 3 && ch->thread->id != 4 && ch->thread->id != 6 && pone == 1){
				//	SPDK_NOTICELOG("3_p:%d, thread %d->send to th3\n",pone, ch->thread->id);
					spdk_thread_send_msg(th3, rout_submit_request,bdev_io);
				zeone=2;
			}
			else if(ch->thread->id != 2 && ch->thread->id != 9 && ch->thread->id != 3 && ch->thread->id != 5 && ch->thread->id != 4 && ch->thread->id != 6 && pone == 2){
			//else if (ch->thread->id != 5 && ch->thread->id != 3 && ch->thread->id != 4 && ch->thread->id != 6 && pone == 2){
				//	SPDK_NOTICELOG("6_p:%d, thread %d->send to th6\n",pone, ch->thread->id);
					spdk_thread_send_msg(th6, rout_submit_request,bdev_io);
				zeone=3;
			}
			else if(ch->thread->id != 2 && ch->thread->id != 9 && ch->thread->id != 3 && ch->thread->id != 5 && ch->thread->id != 4 && ch->thread->id != 6 && pone == 3){
			//else if (ch->thread->id != 5 && ch->thread->id != 3 && ch->thread->id != 4 && ch->thread->id != 6 && pone == 2){
				//	SPDK_NOTICELOG("2_p:%d, thread %d->send to th2\n",pone, ch->thread->id);
					spdk_thread_send_msg(th2, rout_submit_request,bdev_io);
				zeone=4;
			}
			else if(ch->thread->id != 2 && ch->thread->id != 9 && ch->thread->id != 3 && ch->thread->id != 5 && ch->thread->id != 4 && ch->thread->id != 6 && pone == 4){
			//else if (ch->thread->id != 5 && ch->thread->id != 3 && ch->thread->id != 4 && ch->thread->id != 6 && pone == 2){
				//	SPDK_NOTICELOG("9_p:%d, thread %d->send to th9\n",pone, ch->thread->id);
					spdk_thread_send_msg(th9, rout_submit_request,bdev_io);
				zeone=5;
			}	
			//else if (th4 !=NULL && ch->thread->id != 3 && ch->thread->id != 2 && ch->thread->id != 4 &&	pone == 2){
			//else if (ch->thread->id != 3 && ch->thread->id != 2 && ch->thread->id != 4 &&	pone == 2){
			else{

				//	SPDK_NOTICELOG("4_p:%d, thread %d->send to th4\n",pone, ch->thread->id);
					spdk_thread_send_msg(th4, rout_submit_request,bdev_io);
					zeone=0;
			}
			}	
			else{
				SPDK_NOTICELOG("out_____p:%d,now th_id:%d\n",pone,ch->thread->id);
				spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);

			}
			}*/
			///////////////////
			///////////////////
			///////////////////



			/*
			
			if(bdev_io->u.nvme_passthru.cmd.cdw12 == 1){
				switch (ch->thread->id){
				case 6:
					break;
				case 7:
					break;
				case 8:
					break;
				case 9:
					break;
				case 10:
					break;
				case 2:
					if(th6 != NULL){
						SPDK_NOTICELOG("2. 2,6send:%d->%d __ size:%d\n",ch->thread->id, th6->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th6;
						spdk_thread_send_msg(th6, rout_submit_request, bdev_io);
					}
					break;
				case 3:
					if(th7 != NULL){
						SPDK_NOTICELOG("2. 3,7send:%d->%d __ size:%d\n",ch->thread->id, th7->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th7;
						spdk_thread_send_msg(th7, rout_submit_request, bdev_io);
					}
					break;
				case 4:
					if(th8 != NULL){
						SPDK_NOTICELOG("2. 4,8send:%d->%d __ size:%d\n",ch->thread->id, th8->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th8;
						spdk_thread_send_msg(th8, rout_submit_request, bdev_io);
					}
					break;
				case 5:
					if(th9 != NULL){
						SPDK_NOTICELOG("2. 5,9send:%d->%d __ size:%d\n",ch->thread->id, th9->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th9;
						spdk_thread_send_msg(th9, rout_submit_request, bdev_io);
					}
					break;
				
				default:
					if(th10 != NULL){
						SPDK_NOTICELOG("2. ?,10send:%d->%d __ size:%d\n",ch->thread->id, th10->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th10;
						spdk_thread_send_msg(th10, rout_submit_request, bdev_io);
					}
					break;
				}
			}
			else{
				switch (ch->thread->id){
				case 1:
					break;
				case 2:
					break;
				case 3:
					break;
				case 4:
					break;
				case 5:
					break;
				case 10:
					break;
				case 6:
					if(th2 != NULL){
						SPDK_NOTICELOG("6,2send:%d->%d __ size:%d\n",ch->thread->id, th2->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th2;
						spdk_thread_send_msg(th2, rout_submit_request, bdev_io);
					}
					break;
				case 7:
					if(th3 != NULL){
						SPDK_NOTICELOG("7,3send:%d->%d __ size:%d\n",ch->thread->id, th3->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th3;
						spdk_thread_send_msg(th3, rout_submit_request, bdev_io);
					}
					break;
				case 8:
					if(th4 != NULL){
						SPDK_NOTICELOG("8,4send:%d->%d __ size:%d\n",ch->thread->id, th4->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th4;
						spdk_thread_send_msg(th4, rout_submit_request, bdev_io);
					}
					break;
				case 9:
					if(th5 != NULL){
						SPDK_NOTICELOG("9,5send:%d->%d __ size:%d\n",ch->thread->id, th5->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th5;
						spdk_thread_send_msg(th5, rout_submit_request, bdev_io);
					}
					break;
				default:
					if(th10 != NULL){
						SPDK_NOTICELOG("?,10send:%d->%d __ size:%d\n",ch->thread->id, th10->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th10;
						spdk_thread_send_msg(th10, rout_submit_request, bdev_io);
					}
					break;
				}
			}

			if(ch_b == NULL){
				struct spdk_thread *thread_now = spdk_get_thread();
				SPDK_NOTICELOG("2. no move:thread:%d:size-%d\n",thread_now->id,bdev_io->u.nvme_passthru.cmd.cdw12);
				spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}*/

		
		/* background : thread 6,7,8,9,10이면 2, 3, 4, 5로 바꿔줌, 2, 3, 4, 5이면 그대로 전송 

		ch_b = NULL;	
		switch (ch->thread->id){
				case 1:
					break;
				case 2:
					break;
				case 3:
					break;
				case 4:
					break;
				case 5:
					break;
				case 6:
					if(th2 != NULL){
						SPDK_NOTICELOG("1. back6,2send:%d->%d __ size:%d\n",ch->thread->id, th2->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th2;
						spdk_thread_send_msg(th2, rout_submit_request2, th2->id);	
						//spdk_thread_send_msg(th2, rout_submit_request, bdev_io);
					}
					break;
				case 7:
					if(th3 != NULL){
						SPDK_NOTICELOG("1. back7,3send:%d->%d __ size:%d\n",ch->thread->id, th3->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th3;
						spdk_thread_send_msg(th3, rout_submit_request2, th3->id);	
						//spdk_thread_send_msg(th3, rout_submit_request, bdev_io);
					}
					break;
				case 8:
					if(th4 != NULL){
						SPDK_NOTICELOG("1. back8,4send:%d->%d __ size:%d\n",ch->thread->id, th4->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th4;
						spdk_thread_send_msg(th4, rout_submit_request2, th4->id);	
					}
					break;
				case 9:
					if(th5 != NULL){
						SPDK_NOTICELOG("1. back9,5send:%d->%d __ size:%d\n",ch->thread->id, th5->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th5;
						spdk_thread_send_msg(th5, rout_submit_request2, th5->id);	
					}
					break;
				case 10:
					if(th2 != NULL){
						SPDK_NOTICELOG("1. back9,5send:%d->%d __ size:%d\n",ch->thread->id, th2->id, bdev_io->u.nvme_passthru.cmd.cdw12);
						ch_b = th2;
						spdk_thread_send_msg(th2, rout_submit_request2, th2->id);	
					}
					break;
		}
			
		if(ch_b == NULL){
				struct spdk_thread *thread_now = spdk_get_thread();
				SPDK_NOTICELOG("1. background: no move_thread:%d:size-%d\n",thread_now->id,bdev_io->u.nvme_passthru.cmd.cdw12);
				//spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
				//		bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
				uint32_t blk_size, buf_align;
				blk_size = 512;
				if(buc == 0){
					buc++;
					buff = spdk_dma_zmalloc(512,512,NULL);
					snprintf(buff, blk_size, "%s", "Hello World\n");
				}
				spdk_bdev_nvme_io_passthru2(bdev_desc, pt_ch->base_ch, buff, 1024, _back_io, buff);
		}
		****************************************************************************************/

			
			//spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
			//		bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);

			 // foreground I/O 조절 실험
			/*	
			if(g_1 %2 == 0){
				g_1++;
			
				if(g_1 ==0){
					g_1++;
					f_ch = ch;
					n_ch = ch;
				}
				else if(g_1 % 2 == 1){
					g_1++;
					f_ch = ch;
				}
				else{
					g_1++;
					n_ch = ch;
				}
			}
			if(g_1 > 21){
				g_1 = 0;
			}*/
			/* 여기 해제하기
			if(g_1 == 0){
				g_1++;
				n_ch == ch;
				f_ch = ch;
			}
			n_ch = f_ch;
			f_ch = ch;

			if(n_ch != ch){
				ttt = 1;
			}
			SPDK_NOTICELOG("before ch:%d,now ch:%d\n",n_ch->thread->id, f_ch->thread->id);*/
			/*실제 코드
			struct spdk_thread *thread_now = spdk_get_thread();
			SPDK_NOTICELOG("thread:%d:size-%d\n",thread_now->id,bdev_io->u.nvme_passthru.cmd.cdw12);
			spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			*/
			//if(ch_all3 != NULL && n_ch != NULL && f_ch != NULL && f_ch->thread->id != ch_all3->thread->id && n_ch->thread->id != ch_all3->thread->id && ch_all3->thread->id > 0 && ch_all3->thread->id <= 10 && ch_all3->thread->id != ch->thread->id && g_cnt3 == 1 ){
			/*	
			if(ch_all1 != NULL && ch_all1->thread->id != ch->thread->id){
				SPDK_NOTICELOG("C1_11111__:ch:%d->%d\n",ch->thread->id,ch_all1->thread->id);
				spdk_thread_send_msg(ch_all1->thread, rout_submit_request, bdev_io);	
				//spdk_thread_send_msg(ch_all1->thread, rout_submit_request, ch_all1->thread->id);	
			}
			else if(ch_all2 != NULL && ch_all2->thread->id != ch->thread->id){
			//else if(ch_all2 != NULL && n_ch != NULL && f_ch != NULL && f_ch->thread->id != ch_all2->thread->id && n_ch->thread->id != ch_all2->thread->id && ch_all2->thread->id > 0 && ch_all2->thread->id <= 10 && ch_all2->thread->id != ch->thread->id && g_cnt2 == 1 ){
				SPDK_NOTICELOG("C1_22222___:ch:%d->%d\n",ch->thread->id,ch_all2->thread->id);
				spdk_thread_send_msg(ch_all2->thread, rout_submit_request, bdev_io);	
				//spdk_thread_send_msg(ch_all2->thread, rout_submit_request, ch_all2->thread->id);	
			}*/
			//}
			//else{
			//	g_1++;
			//SPDK_NOTICELOG("3:%d,2:%d,ch:%d\n",ch_all3->thread->id, ch_all2->thread->id, ch->thread->id);
			/*
			spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
					bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			}
			
			if(g_1 > 20){
				g_1 = 0;
			}*/
		/*
			SPDK_NOTICELOG("____ch_all2:%d,___ch_all3:%d,___ch_all4:%d___ch_all5:%d,____ch:%d\n",ch_all2->thread->id, ch_all3->thread->id,ch_all4->thread->id,ch_all5->thread->id,ch->thread->id);

			if(ttt>0){
				if(ttt > 10)
					ttt = 0;
				else
					ttt++;
			}
			else if(ch == n_ch){
				if(ch_all1 != NULL && ch->thread->id == 6 ){
					SPDK_NOTICELOG("C1_ch_all1:ch:%d->%d\n",ch->thread->id,ch_all1->thread->id);
					spdk_thread_send_msg(ch_all1->thread, rout_submit_request2, ch_all1->thread->id);	
				}
				else if(ch_all2 != NULL && ch->thread->id == 7){
					SPDK_NOTICELOG("C1_ch_all2:ch:%d->%d\n",ch->thread->id,ch_all2->thread->id);
					spdk_thread_send_msg(ch_all2->thread, rout_submit_request2, ch_all2->thread->id);
				}
				else if(ch_all3 != NULL && ch->thread->id == 8){
					SPDK_NOTICELOG("C1_ch_all3:ch:%d->%d\n",ch->thread->id,ch_all3->thread->id);
					spdk_thread_send_msg(ch_all3->thread, rout_submit_request2, ch_all3->thread->id);
				}
				else if(ch_all4 != NULL && ch->thread->id == 9){
					SPDK_NOTICELOG("C1_ch_all4:ch:%d->%d\n",ch->thread->id,ch_all4->thread->id);
					spdk_thread_send_msg(ch_all4->thread, rout_submit_request2, ch_all4->thread->id);
				}
				else if(ch_all5 != NULL && ch->thread->id == 10){
					SPDK_NOTICELOG("C1_ch_all15:ch:%d->%d\n",ch->thread->id,ch_all5->thread->id);
					spdk_thread_send_msg(ch_all5->thread, rout_submit_request2, ch_all5->thread->id);
				}
				else{
					SPDK_NOTICELOG("C1 orig:ch:%d\n",ch->thread->id);
					spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
						bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
				}
			}*/


			/*
			if(ch_all1 != NULL && ch_all1->thread->id > 0 && ch_all1->thread->id <= 10 && ch_all1->thread->id != ch->thread->id && g_cnt1 == 1 && ch->thread->id != n_ch->thread->id){
				SPDK_NOTICELOG("C1_2___:ch:%d->%d\n",ch->thread->id,ch_all2->thread->id);
				spdk_thread_send_msg(ch_all1->thread, rout_submit_request2, ch_all1->thread->id);	
			}
			else if(ch_all2 != NULL && ch_all2->thread->id > 0 && ch_all2->thread->id <= 10 && ch_all2->thread->id != ch->thread->id && g_cnt2 == 1 && ch->thread->id != n_ch->thread->id){
				SPDK_NOTICELOG("C1_2___:ch:%d->%d\n",ch->thread->id,ch_all2->thread->id);
				spdk_thread_send_msg(ch_all2->thread, rout_submit_request2, ch_all2->thread->id);	
			}
			else if(ch_all3 != NULL && ch_all3->thread->id > 0 && ch_all3->thread->id <= 10 && ch_all3->thread->id != ch->thread->id && g_cnt3 == 1 ){
				SPDK_NOTICELOG("C1_3___:ch:%d->%d\n",ch->thread->id,ch_all3->thread->id);
				spdk_thread_send_msg(ch_all3->thread, rout_submit_request2, ch_all3->thread->id);	
			}
			else if(ch_all4 != NULL && ch_all4->thread->id > 0 && ch_all4->thread->id <= 10 && ch_all4->thread->id != ch->thread->id && g_cnt4 == 1 ){
				SPDK_NOTICELOG("C1_4___:ch:%d->%d\n",ch->thread->id,ch_all4->thread->id);
				spdk_thread_send_msg(ch_all4->thread, rout_submit_request2, ch_all4->thread->id);	
			}
			}
			*/
			//else {
			//	ttt++;
				/*
				if(ch_all3 != NULL && ch_all3->thread->id > 0 && ch_all3->thread->id <= 10 && ch_all3->thread->id != ch->thread->id && g_cnt3 == 1i && n_ch->thread->id != ch_all3->thread->id ){
				SPDK_NOTICELOG("C1_/3___:ch:%d->%d\n",ch->thread->id,ch_all3->thread->id);
				spdk_thread_send_msg(ch_all3->thread, rout_submit_request2, ch_all3->thread->id);	
				}
			else if(ch_all2 != NULL && ch_all2->thread->id > 0 && ch_all2->thread->id <= 10 && ch_all2->thread->id != ch->thread->id && g_cnt2 == 1 && ch->thread->id != n_ch->thread->id && n_ch->thread->id != ch_all2->thread->id){
				SPDK_NOTICELOG("C1_/2___:ch:%d->%d\n",ch->thread->id,ch_all2->thread->id);
				spdk_thread_send_msg(ch_all2->thread, rout_submit_request2, ch_all2->thread->id);	
				}
			else if(ch_all4 != NULL && ch_all4->thread->id > 0 && ch_all4->thread->id <= 10 && ch_all4->thread->id != ch->thread->id && g_cnt4 == 1 && n_ch->thread->id && ch_all4->thread->id){
				SPDK_NOTICELOG("C1_/4___:ch:%d->%d\n",ch->thread->id,ch_all4->thread->id);
				spdk_thread_send_msg(ch_all4->thread, rout_submit_request2, ch_all4->thread->id);	
				}
			}
			SPDK_NOTICELOG("C1 orig:ch:%d\n",ch->thread->id);
			spdk_bdev_nvme_io_passthru(pt_node->base_desc, pt_ch->base_ch, &bdev_io->u.nvme_passthru.cmd,
					bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw12*512, _pt_complete_io, bdev_io);
			
			*/
			
		}
		else if(bdev_io->u.nvme_passthru.cmd.opc==0xD1){
			if(ch_all3 != NULL && ch_all3->thread->id > 0 && ch_all3->thread->id <= 10 && ch_all3->thread->id != ch->thread->id && g_cnt3 == 1 ){
				SPDK_NOTICELOG("D1___:ch3:%d\n",ch_all3->thread->id);
				pt_ch2 = spdk_io_channel_get_ctx(ch_all3);
				spdk_thread_send_msg(ch_all3->thread, rout_submit_request, bdev_io);
			}
			else if(ch_all2 != NULL && ch_all2->thread->id > 0 && ch_all2->thread->id <= 10 && ch_all2->thread->id != ch->thread->id && g_cnt2 == 1 ){
				SPDK_NOTICELOG("D1___:ch3:%d\n",ch_all2->thread->id);
				pt_ch2 = spdk_io_channel_get_ctx(ch_all2);
				spdk_thread_send_msg(ch_all2->thread, rout_submit_request, bdev_io);
			}
		}
		break;
	default:
		SPDK_ERRLOG("rout: unknown I/O type %d\n", bdev_io->type);
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
		return;
	}
	if (rc != 0) {
		if (rc == -ENOMEM) {
			SPDK_ERRLOG("No memory, start to queue io for rout.\n");
			io_ctx->ch = ch;
			vbdev_rout_queue_io(bdev_io);
		} else {
			SPDK_ERRLOG("ERROR on bdev_io submission!\n");
			spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
		}
	}
}

/* We'll just call the base bdev and let it answer however if we were more
 * restrictive for some reason (or less) we could get the response back
 * and modify according to our purposes.
 */
static bool
vbdev_rout_io_type_supported(void *ctx, enum spdk_bdev_io_type io_type)
{
	struct vbdev_rout *pt_node = (struct vbdev_rout *)ctx;

	return spdk_bdev_io_type_supported(pt_node->base_bdev, io_type);
}

/* We supplied this as an entry point for upper layers who want to communicate to this
 * bdev.  This is how they get a channel. We are passed the same context we provided when
 * we created our PT vbdev in examine() which, for this bdev, is the address of one of
 * our context nodes. From here we'll ask the SPDK channel code to fill out our channel
 * struct and we'll keep it in our PT node.
 */
static struct spdk_io_channel *
vbdev_rout_get_io_channel(void *ctx)
{
	struct vbdev_rout *pt_node = (struct vbdev_rout *)ctx;
	struct spdk_io_channel *pt_ch = NULL;

	/* The IO channel code will allocate a channel for us which consists of
	 * the SPDK channel structure plus the size of our pt_io_channel struct
	 * that we passed in when we registered our IO device. It will then call
	 * our channel create callback to populate any elements that we need to
	 * update.
	 */
	pt_ch = spdk_get_io_channel(pt_node);

	return pt_ch;
}

/* This is the output for bdev_get_bdevs() for this vbdev */
static int
vbdev_rout_dump_info_json(void *ctx, struct spdk_json_write_ctx *w)
{
	struct vbdev_rout *pt_node = (struct vbdev_rout *)ctx;

	spdk_json_write_name(w, "rout_external");
	spdk_json_write_object_begin(w);
	spdk_json_write_named_string(w, "name", spdk_bdev_get_name(&pt_node->pt_bdev));
	spdk_json_write_named_string(w, "base_bdev_name", spdk_bdev_get_name(pt_node->base_bdev));
	spdk_json_write_object_end(w);

	return 0;
}

/* This is used to generate JSON that can configure this module to its current state. */
static int
vbdev_rout_config_json(struct spdk_json_write_ctx *w)
{
	struct vbdev_rout *pt_node;

	TAILQ_FOREACH(pt_node, &g_pt_nodes, link) {
		spdk_json_write_object_begin(w);
		spdk_json_write_named_string(w, "method", "construct_ext_rout_bdev");
		spdk_json_write_named_object_begin(w, "params");
		spdk_json_write_named_string(w, "base_bdev_name", spdk_bdev_get_name(pt_node->base_bdev));
		spdk_json_write_named_string(w, "name", spdk_bdev_get_name(&pt_node->pt_bdev));
		spdk_json_write_object_end(w);
		spdk_json_write_object_end(w);
	}
	return 0;
}

/* We provide this callback for the SPDK channel code to create a channel using
 * the channel struct we provided in our module get_io_channel() entry point. Here
 * we get and save off an underlying base channel of the device below us so that
 * we can communicate with the base bdev on a per channel basis.  If we needed
 * our own poller for this vbdev, we'd register it here.
 */
static int
pt_bdev_ch_create_cb(void *io_device, void *ctx_buf)
{
	struct pt_io_channel *pt_ch = ctx_buf;
	struct vbdev_rout *pt_node = io_device;

	pt_ch->base_ch = spdk_bdev_get_io_channel(pt_node->base_desc);

	return 0;
}

/* We provide this callback for the SPDK channel code to destroy a channel
 * created with our create callback. We just need to undo anything we did
 * when we created. If this bdev used its own poller, we'd unregister it here.
 */
static void
pt_bdev_ch_destroy_cb(void *io_device, void *ctx_buf)
{
	struct pt_io_channel *pt_ch = ctx_buf;

	spdk_put_io_channel(pt_ch->base_ch);
}

/* Create the rout association from the bdev and vbdev name and insert
 * on the global list. */
static int
vbdev_rout_insert_name(const char *bdev_name, const char *vbdev_name)
{
	struct bdev_names *name;

	TAILQ_FOREACH(name, &g_bdev_names, link) {
		if (strcmp(vbdev_name, name->vbdev_name) == 0) {
			SPDK_ERRLOG("rout bdev %s already exists\n", vbdev_name);
			return -EEXIST;
		}
	}

	name = calloc(1, sizeof(struct bdev_names));
	if (!name) {
		SPDK_ERRLOG("could not allocate bdev_names\n");
		return -ENOMEM;
	}

	name->bdev_name = strdup(bdev_name);
	if (!name->bdev_name) {
		SPDK_ERRLOG("could not allocate name->bdev_name\n");
		free(name);
		return -ENOMEM;
	}

	name->vbdev_name = strdup(vbdev_name);
	if (!name->vbdev_name) {
		SPDK_ERRLOG("could not allocate name->vbdev_name\n");
		free(name->bdev_name);
		free(name);
		return -ENOMEM;
	}

	TAILQ_INSERT_TAIL(&g_bdev_names, name, link);

	return 0;
}

/* On init, just perform bdev module specific initialization. */
static int
vbdev_rout_init(void)
{
	return 0;
}

/* Called when the entire module is being torn down. */
static void
vbdev_rout_finish(void)
{
	struct bdev_names *name;

	while ((name = TAILQ_FIRST(&g_bdev_names))) {
		TAILQ_REMOVE(&g_bdev_names, name, link);
		free(name->bdev_name);
		free(name->vbdev_name);
		free(name);
	}
}

/* During init we'll be asked how much memory we'd like passed to us
 * in bev_io structures as context. Here's where we specify how
 * much context we want per IO.
 */
static int
vbdev_rout_get_ctx_size(void)
{
	return sizeof(struct rout_bdev_io);
}

/* Where vbdev_rout_config_json() is used to generate per module JSON config data, this
 * function is called to output any per bdev specific methods. For the PT module, there are
 * none.
 */
static void
vbdev_rout_write_config_json(struct spdk_bdev *bdev, struct spdk_json_write_ctx *w)
{
	/* No config per bdev needed */
}

/* When we register our bdev this is how we specify our entry points. */
static const struct spdk_bdev_fn_table vbdev_rout_fn_table = {
	.destruct		= vbdev_rout_destruct,
	.submit_request		= vbdev_rout_submit_request,
	.io_type_supported	= vbdev_rout_io_type_supported,
	.get_io_channel		= vbdev_rout_get_io_channel,
	.dump_info_json		= vbdev_rout_dump_info_json,
	.write_config_json	= vbdev_rout_write_config_json,
};

static void
vbdev_rout_base_bdev_hotremove_cb(struct spdk_bdev *bdev_find)
{
	struct vbdev_rout *pt_node, *tmp;

	TAILQ_FOREACH_SAFE(pt_node, &g_pt_nodes, link, tmp) {
		if (bdev_find == pt_node->base_bdev) {
			spdk_bdev_unregister(&pt_node->pt_bdev, NULL, NULL);
		}
	}
}

/* Called when the underlying base bdev triggers asynchronous event such as bdev removal. */
static void
vbdev_rout_base_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev,
				  void *event_ctx)
{
	switch (type) {
	case SPDK_BDEV_EVENT_REMOVE:
		vbdev_rout_base_bdev_hotremove_cb(bdev);
		break;
	default:
		SPDK_NOTICELOG("Unsupported bdev event: type %d\n", type);
		break;
	}
}

/* Create and register the rout vbdev if we find it in our list of bdev names.
 * This can be called either by the examine path or RPC method.
 */
static int
vbdev_rout_register(const char *bdev_name)
{
	struct bdev_names *name;
	struct vbdev_rout *pt_node;
	struct spdk_bdev *bdev;
	int rc = 0;

	/* Check our list of names from config versus this bdev and if
	 * there's a match, create the pt_node & bdev accordingly.
	 */
	TAILQ_FOREACH(name, &g_bdev_names, link) {
		if (strcmp(name->bdev_name, bdev_name) != 0) {
			continue;
		}

		//SPDK_NOTICELOG("Match on %s\n", bdev_name);
		pt_node = calloc(1, sizeof(struct vbdev_rout));
		if (!pt_node) {
			rc = -ENOMEM;
			SPDK_ERRLOG("could not allocate pt_node\n");
			break;
		}

		pt_node->pt_bdev.name = strdup(name->vbdev_name);
		if (!pt_node->pt_bdev.name) {
			rc = -ENOMEM;
			SPDK_ERRLOG("could not allocate pt_bdev name\n");
			free(pt_node);
			break;
		}
		pt_node->pt_bdev.product_name = "rout";

		/* The base bdev that we're attaching to. */
		rc = spdk_bdev_open_ext(bdev_name, true, vbdev_rout_base_bdev_event_cb,
					NULL, &pt_node->base_desc);
		if (rc) {
			if (rc != -ENODEV) {
				SPDK_ERRLOG("could not open bdev %s\n", bdev_name);
			}
			free(pt_node->pt_bdev.name);
			free(pt_node);
			break;
		}
		//SPDK_NOTICELOG("base bdev opened\n");

		bdev = spdk_bdev_desc_get_bdev(pt_node->base_desc);
		pt_node->base_bdev = bdev;

		/* Copy some properties from the underlying base bdev. */
		pt_node->pt_bdev.write_cache = bdev->write_cache;
		pt_node->pt_bdev.required_alignment = bdev->required_alignment;
		pt_node->pt_bdev.optimal_io_boundary = bdev->optimal_io_boundary;
		pt_node->pt_bdev.blocklen = bdev->blocklen;
		pt_node->pt_bdev.blockcnt = bdev->blockcnt;

		pt_node->pt_bdev.md_interleave = bdev->md_interleave;
		pt_node->pt_bdev.md_len = bdev->md_len;
		pt_node->pt_bdev.dif_type = bdev->dif_type;
		pt_node->pt_bdev.dif_is_head_of_md = bdev->dif_is_head_of_md;
		pt_node->pt_bdev.dif_check_flags = bdev->dif_check_flags;

		/* This is the context that is passed to us when the bdev
		 * layer calls in so we'll save our pt_bdev node here.
		 */
		pt_node->pt_bdev.ctxt = pt_node;
		pt_node->pt_bdev.fn_table = &vbdev_rout_fn_table;
		pt_node->pt_bdev.module = &rout_if;
		TAILQ_INSERT_TAIL(&g_pt_nodes, pt_node, link);

		spdk_io_device_register(pt_node, pt_bdev_ch_create_cb, pt_bdev_ch_destroy_cb,
					sizeof(struct pt_io_channel),
					name->vbdev_name);
		//SPDK_NOTICELOG("io_device created at: 0x%p\n", pt_node);

		/* Save the thread where the base device is opened */
		pt_node->thread = spdk_get_thread();

		rc = spdk_bdev_module_claim_bdev(bdev, pt_node->base_desc, pt_node->pt_bdev.module);
		if (rc) {
			SPDK_ERRLOG("could not claim bdev %s\n", bdev_name);
			spdk_bdev_close(pt_node->base_desc);
			TAILQ_REMOVE(&g_pt_nodes, pt_node, link);
			spdk_io_device_unregister(pt_node, NULL);
			free(pt_node->pt_bdev.name);
			free(pt_node);
			break;
		}
		//SPDK_NOTICELOG("bdev claimed\n");

		rc = spdk_bdev_register(&pt_node->pt_bdev);
		if (rc) {
			SPDK_ERRLOG("could not register pt_bdev\n");
			spdk_bdev_module_release_bdev(&pt_node->pt_bdev);
			spdk_bdev_close(pt_node->base_desc);
			TAILQ_REMOVE(&g_pt_nodes, pt_node, link);
			spdk_io_device_unregister(pt_node, NULL);
			free(pt_node->pt_bdev.name);
			free(pt_node);
			break;
		}
		//SPDK_NOTICELOG("ext_pt_bdev registered\n");
		//SPDK_NOTICELOG("created ext_pt_bdev for: %s\n", name->vbdev_name);
	}

	return rc;
}

/* Create the rout disk from the given bdev and vbdev name. */
int
bdev_rout_external_create_disk(const char *bdev_name, const char *vbdev_name)
{
	int rc;

	/* Insert the bdev name into our global name list even if it doesn't exist yet,
	 * it may show up soon...
	 */
	rc = vbdev_rout_insert_name(bdev_name, vbdev_name);
	if (rc) {
		return rc;
	}

	rc = vbdev_rout_register(bdev_name);
	if (rc == -ENODEV) {
		/* This is not an error, we tracked the name above and it still
		 * may show up later.
		 */
		SPDK_NOTICELOG("vbdev creation deferred pending base bdev arrival\n");
		rc = 0;
	}

	return rc;
}

void
bdev_rout_external_delete_disk(struct spdk_bdev *bdev, spdk_bdev_unregister_cb cb_fn,
				   void *cb_arg)
{
	struct bdev_names *name;

	if (!bdev || bdev->module != &rout_if) {
		cb_fn(cb_arg, -ENODEV);
		return;
	}

	/* Remove the association (vbdev, bdev) from g_bdev_names. This is required so that the
	 * vbdev does not get re-created if the same bdev is constructed at some other time,
	 * unless the underlying bdev was hot-removed.
	 */
	TAILQ_FOREACH(name, &g_bdev_names, link) {
		if (strcmp(name->vbdev_name, bdev->name) == 0) {
			TAILQ_REMOVE(&g_bdev_names, name, link);
			free(name->bdev_name);
			free(name->vbdev_name);
			free(name);
			break;
		}
	}

	/* Additional cleanup happens in the destruct callback. */
	spdk_bdev_unregister(bdev, cb_fn, cb_arg);
}

/* Because we specified this function in our pt bdev function table when we
 * registered our pt bdev, we'll get this call anytime a new bdev shows up.
 * Here we need to decide if we care about it and if so what to do. We
 * parsed the config file at init so we check the new bdev against the list
 * we built up at that time and if the user configured us to attach to this
 * bdev, here's where we do it.
 */
static void
vbdev_rout_examine(struct spdk_bdev *bdev)
{
	vbdev_rout_register(bdev->name);

	spdk_bdev_module_examine_done(&rout_if);
}
