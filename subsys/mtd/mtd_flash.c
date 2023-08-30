/*
 * Copyright (c) 2023, Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/mtd/mtd.h>
#include <errno.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(mtd_flash, CONFIG_MTD_LOG_LEVEL);

static int mtd_flash_read(const struct mtd_dev_cfg *dev_cfg, off_t off,
			  void *dst, size_t len)
{
	if (dev_cfg == NULL) {
		return -EINVAL;
	}

	if (!device_is_ready(dev_cfg->dev)) {
		return -ENODEV;
	}

	LOG_DBG("read %d byte at 0x%lx", len, off);
	return flash_read(dev_cfg->dev, off, dst, len);
}

static int mtd_flash_write(const struct mtd_dev_cfg *dev_cfg, off_t off,
			   const void *src, size_t len)
{
	if (dev_cfg == NULL) {
		return -EINVAL;
	}

	if (!device_is_ready(dev_cfg->dev)) {
		return -ENODEV;
	}

	LOG_DBG("write %d byte at 0x%lx", len, off);
	return flash_write(dev_cfg->dev, off, src, len);
}

static int mtd_flash_erase(const struct mtd_dev_cfg *dev_cfg, off_t off,
			   size_t len)
{
	if (dev_cfg == NULL) {
		return -EINVAL;
	}

	if (!device_is_ready(dev_cfg->dev)) {
		return -ENODEV;
	}

	LOG_DBG("erase %d byte at 0x%lx", len, off);
	return flash_erase(dev_cfg->dev, off, len);
}

#include "mtd_define.h"

#define MTD_FLASH_SIZE(inst)							\
	COND_CODE_1(DT_NODE_HAS_COMPAT(inst, soc_nv_flash), (DT_REG_SIZE(inst)),\
		    (DT_PROP_OR(inst, size, 0) / 8))
#define MTD_FLASH_DEVICE(inst)							\
	COND_CODE_1(DT_NODE_HAS_COMPAT(inst, soc_nv_flash),			\
		    (DEVICE_DT_GET(DT_PARENT(inst))), (DEVICE_DT_GET(inst)))

#define MTD_FLASH_DEFINE(inst)							\
	MTD_DEV_CFG_DEFINE(inst, MTD_FLASH_DEVICE(inst), FLASH, mtd_flash_read,	\
			   mtd_flash_write, mtd_flash_erase);			\
	MTD_INFO_DEFINE(inst, &mtd_dev_cfg_##inst, NULL, 0,			\
			MTD_FLASH_SIZE(inst), MTD_RO(inst));

DT_FOREACH_STATUS_OKAY(zephyr_mtd_flash, MTD_FLASH_DEFINE)

MTD_COMPATIBLE_PARTITIONS_DEFINE(zephyr_mtd_flash)
