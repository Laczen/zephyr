/*
 * Copyright (c) 2023, Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/logging/log.h>
#include <zephyr/nvmp/nvmp.h>

LOG_MODULE_REGISTER(nvmp_flash, CONFIG_NVMP_LOG_LEVEL);

static int nvmp_flash_read(const struct nvmp_info *info, off_t off,
			   void *data, size_t len)
{
	if ((info == NULL) || (info->size < len) ||
	    ((info->size - len) < (size_t)off)) {
		return -EINVAL;
	}

	const struct device *dev = (const struct device *)info->store;

	if (!device_is_ready(dev)) {
		return -ENODEV;
	}

	off += info->store_off;
	LOG_DBG("read %d byte at 0x%lx", len, off);
	return flash_read(dev, off, data, len);
}

static int nvmp_flash_write(const struct nvmp_info *info, off_t off,
			    const void *data, size_t len)
{
	if ((info == NULL) || (info->size < len) ||
	    ((info->size - len) < (size_t)off)) {
		return -EINVAL;
	}

	if (info->read_only) {
		return -EACCES;
	}

	const struct device *dev = (const struct device *)info->store;

	if (!device_is_ready(dev)) {
		return -ENODEV;
	}

	off += info->store_off;
	LOG_DBG("write %d byte at 0x%lx", len, off);
	return flash_write(dev, off, data, len);
}

static int nvmp_flash_erase(const struct nvmp_info *info, off_t off, size_t len)
{
	if ((info == NULL) || (info->size < len) ||
	    ((info->size - len) < (size_t)off)) {
		return -EINVAL;
	}

	if (info->read_only) {
		return -EACCES;
	}

	const struct device *dev = (const struct device *)info->store;

	if (!device_is_ready(dev)) {
		return -ENODEV;
	}

	off += info->store_off;
	LOG_DBG("erase %d byte at 0x%lx", len, off);
	return flash_erase(dev, off, len);
}

#include "nvmp_define.h"

#define NVMP_FLASH_SIZE(inst)							\
	COND_CODE_1(DT_NODE_HAS_COMPAT(inst, soc_nv_flash), (DT_REG_SIZE(inst)),\
		    (DT_PROP_OR(inst, size, 0) / 8))
#define NVMP_FLASH_DEVICE(inst)							\
	COND_CODE_1(DT_NODE_HAS_COMPAT(inst, soc_nv_flash),			\
		    (DEVICE_DT_GET(DT_PARENT(inst))), (DEVICE_DT_GET(inst)))

#define NVMP_FLASH_DEFINE(inst)							\
	const struct device * const nvmp_info_store_##inst =			\
		NVMP_FLASH_DEVICE(inst);					\
	const off_t nvmp_info_store_offset_##inst = 0;				\
	NVMP_INFO_DEFINE(inst, FLASH, NVMP_FLASH_SIZE(inst), NVMP_RO(inst),	\
			 (void *)nvmp_info_store_##inst,			\
			 nvmp_info_store_offset_##inst, nvmp_flash_read,	\
			 nvmp_flash_write, nvmp_flash_erase);

DT_FOREACH_STATUS_OKAY(zephyr_nvmp_flash, NVMP_FLASH_DEFINE)

NVMP_COMPATIBLE_PARTITIONS_DEFINE(zephyr_nvmp_flash, FLASH, nvmp_flash_read,
				  nvmp_flash_write, nvmp_flash_erase)
