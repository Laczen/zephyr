/*
 * Copyright (c) 2023, Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <errno.h>
#include <zephyr/mtd/mtd.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mtd_retained_mem, CONFIG_MTD_LOG_LEVEL);

static int mtd_retained_mem_read(const struct mtd_dev_cfg *dev_cfg, off_t off,
				 void *dst, size_t len)
{
	if (dev_cfg == NULL) {
		return -EINVAL;
	}

	if (!device_is_ready(dev_cfg->dev)) {
		return -ENODEV;
	}

	LOG_DBG("read %d byte at 0x%lx", len, off);
	return retained_mem_read(dev_cfg->dev, off, dst, len);
}

static int mtd_retained_mem_write(const struct mtd_dev_cfg *dev_cfg, off_t off,
				  const void *src, size_t len)
{
	if (dev_cfg == NULL) {
		return -EINVAL;
	}

	if (!device_is_ready(dev_cfg->dev)) {
		return -ENODEV;
	}

	LOG_DBG("write %d byte at 0x%lx", len, off);
	return retained_mem_write(dev_cfg->dev, off, src, len);
}

static int mtd_retained_mem_erase(const struct mtd_dev_cfg *dev_cfg, off_t off,
				  size_t len)
{
	int rc;
	uint8_t buf[CONFIG_MTD_ERASE_BUFSIZE];

	if (dev_cfg == NULL) {
		return -EINVAL;
	}

	LOG_DBG("erase %d byte at 0x%lx", len, off);
	memset(buf, (char)CONFIG_MTD_ERASE_VALUE, sizeof(buf));
	while (len != 0) {
		size_t wrlen = MIN(len, sizeof(buf));

		rc = dev_cfg->write(dev_cfg, off, buf, wrlen);
		if (rc) {
			break;
		}

		len -= wrlen;
		off += wrlen;
	}

	return rc;
}

#include "mtd_define.h"

#define MTD_RETAINED_MEM_SIZE(inst)						\
	COND_CODE_1(DT_NODE_HAS_COMPAT(inst, zephyr_retained_ram),		\
		(DT_REG_SIZE(DT_PARENT(inst))),					\
		(COND_CODE_1(DT_NODE_HAS_COMPAT(inst, nordic_nrf_gpregret),	\
			(DT_REG_SIZE(inst)), (0))))
#define MTD_RETAINED_MEM_DEVICE(inst) DEVICE_DT_GET(inst)

#define MTD_RETAINED_MEM_DEFINE(inst)						\
	MTD_DEV_CFG_DEFINE(inst, MTD_RETAINED_MEM_DEVICE(inst), RETAINED_MEM,	\
			   mtd_retained_mem_read, mtd_retained_mem_write,	\
			   mtd_retained_mem_erase);				\
	MTD_INFO_DEFINE(inst, &mtd_dev_cfg_##inst, NULL, 0,			\
			MTD_RETAINED_MEM_SIZE(inst), MTD_RO(inst));

DT_FOREACH_STATUS_OKAY(zephyr_mtd_retained_mem, MTD_RETAINED_MEM_DEFINE)

MTD_COMPATIBLE_PARTITIONS_DEFINE(zephyr_mtd_retained_mem)
