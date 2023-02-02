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
#include "sys/time.h"
#include "time.h"
#include "stdio.h"

#include "vbdev_passthru.h"
#include "spdk/rpc.h"
#include "spdk/env.h"
#include "spdk/endian.h"
#include "spdk/string.h"
#include "spdk/thread.h"
#include "spdk/util.h"

#include "spdk/bdev_module.h"
#include "spdk/log.h"
#include <openssl/sha.h>

int fin0 = 0;
double Result = 0;
int RC = 0;
static int vbdev_passthru_init(void);
static int vbdev_passthru_get_ctx_size(void);
static void vbdev_passthru_examine(struct spdk_bdev *bdev);
static void vbdev_passthru_finish(void);
static int vbdev_passthru_config_json(struct spdk_json_write_ctx *w);
/*
struct timeval {
	time_t tv_sec;
	suseconds_t tv_usec;
}
*/
struct _node{
	int address;
	int hash;
	struct _node *next;
};

struct arrayitem{
	struct _node *head;
	struct _node *tail;
};

struct arrayitem *array_ = NULL;
struct arrayitem *array1 = NULL;
struct arrayitem *array2 = NULL;
struct arrayitem *array3 = NULL;
struct arrayitem *array4 = NULL;

int arraycheck[4096] = {0,};
int arraycheck1[4096] = {0,};
int arraycheck2[4096] = {0,};
int arraycheck3[4096] = {0,};
int arraycheck4[4096] = {0,};

int journalcount = 0;
int chec = 20;
int bitvec[10000000] = { 0, };

static struct spdk_bdev_module passthru_if = {
	.name = "passthru_external",
	.module_init = vbdev_passthru_init,
	.get_ctx_size = vbdev_passthru_get_ctx_size,
	.examine_config = vbdev_passthru_examine,
	.module_fini = vbdev_passthru_finish,
	.config_json = vbdev_passthru_config_json
};

SPDK_BDEV_MODULE_REGISTER(ext_passthru, &passthru_if)

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
struct vbdev_passthru {
	struct spdk_bdev		*base_bdev; /* the thing we're attaching to */
	struct spdk_bdev_desc		*base_desc; /* its descriptor we get from open */
	/* 추가한 항목 */
	struct spdk_io_channel *bdev_io_channel;
	int hash;
	int offsets;
	int nblocks;
	struct spdk_bdev_io *orig_IO;
	struct spdk_bdev_io_wait_entry bdev_io_wait;
	struct pt_io_channel *pt_channel;
	/***************/
	struct spdk_bdev		pt_bdev;    /* the PT virtual bdev */
	TAILQ_ENTRY(vbdev_passthru)	link;
	struct spdk_thread		*thread;    /* thread where base device is opened */
};
static TAILQ_HEAD(, vbdev_passthru) g_pt_nodes = TAILQ_HEAD_INITIALIZER(g_pt_nodes);

/* The pt vbdev channel struct. It is allocated and freed on my behalf by the io channel code.
 * If this vbdev needed to implement a poller or a queue for IO, this is where those things
 * would be defined. This passthru bdev doesn't actually need to allocate a channel, it could
 * simply pass back the channel of the bdev underneath it but for example purposes we will
 * present its own to the upper layers.
 */
struct pt_io_channel {
	struct spdk_io_channel	*base_ch; /* IO channel of base device */
};

/* Just for fun, this pt_bdev module doesn't need it but this is essentially a per IO
 * context that we get handed by the bdev layer.
 */
struct passthru_bdev_io {
	uint8_t test;

	/* bdev related */
	struct spdk_io_channel *ch;

	/* for bdev_io_wait */
	struct spdk_bdev_io_wait_entry bdev_io_wait;
};

static void
vbdev_passthru_submit_request(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io);


/* Callback for unregistering the IO device. */
static void
_device_unregister_cb(void *io_device)
{
	struct vbdev_passthru *pt_node  = io_device;

	/* Done with this pt_node. */
	free(pt_node->pt_bdev.name);
	free(pt_node);
}

/* Wrapper for the bdev close operation. */
static void
_vbdev_passthru_destruct(void *ctx)
{
	struct spdk_bdev_desc *desc = ctx;

	spdk_bdev_close(desc);
}

/* Called after we've unregistered following a hot remove callback.
 * Our finish entry point will be called next.
 */
static int
vbdev_passthru_destruct(void *ctx)
{
	struct vbdev_passthru *pt_node = (struct vbdev_passthru *)ctx;

	/* It is important to follow this exact sequence of steps for destroying
	 * a vbdev...
	 */

	TAILQ_REMOVE(&g_pt_nodes, pt_node, link);

	/* Unclaim the underlying bdev. */
	spdk_bdev_module_release_bdev(pt_node->base_bdev);

	/* Close the underlying bdev on its same opened thread. */
	if (pt_node->thread && pt_node->thread != spdk_get_thread()) {
		spdk_thread_send_msg(pt_node->thread, _vbdev_passthru_destruct, pt_node->base_desc);
	} else {
		spdk_bdev_close(pt_node->base_desc);
	}

	/* Unregister the io_device. */
	spdk_io_device_unregister(pt_node, _device_unregister_cb);

	return 0;
}
static void
vbdev_passthru_base_bdev_hotremove_cb(struct spdk_bdev *bdev_find)
{
	struct vbdev_passthru *pt_node, *tmp;

	TAILQ_FOREACH_SAFE(pt_node, &g_pt_nodes, link, tmp) {
		if (bdev_find == pt_node->base_bdev) {
			spdk_bdev_unregister(&pt_node->pt_bdev, NULL, NULL);
		}
	}
}
/* Called when the underlying base bdev triggers asynchronous event such as bdev removal. */
static void
vbdev_passthru_base_bdev_event_cb(enum spdk_bdev_event_type type, struct spdk_bdev *bdev,
				  void *event_ctx)
{
	switch (type) {
	case SPDK_BDEV_EVENT_REMOVE:
		vbdev_passthru_base_bdev_hotremove_cb(bdev);
		break;
	default:
		SPDK_NOTICELOG("Unsupported bdev event: type %d\n", type);
		break;
	}
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
	int tt = 0;
	struct passthru_bdev_io *io_ctx= (struct passthru_bdev_io *)orig_io->driver_ctx;

	time_t t;
	struct tm *lt;
	struct timeval tv;
/*	
	clock_t start = clock();
	
	if(orig_io->u.nvme_passthru.cmd.opc==0xC1){
		pthread_mutex_lock(&(orig_io->bdev)->internal.mutex);
		Result = Result + clock();
		RC++;
		printf("Result = %f\n",Result/RC);
		pthread_mutex_unlock(&(orig_io->bdev)->internal.mutex);
	}*/
	char buff[4096];
	char *pp;
	fin0 = 0;
	if (io_ctx->test != 0x5a) {
		SPDK_ERRLOG("Error, original IO device_ctx is wrong! 0x%x\n",
			    io_ctx->test);
	}
	
	//printf("pt_complete_io\n\n");
	/* Complete the original IO and then free the one that we created here
	 * as a result of issuing an IO via submit_request.
	 */
	spdk_bdev_io_complete(orig_io, status);
	spdk_bdev_free_io(bdev_io);
	//printf("pt_complete_io_fin\n");
}

static void
write_complete3(struct spdk_bdev_io *bdev_io, bool success, void *ch_arg)
{
	struct spdk_bdev_io *orig_io = ch_arg;
	struct vbdev_passthru *pt_node = SPDK_CONTAINEROF(orig_io->bdev, struct vbdev_passthru, pt_bdev);
	int status = success ? SPDK_BDEV_IO_STATUS_SUCCESS : SPDK_BDEV_IO_STATUS_FAILED;
	int offset_block = 0;
	spdk_bdev_io_complete(orig_io, status);
	spdk_bdev_free_io(bdev_io);
	pt_node->base_bdev = spdk_bdev_desc_get_bdev(pt_node->base_desc);

//	printf("write_complete3:bdev_io->u.bdev.offset_blocks:%d\n",bdev_io->u.bdev.offset_blocks);
	//_pt_complete_io(bdev_io, success, orig_io);
	spdk_bdev_write(pt_node->base_desc, pt_node->bdev_io_channel,
				pt_node->orig_IO->u.nvme_passthru.buf, offset_block*512, 512, _pt_complete_io, pt_node->base_bdev);
}

static int
bdev_Journaling(struct vbdev_passthru *pt_node, struct spdk_io_channel *ch)
{
	struct spdk_bdev_desc *desc = pt_node->base_desc;
	struct spdk_bdev *bdev = spdk_bdev_desc_get_bdev(desc);
	struct spdk_bdev_io *bdev_io = pt_node->orig_IO;
	void *buf = pt_node->orig_IO->u.nvme_passthru.buf;
	/*
	int hash = pt_node->hash;
	int j,i=0;
	char *buff;
	uint64_t offset = pt_node->offsets;
	uint64_t nbytes = pt_node->nblocks;
	//printf("bdev_journaling start!\n");
	buff = spdk_dma_zmalloc(512, 0, NULL);

	j = sprintf(buff,"%d,%d,%d",hash,offset,nbytes);
	printf("journal buf: %s\n",buff);*/
	/*
	pthread_mutex_lock(&bdev->internal.mutex);
	if(journalcount >= 150){
		journalcount = 0;
	}
	i = journalcount;
	journalcount++;
	pthread_mutex_unlock(&bdev->internal.mutex);*/
	int rc = 0;
	rc = spdk_bdev_write(desc, ch, buf, 0, 512, _pt_complete_io, bdev_io);
	//return 0;
}

static void
write_complete(struct spdk_bdev_io *bdev_io, bool success, void *ch_arg)
{
	struct spdk_bdev_io *orig_io = ch_arg;/*
	struct vbdev_passthru *pt_node = SPDK_CONTAINEROF(orig_io->bdev, struct vbdev_passthru, pt_bdev);
	struct spdk_io_channel *ch = pt_node->bdev_io_channel;
	struct pt_io_channel *pt_ch = spdk_io_channel_get_ctx(ch);
	struct passthru_bdev_io *io_ctx = (struct passthru_bdev_io *)orig_io->driver_ctx;
	spdk_bdev_free_io(bdev_io);*/
	//struct spdk_bdev_io *orig_io = ch_arg;
	/*
	struct vbdev_passthru *pt_node = ch_arg;
	struct spdk_bdev *bdev = pt_node->base_bdev;
	struct spdk_bdev_desc *desc = pt_node->base_desc;
	struct spdk_bdev_io *orig_io = pt_node->orig_IO;
	struct spdk_io_channel *ch = pt_node->pt_channel->base_ch;
	void *buf = pt_node->orig_IO->u.nvme_passthru.buf;*/
	/* journaling 
	uint64_t offset = pt_node->offsets;
	uint64_t nbytes = pt_node->nblocks;
	char *buff;
	buff = spdk_dma_zmalloc(512, 0, NULL);
	j = sprintf(buff,"%d,%d,%d",hash,offset,nbytes);
	printf("journal buf: %s\n",buff);*/
	int i = 0;
	/*
	//uint64_t offset = pt_node->orig_IO->u.nvme_passthru.cmd.cdw10*512;
	//uint64_t nbytes = pt_node->orig_IO->u.nvme_passthru.cmd.cdw12*512;
	struct pt_io_channel *pt_ch = pt_node->pt_channel;
	//spdk_bdev_free_io(bdev_io);
	printf("write_complet!\n");
	//bdev_Journaling(pt_node, pt_node->pt_channel->base_ch);
	//spdk_bdev_write(desc, ,buf,offset*512,nbytes*512,_pt_complete_io, orig_io);
	int hash = pt_node->hash;
	int j,i=0;
	char *buff;
	uint64_t offset = pt_node->offsets;
	uint64_t nbytes = pt_node->nblocks;
	//printf("bdev_journaling start!\n");
	buff = spdk_dma_zmalloc(512, 0, NULL);

	j = sprintf(buff,"%d,%d,%d",hash,offset,nbytes);
	printf("journal buff: %s\n",buff);*/
	//spdk_bdev_write(pt_node->base_desc, pt_ch->base_ch, orig_io->u.nvme_passthru.buf, i*512, 512, _pt_complete_io, orig_io);
	_pt_complete_io(bdev_io, success, orig_io);

}
static void
write_complete4(struct spdk_bdev_io *bdev_io, bool success, void *ch_arg)
{
	struct vbdev_passthru *pt_node = ch_arg;
	int rc;
	spdk_bdev_free_io(bdev_io);
	if (success){
		//printf("bdev : data write completed successfully\n");
	}

	//rc = bdev_journaling(pt_node->base_desc, pt_node->bdev_io_channel, 
	//		pt_node->orig_IO->u.nvme_passthru.buf, 512, 512, write_complete3,pt_node->orig_IO);
	rc = spdk_bdev_write(pt_node->base_desc, pt_node->bdev_io_channel,
				pt_node->orig_IO->u.nvme_passthru.buf, pt_node->orig_IO->u.nvme_passthru.cmd.cdw10*512, pt_node->orig_IO->u.nvme_passthru.cmd.cdw12*512,_pt_complete_io, pt_node->orig_IO);

	if(rc == -ENOMEM) {
	//	printf("Queueing io\n");
		pt_node->bdev_io_wait.bdev = pt_node->base_bdev ;
		pt_node->bdev_io_wait.cb_fn = write_complete4;
		pt_node->bdev_io_wait.cb_arg = pt_node;
		spdk_bdev_queue_io_wait(pt_node->base_bdev, pt_node->bdev_io_channel,
				&pt_node->bdev_io_wait);
	}
	else if(rc){
	//	printf("error while reading from bdev\n");
	}


}
static void
write_complete2(struct spdk_bdev_io *bdev_io, bool success, void *ch_arg)
{
	struct spdk_bdev_io *orig_io = ch_arg;
	struct vbdev_passthru *pt_node = SPDK_CONTAINEROF(orig_io->bdev, struct vbdev_passthru, pt_bdev);
	struct passthru_bdev_io *io_ctx = (struct passthru_bdev_io *)orig_io->driver_ctx;
	struct pt_io_channel *pt_ch = spdk_io_channel_get_ctx(bdev_io->internal.ch);
	struct spdk_io_channel *io_channel;
	int status = success ? SPDK_BDEV_IO_STATUS_SUCCESS : SPDK_BDEV_IO_STATUS_FAILED;
	
	struct bdev_names *name;
	struct vbdev_passthru *pt_node2;
	struct spdk_bdev *bdev;
	int rc = 0;
	
	spdk_bdev_io_complete(orig_io, status);
	spdk_bdev_free_io(bdev_io);

	pt_node = calloc(1, sizeof(struct vbdev_passthru));
	//pt_node->pt_bdev.name = strdup(name->vbdev_name);
	//pt_node->pt_bdev.name = "use";
	rc = spdk_bdev_open_ext("Nvme0n1", true, vbdev_passthru_base_bdev_event_cb, NULL, &pt_node->base_desc);
	pt_node->base_bdev = spdk_bdev_desc_get_bdev(pt_node->base_desc);
	
	pt_node->bdev_io_channel = spdk_bdev_get_io_channel(pt_node->base_desc);
	char *buff;
	int offset_block = 0;
	snprintf(buff,20,"hello world\n");
	//printf("offset-bdev:%d\n",bdev_io->u.bdev.offset_blocks);
	offset_block = bdev_io->u.bdev.offset_blocks;
	//_pt_complete_io(bdev_io, success, orig_io);
	spdk_bdev_write(pt_node->base_desc, pt_node->bdev_io_channel,
				buff, offset_block*512, 512, _pt_complete_io, pt_node->base_bdev);
	//bdev_journaling(pt_node2->base_desc, pt_node2->bdev_io_channel,
	//buff, offset_block*512, 512,write_complete3, orig_io);
	

}


static void
_pt_complete_zcopy_io(struct spdk_bdev_io *bdev_io, bool success, void *cb_arg)
{
	struct spdk_bdev_io *orig_io = cb_arg;
	int status = success ? SPDK_BDEV_IO_STATUS_SUCCESS : SPDK_BDEV_IO_STATUS_FAILED;
	struct passthru_bdev_io *io_ctx = (struct passthru_bdev_io *)orig_io->driver_ctx;

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
vbdev_passthru_resubmit_io(void *arg)
{
	struct spdk_bdev_io *bdev_io = (struct spdk_bdev_io *)arg;
	struct passthru_bdev_io *io_ctx = (struct passthru_bdev_io *)bdev_io->driver_ctx;

	vbdev_passthru_submit_request(io_ctx->ch, bdev_io);
}

static void
vbdev_passthru_queue_io(struct spdk_bdev_io *bdev_io)
{
	struct passthru_bdev_io *io_ctx = (struct passthru_bdev_io *)bdev_io->driver_ctx;
	struct pt_io_channel *pt_ch = spdk_io_channel_get_ctx(io_ctx->ch);
	int rc;

	io_ctx->bdev_io_wait.bdev = bdev_io->bdev;
	io_ctx->bdev_io_wait.cb_fn = vbdev_passthru_resubmit_io;
	io_ctx->bdev_io_wait.cb_arg = bdev_io;

	/* Queue the IO using the channel of the base device. */
	rc = spdk_bdev_queue_io_wait(bdev_io->bdev, pt_ch->base_ch, &io_ctx->bdev_io_wait);
	if (rc != 0) {
		SPDK_ERRLOG("Queue io failed in vbdev_passthru_queue_io, rc=%d.\n", rc);
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
	struct vbdev_passthru *pt_node = SPDK_CONTAINEROF(bdev_io->bdev, struct vbdev_passthru,
					 pt_bdev);
	struct pt_io_channel *pt_ch = spdk_io_channel_get_ctx(ch);
	struct passthru_bdev_io *io_ctx = (struct passthru_bdev_io *)bdev_io->driver_ctx;
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
			SPDK_ERRLOG("No memory, start to queue io for passthru.\n");
			io_ctx->ch = ch;
			vbdev_passthru_queue_io(bdev_io);
		} else {
			SPDK_ERRLOG("ERROR on bdev_io submission!\n");
			spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
		}
	}
}

static int
bdev_KV(struct vbdev_passthru *pt_node, struct spdk_io_channel *ch)
{
	//struct spdk_bdev *bdev = pt_node->base_bdev;
	//struct spdk_io_channel *ch = pt_node->bdev_io_channel; 
	struct spdk_bdev_desc *desc = pt_node->base_desc;
	struct spdk_bdev *bdev = spdk_bdev_desc_get_bdev(desc);
	struct spdk_bdev_io *bdev_io = pt_node->orig_IO;
	void *buf = pt_node->orig_IO->u.nvme_passthru.buf;
	uint64_t offset = pt_node->orig_IO->u.nvme_passthru.cmd.cdw10;
	uint64_t nbytes = pt_node->orig_IO->u.nvme_passthru.cmd.cdw12;

	int bit=0, cel = 0,t = 0,ii=0, tb=0;
	int a=0, b = 0,i=0,j=0,num, bucket,has;
	uint64_t *pi;
	uint64_t hash1 = 0;
	int rc = 0;
	char *buff;
	int blk_size = 512;
	int buf_align = 0;
	buff = spdk_dma_zmalloc(blk_size, buf_align, NULL);

	int hashint, entry;
	/************************** hash : SHA1******************************/
	unsigned char digest[SHA_DIGEST_LENGTH];
	char mdString[10];
	char string[512];

	sprintf(string,"%ld", offset);
	//printf("string:%s,%x\n",string,*string);
	SHA1((unsigned char*)string, strlen(string), (unsigned char*)&digest);
	for(ii=0;ii<SHA_DIGEST_LENGTH/5;ii++){
		sprintf(&mdString[ii*2], "%02x", (unsigned int)digest[ii]);
	}
	pi = (int*)mdString;

	//snprintf(buff,10,"%d",*mdString);
	//printf("translate buff:%s\n",buff);
	/********************************************************************/


	/************************ HASH TABLE 삽입****************************/
	cel = nbytes/512+1;
	hashint = *pi;
	has = hashint & 15;
	entry = (hashint & 65520)/16;
	//printf("hashint:%d,%x\n",hashint,hashint);
	//node 할당 및 초기화
	struct _node *item = (struct _node*)malloc(sizeof(struct _node));
	//item->hash = (hashint & 4294901760)>>16;
	pt_node->hash = hashint;
	pt_node->nblocks = nbytes;
	item->hash = hashint;
	item->next = NULL;
	item->address = 0;
	
	//array[entry] 에 가장 먼저 진입하는 경우
	if(has==0 || has==5){
	printf("0,5 has:%d,hash:%d\n",has,hashint);
	pthread_mutex_lock(&bdev->internal.mutex);
	if(arraycheck[entry] == 0){
		arraycheck[entry] = 1;
		if(array_ == NULL){
			array_ = (struct arrayitem*)malloc(4096*sizeof(struct arrayitem*));
		}
		array_[entry].head = item;
		array_[entry].tail = item;
		pthread_mutex_unlock(&bdev->internal.mutex);
	}
	else{//entry에 가장먼저 진입하지 않고, 
		pthread_mutex_unlock(&bdev->internal.mutex);
		struct _node *tem = array_[entry].head;
		while(tem!=NULL){
			if(tem->hash==item->hash){//이미 존재하는 hash node인 경우
				printf("0,5write-hashint:%X,:has:%X,hash1:%X,address:%X\n",hashint,has,item->hash,tem->address);
				//rc = spdk_bdev_write(desc,ch,buf,tem->address*4096,cel*4096,write_complete,bdev_io);
				pt_node->offsets = tem->address*8;
				rc = spdk_bdev_write(desc,ch,buf,tem->address*4096,cel*4096,write_complete,bdev_io);
				free(item);
				return 0;
			}
			tem = tem->next;
		}
		//존재하는 hash node 없는 경우
		pthread_mutex_lock(&bdev->internal.mutex);
		array_[entry].tail->next = item;
		array_[entry].tail = item;
		pthread_mutex_unlock(&bdev->internal.mutex);
	}
	}
	else if(has==1 ||has==6){
			printf("1,6 has:%d,hash:%d\n",has,hashint);
			pthread_mutex_lock(&bdev->internal.mutex);
			if(arraycheck1[entry] == 0){
			arraycheck1[entry] = 1;
			if(array1 == NULL){
				array1 = (struct arrayitem*)malloc(4096*sizeof(struct arrayitem*));
			}
			array1[entry].head = item;
			array1[entry].tail = item;
			pthread_mutex_unlock(&bdev->internal.mutex);
		}
		else{//entry에 가장먼저 진입하지 않고, 
			pthread_mutex_unlock(&bdev->internal.mutex);
			struct _node *tem = array1[entry].head;
			while(tem!=NULL){
				if(tem->hash==item->hash){//이미 존재하는 hash node인 경우
					printf("1,6write-hashint:%X,:has:%X,hash1:%X,address:%X\n",hashint,has,item->hash,tem->address);
					pt_node->offsets = tem->address*8;
					//rc = spdk_bdev_writdesc,ch,buf,tem->address*4096,cel*4096,write_complete,bdev_io);
					rc = spdk_bdev_write(desc,ch,buf,tem->address*4096,cel*4096,write_complete,bdev_io);
					free(item);
					return 0;
				}
				tem = tem->next;
			}
		pthread_mutex_lock(&bdev->internal.mutex);
		array1[entry].tail->next = item;
		array1[entry].tail = item;
		pthread_mutex_unlock(&bdev->internal.mutex);
	}
	}
	else if(has==2 ||has==7){
			printf("2,7 has:%d,key:%d\n",has,hashint);
			
			pthread_mutex_lock(&bdev->internal.mutex);
			if(arraycheck2[entry] == 0){
			arraycheck2[entry] = 1;
			if(array2 == NULL){
				array2 = (struct arrayitem*)malloc(4096*sizeof(struct arrayitem*));
			}
			array2[entry].head = item;
			array2[entry].tail = item;
			pthread_mutex_unlock(&bdev->internal.mutex);
			}
			else{//entry에 가장먼저 진입하지 않고, 
			pthread_mutex_unlock(&bdev->internal.mutex);
			struct _node *tem = array2[entry].head;
			while(tem!=NULL){
				if(tem->hash==item->hash){//이미 존재하는 hash node인 경우
					printf("2,7write-hashint:%X,has:%X,hash1:%X,address:%X\n",hashint,has,item->hash,tem->address);
					pt_node->offsets = tem->address*8;
					//rc = spdk_bdev_write(desc,ch,buf,tem->address*4096,cel*4096,write_complete,bdev_io);
					rc = spdk_bdev_write(desc,ch,buf,tem->address*4096,cel*4096,write_complete,bdev_io);
					free(item);
					return 0;
				}
				tem = tem->next;
			}
		//존재하는 hash node 없는 경우
		pthread_mutex_lock(&bdev->internal.mutex);
		array2[entry].tail->next = item;
		array2[entry].tail = item;
		pthread_mutex_unlock(&bdev->internal.mutex);
	}
	}
	else if(has==3 || has==8){
		
			printf("3,8 has:%d,key:%d\n",has,hashint);
		pthread_mutex_lock(&bdev->internal.mutex);
			if(arraycheck3[entry] == 0){
			arraycheck3[entry] = 1;
			if(array3 == NULL){
				array3 = (struct arrayitem*)malloc(4096*sizeof(struct arrayitem*));
			}
			array3[entry].head = item;
			array3[entry].tail = item;
			pthread_mutex_unlock(&bdev->internal.mutex);
	}
	else{//entry에 가장먼저 진입하지 않고, 
			pthread_mutex_unlock(&bdev->internal.mutex);
			struct _node *tem = array3[entry].head;
			while(tem!=NULL){
				if(tem->hash==item->hash){//이미 존재하는 hash node인 경우
					printf("3,8write-hashint:%X,has:%X,hash1:%X,address:%X\n",hashint,has,item->hash,tem->address);
					pt_node->offsets = tem->address*8;
					//rc = spdk_bdev_write(desc,ch,buf,tem->address*4096,cel*4096,write_complete,bdev_io);
					rc = spdk_bdev_write(desc,ch,buf,tem->address*4096,cel*4096,write_complete,bdev_io);
					free(item);
					return 0;
				}
				tem = tem->next;
			}
		//존재하는 hash node 없는 경우
		pthread_mutex_lock(&bdev->internal.mutex);
		array3[entry].tail->next = item;
		array3[entry].tail = item;
		pthread_mutex_unlock(&bdev->internal.mutex);
	}
	}
	else{
			printf("4,9 has:%d,key:%d\n",has,hashint);
		
		pthread_mutex_lock(&bdev->internal.mutex);
			if(arraycheck4[entry] == 0){
			arraycheck4[entry] = 1;
			if(array4 == NULL){
				array4 = (struct arrayitem*)malloc(4096*sizeof(struct arrayitem*));
			}
			array4[entry].head = item;
			array4[entry].tail = item;
			pthread_mutex_unlock(&bdev->internal.mutex);
	}
	else{//entry에 가장먼저 진입하지 않고, 
			pthread_mutex_unlock(&bdev->internal.mutex);
			struct _node *tem = array4[entry].head;
			while(tem!=NULL){
				if(tem->hash==item->hash){//이미 존재하는 hash node인 경우
					printf("4,9write-hashint:%X,has:%X,hash1:%X,address:%X\n",hashint,has,item->hash,tem->address);
					//rc = spdk_bdev_write(desc,ch,buf,tem->address*4096,cel*4096,write_complete,bdev_io);
					pt_node->offsets = tem->address*8;
					rc = spdk_bdev_write(desc,ch,buf,tem->address*4096,cel*4096,write_complete,bdev_io);
					free(item);
					return 0;
				}
				tem = tem->next;
			}
		//존재하는 hash node 없는 경우
		pthread_mutex_lock(&bdev->internal.mutex);
		array4[entry].tail->next = item;
		array4[entry].tail = item;
		pthread_mutex_unlock(&bdev->internal.mutex);
	
	}
	}
	/*********************************************************************/

	/************************ Page Allocation ****************************/
	t=0;
	t= (1<<(cel))-1;
	pthread_mutex_lock(&bdev->internal.mutex);
	a = chec >> 5;
	b = chec & 31;
	if((cel + b) > 32){
		bitvec[a] |= t <<b;
		bitvec[a+1] |= (1<<(cel+b-32))-1;
		//printf("f_bitvec[a] = %X, bitvec[a+1] = %X\n",bitvec[a],bitvec[a+1]);
	}
	else{
		//printf("normal:bitvec[a] =%X\n",bitvec[a]); 
		bitvec[a] |= t<<b;
		//printf("f_normal:bitvec[a] =%X\n",bitvec[a]); 
	}
	item->address = chec;
	chec += cel;
	pthread_mutex_unlock(&bdev->internal.mutex);
	printf("*******hashint:%X,write:has:%X,hash1:%X,address:%X\n",hashint,has,item->hash,item->address);
	pt_node->offsets = item->address*8;
	
	//rc = spdk_bdev_write(desc,ch,buf,(item->address)*4096,cel*4096,write_complete,bdev_io);
	rc = spdk_bdev_write(desc,ch,buf,(item->address)*4096,cel*4096,write_complete,bdev_io);

	return 0;
}

/* Called when someone above submits IO to this pt vbdev. We're simply passing it on here
 * via SPDK IO calls which in turn allocate another bdev IO and call our cpl callback provided
 * below along with the original bdev_io so that we can complete it once this IO completes.
 */
static void
vbdev_passthru_submit_request(struct spdk_io_channel *ch, struct spdk_bdev_io *bdev_io)
{
	struct vbdev_passthru *pt_node = SPDK_CONTAINEROF(bdev_io->bdev, struct vbdev_passthru, pt_bdev);
	struct pt_io_channel *pt_ch = spdk_io_channel_get_ctx(ch);
	struct passthru_bdev_io *io_ctx = (struct passthru_bdev_io *)bdev_io->driver_ctx;
	struct spdk_bdev_io *bdev_io2, *bdev_io3;
	
	pt_node->bdev_io_channel = ch;
	pt_node->orig_IO = bdev_io;
	pt_node->pt_channel = pt_ch;
	time_t t;
	struct tm *lt;
	struct timeval tv;
	int rc = 0;

	clock_t start = clock();

	//printf("vbdev_passthru_submit_request_ch:%p,pt_ch->base_ch:%p\n",ch,pt_ch->base_ch);
	/*
	if((t = gettimeofday(&tv, NULL)) == -1) {
		perror("gettimeofday() call error");
	}
	if((lt = localtime(&tv.tv_sec)) == NULL) {
		perror("localtime() call error");
	}
	
	printf("sta 시간 : %04d-%02d-%02d %02d:%02d:%02d.%06d\n",
			lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
			lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec);

	int rc = 0;
	*/
	/* Setup a per IO context value; we don't do anything with it in the vbdev other
	 * than confirm we get the same thing back in the completion callback just to
	 * demonstrate.
	 */
	io_ctx->test = 0x5a;
	//io_ctx->ch = ch;
	switch (bdev_io->type) {
	case SPDK_BDEV_IO_TYPE_READ:
			//printf("READ_opcode:%u,offset_block(cdw10):%u,num_block(cdw12):%u\n",
			//		bdev_io->u.nvme_passthru.cmd.opc,bdev_io->u.nvme_passthru.cmd.cdw10,bdev_io->u.nvme_passthru.cmd.cdw12);
			/*
			printf("opcode:%u,fuse:%u,rsvd%u,psdt%u,nsid%u,rsvd2%u,rsvd3%u,offset_block(cdw10):%u,num_block(cdw12):%u\n",
					bdev_io->u.nvme_passthru.cmd.opc,bdev_io->u.nvme_passthru.cmd.fuse,
					bdev_io->u.nvme_passthru.cmd.rsvd1,bdev_io->u.nvme_passthru.cmd.psdt,
					bdev_io->u.nvme_passthru.cmd.nsid,bdev_io->u.nvme_passthru.cmd.rsvd2,
					bdev_io->u.nvme_passthru.cmd.rsvd3,
					bdev_io->u.nvme_passthru.cmd.cdw10,bdev_io->u.nvme_passthru.cmd.cdw12);*/
		//printf("********read-offset_block, num_block:%u,%u\n",bdev_io->u.bdev.offset_blocks, bdev_io->u.bdev.num_blocks);
		spdk_bdev_io_get_buf(bdev_io, pt_read_get_buf_cb,
				     bdev_io->u.bdev.num_blocks * bdev_io->bdev->blocklen);
		break;
	case SPDK_BDEV_IO_TYPE_WRITE:
			//printf("WRITE_opcode:%u,offset_block(cdw10):%u,num_block(cdw12):%u\n",
			//		bdev_io->u.nvme_passthru.cmd.opc,bdev_io->u.nvme_passthru.cmd.cdw10,bdev_io->u.nvme_passthru.cmd.cdw12);
		
		/*
		printf("sta 시간 : %04d-%02d-%02d %02d:%02d:%02d.%06d\n",
			lt->tm_year + 1900, lt->tm_mon + 1, lt->tm_mday,
			lt->tm_hour, lt->tm_min, lt->tm_sec, tv.tv_usec);*/
		//printf("********write-offset_block, num_block:%u,%u\n",bdev_io->u.bdev.offset_blocks, bdev_io->u.bdev.num_blocks);
		
		if (bdev_io->u.bdev.md_buf == NULL) {
			//bdev_chunk(pt_node->base_desc, pt_ch->base_ch,
			//	bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw10*512, bdev_io->u.nvme_passthru.cmd.cdw12*512,write_complete2, bdev_io);
		
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
		if(bdev_io->u.nvme_passthru.cmd.opc==0xA2){
			printf("opcode:%u,offset_block(cdw10):%u,num_block(cdw12):%u\n",
					bdev_io->u.nvme_passthru.cmd.opc,bdev_io->u.nvme_passthru.cmd.cdw10,bdev_io->u.nvme_passthru.cmd.cdw12);
			//buffer 잘 비어있는지 확인하기
			//printf("buf:%s,%d\n",bdev_io->u.nvme_passthru.buf,bdev_io->u.nvme_passthru.buf);
			//read command 보내기
			spdk_bdev_read(pt_node->base_desc, pt_ch->base_ch,
				bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw10*512, bdev_io->u.nvme_passthru.cmd.cdw12*512,_pt_complete_io, bdev_io);
		}
		else if(bdev_io->u.nvme_passthru.cmd.opc==0xC2){
		//	printf("C2_opcode:%u,offset_block(cdw10):%u,num_block(cdw12):%u\n",
		//			bdev_io->u.nvme_passthru.cmd.opc,bdev_io->u.nvme_passthru.cmd.cdw10,bdev_io->u.nvme_passthru.cmd.cdw12);
			bdev_add_search(pt_node->base_desc, pt_ch->base_ch,
				bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw10, bdev_io->u.nvme_passthru.cmd.cdw12*512,_pt_complete_io, bdev_io);

		}
		else if(bdev_io->u.nvme_passthru.cmd.opc==0xB1){
			/*printf("opcode:%u,fuse:%u,rsvd%u,psdt%u,nsid%u,rsvd2%u,rsvd3%u,offset_block(cdw10):%u,num_block(cdw12):%u\n",
					bdev_io->u.nvme_passthru.cmd.opc,bdev_io->u.nvme_passthru.cmd.fuse,
					bdev_io->u.nvme_passthru.cmd.rsvd1,bdev_io->u.nvme_passthru.cmd.psdt,
					bdev_io->u.nvme_passthru.cmd.nsid,bdev_io->u.nvme_passthru.cmd.rsvd2,
					bdev_io->u.nvme_passthru.cmd.rsvd3,
					bdev_io->u.nvme_passthru.cmd.cdw10,bdev_io->u.nvme_passthru.cmd.cdw12);*/
			bdev_chunk(pt_node->base_desc, pt_ch->base_ch,
				bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw10*512, bdev_io->u.nvme_passthru.cmd.cdw12*512,write_complete2, bdev_io);
		}
		else if(bdev_io->u.nvme_passthru.cmd.opc==0xA0){
			printf("A0_opcode:%u,key(cdw10):%u,num_block(cdw12):%u\n",bdev_io->u.nvme_passthru.cmd.opc,bdev_io->u.nvme_passthru.cmd.cdw10,bdev_io->u.nvme_passthru.cmd.cdw12);
			
			bdev_add_translate(pt_node->base_desc, pt_ch->base_ch,
				bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw10*512, bdev_io->u.nvme_passthru.cmd.cdw12*512,write_complete2, bdev_io);
		}
		else if(bdev_io->u.nvme_passthru.cmd.opc==0xC1){
/*
		pthread_mutex_lock(&(bdev_io->bdev)->internal.mutex);
		Result = Result - clock();
		pthread_mutex_unlock(&(bdev_io->bdev)->internal.mutex);*/

		//	printf("C1_opcode:%u,key(cdw10):%u,num_block(cdw12):%u\n",bdev_io->u.nvme_passthru.cmd.opc,bdev_io->u.nvme_passthru.cmd.cdw10,bdev_io->u.nvme_passthru.cmd.cdw12);

//			printf("offset_block(cdw10):%u,num_block(cdw12):%u\n",	
//					bdev_io->u.nvme_passthru.cmd.cdw10,bdev_io->u.nvme_passthru.cmd.cdw12);/*
//			printf("orig_IO:offset_block(cdw10):%u,num_block(cdw12):%u\n",	
//					pt_node->orig_IO->u.nvme_passthru.cmd.cdw10,pt_node->orig_IO->u.nvme_passthru.cmd.cdw12);*/
			/* write_complete2로 받을 때, callback 시 bdev_io 전달 */
			//bdev_KV(pt_node,pt_ch->base_ch);
			//bdev_Journaling(pt_node,pt_ch->base_ch);
			//printf("finished bdev_KV\n");
			bdev_add_translate(pt_node->base_desc, pt_ch->base_ch,
			bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw10*512, bdev_io->u.nvme_passthru.cmd.cdw12*512,_pt_complete_io, bdev_io);
		//	bdev_add_translate(pt_node->base_desc, pt_ch->base_ch,
		//	bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw10*512, bdev_io->u.nvme_passthru.cmd.cdw12*512,_pt_complete_io, bdev_io);
			//bdev_add_translate(pt_node->base_desc, pt_ch->base_ch,
			//	bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw10, bdev_io->u.nvme_passthru.cmd.cdw12*512,write_complete3, bdev_io);
			/* write_complete로 받을 때, callback 시 pt_node 전달 */
			//bdev_add_translate(pt_node->base_desc, pt_ch->base_ch,
			//	bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw10, bdev_io->u.nvme_passthru.cmd.cdw12*512,write_complete, pt_node);
			//bdev_journaling(pt_node->base_desc, pt_ch->base_ch,
			//	bdev_io->u.nvme_passthru.buf, bdev_io->u.nvme_passthru.cmd.cdw10*512, 512,write_complete3, bdev_io);
		}
		break;
	default:
		SPDK_ERRLOG("passthru: unknown I/O type %d\n", bdev_io->type);
		spdk_bdev_io_complete(bdev_io, SPDK_BDEV_IO_STATUS_FAILED);
		return;
	}
	if (rc != 0) {
		if (rc == -ENOMEM) {
			SPDK_ERRLOG("No memory, start to queue io for passthru.\n");
			io_ctx->ch = ch;
			vbdev_passthru_queue_io(bdev_io);
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
vbdev_passthru_io_type_supported(void *ctx, enum spdk_bdev_io_type io_type)
{
	struct vbdev_passthru *pt_node = (struct vbdev_passthru *)ctx;

	return spdk_bdev_io_type_supported(pt_node->base_bdev, io_type);
}

/* We supplied this as an entry point for upper layers who want to communicate to this
 * bdev.  This is how they get a channel. We are passed the same context we provided when
 * we created our PT vbdev in examine() which, for this bdev, is the address of one of
 * our context nodes. From here we'll ask the SPDK channel code to fill out our channel
 * struct and we'll keep it in our PT node.
 */
static struct spdk_io_channel *
vbdev_passthru_get_io_channel(void *ctx)
{
	struct vbdev_passthru *pt_node = (struct vbdev_passthru *)ctx;
	struct spdk_io_channel *pt_ch = NULL;

	/* The IO channel code will allocate a channel for us which consists of
	 * the SPDK channel structure plus the size of our pt_io_channel struct
	 * that we passed in when we registered our IO device. It will then call
	 * our channel create callback to populate any elements that we need to
	 * update.
	 */
//	printf("vbdev_passthru_get_io_channel.*************************************************************************\n");
	pt_ch = spdk_get_io_channel(pt_node);

	return pt_ch;
}

/* This is the output for bdev_get_bdevs() for this vbdev */
static int
vbdev_passthru_dump_info_json(void *ctx, struct spdk_json_write_ctx *w)
{
	struct vbdev_passthru *pt_node = (struct vbdev_passthru *)ctx;

	spdk_json_write_name(w, "passthru_external");
	spdk_json_write_object_begin(w);
	spdk_json_write_named_string(w, "name", spdk_bdev_get_name(&pt_node->pt_bdev));
	spdk_json_write_named_string(w, "base_bdev_name", spdk_bdev_get_name(pt_node->base_bdev));
	spdk_json_write_object_end(w);

	return 0;
}

/* This is used to generate JSON that can configure this module to its current state. */
static int
vbdev_passthru_config_json(struct spdk_json_write_ctx *w)
{
	struct vbdev_passthru *pt_node;

	TAILQ_FOREACH(pt_node, &g_pt_nodes, link) {
		spdk_json_write_object_begin(w);
		spdk_json_write_named_string(w, "method", "construct_ext_passthru_bdev");
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
	struct vbdev_passthru *pt_node = io_device;

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

/* Create the passthru association from the bdev and vbdev name and insert
 * on the global list. */
static int
vbdev_passthru_insert_name(const char *bdev_name, const char *vbdev_name)
{
	struct bdev_names *name;

	TAILQ_FOREACH(name, &g_bdev_names, link) {
		if (strcmp(vbdev_name, name->vbdev_name) == 0) {
			SPDK_ERRLOG("passthru bdev %s already exists\n", vbdev_name);
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
vbdev_passthru_init(void)
{
	return 0;
}

/* Called when the entire module is being torn down. */
static void
vbdev_passthru_finish(void)
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
vbdev_passthru_get_ctx_size(void)
{
	return sizeof(struct passthru_bdev_io);
}

/* Where vbdev_passthru_config_json() is used to generate per module JSON config data, this
 * function is called to output any per bdev specific methods. For the PT module, there are
 * none.
 */
static void
vbdev_passthru_write_config_json(struct spdk_bdev *bdev, struct spdk_json_write_ctx *w)
{
	/* No config per bdev needed */
}

/* When we register our bdev this is how we specify our entry points. */
static const struct spdk_bdev_fn_table vbdev_passthru_fn_table = {
	.destruct		= vbdev_passthru_destruct,
	.submit_request		= vbdev_passthru_submit_request,
	.io_type_supported	= vbdev_passthru_io_type_supported,
	.get_io_channel		= vbdev_passthru_get_io_channel,
	.dump_info_json		= vbdev_passthru_dump_info_json,
	.write_config_json	= vbdev_passthru_write_config_json,
};



/* Create and register the passthru vbdev if we find it in our list of bdev names.
 * This can be called either by the examine path or RPC method.
 */
static int
vbdev_passthru_register(const char *bdev_name)
{
	struct bdev_names *name;
	struct vbdev_passthru *pt_node;
	struct spdk_bdev *bdev;
	int rc = 0;

	/* Check our list of names from config versus this bdev and if
	 * there's a match, create the pt_node & bdev accordingly.
	 */
	TAILQ_FOREACH(name, &g_bdev_names, link) {
		if (strcmp(name->bdev_name, bdev_name) != 0) {
			continue;
		}

		SPDK_NOTICELOG("Match on %s\n", bdev_name);
		pt_node = calloc(1, sizeof(struct vbdev_passthru));
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
		pt_node->pt_bdev.product_name = "passthru";

		/* The base bdev that we're attaching to. */
//		printf("*****************************************bdev_name:%s\n",bdev_name);
		rc = spdk_bdev_open_ext(bdev_name, true, vbdev_passthru_base_bdev_event_cb,
					NULL, &pt_node->base_desc);
		if (rc) {
			if (rc != -ENODEV) {
				SPDK_ERRLOG("could not open bdev %s\n", bdev_name);
			}
			free(pt_node->pt_bdev.name);
			free(pt_node);
			break;
		}
		SPDK_NOTICELOG("base_bdev opened\n");
	
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
		pt_node->pt_bdev.fn_table = &vbdev_passthru_fn_table;
		pt_node->pt_bdev.module = &passthru_if;
		TAILQ_INSERT_TAIL(&g_pt_nodes, pt_node, link);

		spdk_io_device_register(pt_node, pt_bdev_ch_create_cb, pt_bdev_ch_destroy_cb,
					sizeof(struct pt_io_channel),
					name->vbdev_name);
		SPDK_NOTICELOG("io_device created at: 0x%p\n", pt_node);

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
		SPDK_NOTICELOG("bdev claimed\n");

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
		SPDK_NOTICELOG("ext_pt_bdev registered\n");
		SPDK_NOTICELOG("created ext_pt_bdev for: %s\n", name->vbdev_name);
	}

	return rc;
}

/* Create the passthru disk from the given bdev and vbdev name. */
int
bdev_passthru_external_create_disk(const char *bdev_name, const char *vbdev_name)
{
	int rc;

	/* Insert the bdev name into our global name list even if it doesn't exist yet,
	 * it may show up soon...
	 */
	rc = vbdev_passthru_insert_name(bdev_name, vbdev_name);
	if (rc) {
		return rc;
	}

	rc = vbdev_passthru_register(bdev_name);
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
bdev_passthru_external_delete_disk(struct spdk_bdev *bdev, spdk_bdev_unregister_cb cb_fn,
				   void *cb_arg)
{
	struct bdev_names *name;

	if (!bdev || bdev->module != &passthru_if) {
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
vbdev_passthru_examine(struct spdk_bdev *bdev)
{
	printf("vbdev_passthru_register started!i:%s***************************\n",bdev->name);
	vbdev_passthru_register(bdev->name);

	spdk_bdev_module_examine_done(&passthru_if);
}
