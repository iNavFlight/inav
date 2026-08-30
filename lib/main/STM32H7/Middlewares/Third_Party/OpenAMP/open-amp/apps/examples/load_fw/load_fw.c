/*
 * Load firmware example
 *
 * Copyright(c) 2018 Xilinx Ltd.
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <metal/io.h>
#include <metal/sys.h>
#include <openamp/remoteproc.h>
#include <openamp/remoteproc_loader.h>
#include <stdarg.h>
#include <stdio.h>
/* Xilinx headers */
#include <pm_api_sys.h>
#include <pm_defs.h>
#include <xil_printf.h>

#define LPRINTF(format, ...) xil_printf(format, ##__VA_ARGS__)
//#define LPRINTF(format, ...)
#define LPERROR(format, ...) LPRINTF("ERROR: " format, ##__VA_ARGS__)

extern struct remoteproc_ops r5_rproc_ops;
extern struct image_store_ops mem_image_store_ops;

struct mem_file {
	const void *base;
};

static struct mem_file image = {
	.base = (void *)0x3ED00000,
};

static XIpiPsu IpiInst;

static XStatus IpiConfigure(XIpiPsu *const IpiInstPtr)
{
	XStatus Status;
	XIpiPsu_Config *IpiCfgPtr;

	/* Look Up the config data */
	IpiCfgPtr = XIpiPsu_LookupConfig(XPAR_XIPIPSU_0_DEVICE_ID);
	if (NULL == IpiCfgPtr) {
		Status = XST_FAILURE;
		LPERROR("%s ERROR in getting CfgPtr\n", __func__);
		return Status;
	}

	/* Init with the Cfg Data */
	Status = XIpiPsu_CfgInitialize(IpiInstPtr, IpiCfgPtr,
				       IpiCfgPtr->BaseAddress);
	if (XST_SUCCESS != Status) {
		LPERROR("%s ERROR #%d in configuring IPI\n", __func__, Status);
		return Status;
	}
	return Status;
}

static void app_log_handler(enum metal_log_level level,
			       const char *format, ...)
{
	char msg[1024];
	va_list args;
	static const char *level_strs[] = {
		"metal: emergency: ",
		"metal: alert:     ",
		"metal: critical:  ",
		"metal: error:     ",
		"metal: warning:   ",
		"metal: notice:    ",
		"metal: info:      ",
		"metal: debug:     ",
	};

	va_start(args, format);
	vsnprintf(msg, sizeof(msg), format, args);
	va_end(args);

	if (level <= METAL_LOG_EMERGENCY || level > METAL_LOG_DEBUG)
		level = METAL_LOG_EMERGENCY;

	xil_printf("%s%s", level_strs[level], msg);
}

int load_exectuable_block(struct remoteproc *rproc,
			  struct image_store_ops *store_ops, void *store,
			  const char *img_path)
{
	int ret;

	(void)img_path;
	if (rproc == NULL)
		return -EINVAL;
	/* Configure remoteproc to get ready to load executable */
	remoteproc_config(rproc, NULL);
	/* Load remoteproc executable */
	LPRINTF("Start to load executable with remoteproc_load() \r\n");
	ret = remoteproc_load(rproc, NULL, store, store_ops, NULL);
	if (ret) {
		LPRINTF("failed to load firmware\r\n");
		return ret;
	}
	/* Start the processor */
	ret = remoteproc_start(rproc);
	if (ret) {
		LPRINTF("failed to start processor\r\n");
		return ret;
	}
	LPRINTF("successfully started the processor\r\n");
	/* ... */
	asm volatile("wfi");
	LPRINTF("going to stop the processor\r\n");
	remoteproc_stop(rproc);
	/* application may want to do some cleanup before shutdown */
	LPRINTF("going to shutdown the processor\r\n");
	remoteproc_shutdown(rproc);
	return 0;
}

int load_exectuable_noblock(struct remoteproc *rproc,
			     struct image_store_ops *store_ops, void *store,
			     const char *img_path)
{
	int ret;
	const void *img_data;
	void *img_info = NULL;
	metal_phys_addr_t pa;
	struct metal_io_region *io;
	size_t offset, noffset;
	size_t len, nlen, nmlen;
	unsigned char padding;

	if (rproc == NULL)
		return -EINVAL;
	/* Configure remoteproc to get ready to load executable */
	remoteproc_config(rproc, NULL);
	/* Load remoteproc executable */
	LPRINTF("Start to load executable with remoteproc_load() \r\n");
	ret = store_ops->open(store, img_path, &img_data);
	if (ret <= 0)
		return -EINVAL;
	offset = 0;
	len = (size_t)ret;
	do {
		nlen = 0;
		pa = METAL_BAD_PHYS;
		io = NULL;
		nmlen = 0;
		LPRINTF("%s, loading 0x%lx,0x%lx\r\n",
			 __func__, offset, len);
		ret = remoteproc_load_noblock(rproc, img_data, offset, len,
					      &img_info, &pa, &io, &noffset,
					      &nlen, &nmlen, &padding);
		if (ret) {
			LPERROR("failed to load executable, 0x%lx,0x%lx\r\n",
				offset, len);
			return ret;
		}
		if (nlen == 0)
			break;
		offset = noffset;
		len = nlen;
		ret = store_ops->load(store, noffset, nlen, &img_data, pa,
				      io, 1);
		if (ret != (int)nlen) {
			LPERROR("failed to load data to memory, 0x%lx,0x%lx\r\n",
				noffset, nlen);
			return ret;
		}
		if (nmlen > nlen && io != NULL) {
			/* pad the rest of the memory with 0 */
			size_t tmpoffset;

			tmpoffset = metal_io_phys_to_offset(io, pa + nlen);
			metal_io_block_set(io, tmpoffset, padding,
					   (nmlen - nlen));

		}
	} while(1);

	/* Start the processor */
	ret = remoteproc_start(rproc);
	if (ret) {
		LPRINTF("failed to start processor\r\n");
		return ret;
	}
	LPRINTF("successfully started the processor\r\n");
	/* ... */
	asm volatile("wfi");
	LPRINTF("going to stop the processor\r\n");
	remoteproc_stop(rproc);
	/* application may want to do some cleanup before shutdown */
	LPRINTF("going to shutdown the processor\r\n");
	remoteproc_shutdown(rproc);
	return 0;
}


int main(void)
{
	struct remoteproc rproc;
	struct remoteproc *ret_rproc;
	void *store = &image;
	unsigned int cpu_id = NODE_RPU_1;
	int ret;
	struct metal_init_params metal_param = {
		.log_handler = app_log_handler,
		.log_level = METAL_LOG_DEBUG,
	};

	if (XST_SUCCESS != IpiConfigure(&IpiInst)) {
		LPERROR("Failed to config IPI instance\r\n");
		return -1;
	}
	if (XST_SUCCESS != XPm_InitXilpm(&IpiInst)) {
		LPERROR("Failed to initialize PM\r\n");
		return -1;
	}

	LPRINTF("Loading Exectuable Demo\n");
	/* Initialize libmetal evironment */
	metal_init(&metal_param);
	/* Initialize remoteproc instance */
	ret_rproc = remoteproc_init(&rproc, &r5_rproc_ops, &cpu_id);
	if (!ret_rproc) {
		LPRINTF("failed to initialize coprocessor\r\n");
		return -1;
	}

	ret = load_exectuable_block(&rproc, &mem_image_store_ops, store, NULL);
	if (ret < 0) {
		LPERROR("load_exectuable_block failed\r\n");
		/* Make sure the remote is shut down */
		remoteproc_shutdown(&rproc);
		return -1;
	}

	ret = load_exectuable_noblock(&rproc, &mem_image_store_ops, store,
				      NULL);
	if (ret < 0) {
		LPERROR("load_exectuable_noblock failed\r\n");
		/* Make sure the remote is shut down */
		remoteproc_shutdown(&rproc);
		return -1;
	}
	remoteproc_remove(&rproc);
	return ret;
}
