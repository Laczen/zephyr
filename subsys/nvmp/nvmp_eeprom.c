/*
 * Copyright (c) 2023, Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/logging/log.h>
#include <zephyr/nvmp/nvmp.h>

LOG_MODULE_REGISTER(nvmp_eeprom, CONFIG_NVMP_LOG_LEVEL);

static int nvmp_eeprom_read(const struct nvmp_info *info, off_t off,
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
	return eeprom_read(dev, off, data, len);
}

static int nvmp_eeprom_write(const struct nvmp_info *info, off_t off,
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
	return eeprom_write(dev, off, data, len);
}

#ifdef CONFIG_NVMP_EEPROM_ERASE
static int nvmp_eeprom_erase(const struct nvmp_info *info, off_t off,
			     size_t len)
{
	if ((info == NULL) || (info->size < len) ||
	    ((info->size - len) < (size_t)off)) {
		return -EINVAL;
	}

	if (info->read_only) {
		return -EACCES;
	}

	int rc;
	uint8_t buf[CONFIG_NVMP_ERASE_BUFSIZE];

	memset(buf, (char)CONFIG_NVMP_ERASE_VALUE, sizeof(buf));
	while (len != 0) {
		size_t wrlen = MIN(len, sizeof(buf));

		rc = info->write(info, off, buf, wrlen);
		if (rc) {
			break;
		}

		len -= wrlen;
		off += wrlen;
	}

	return rc;
}
#else /* CONFIG_NVMP_EEPROM_ERASE */
static int nvmp_eeprom_erase(const struct nvmp_info *info, off_t off,
			     size_t len)
{
	return -ENOTSUP;
}
#endif /* CONFIG_NVMP_EEPROM_ERASE */

#include "nvmp_define.h"

#define NVMP_EEPROM_SIZE(inst) DT_PROP(inst, size)
#define NVMP_EEPROM_DEVICE(inst) DEVICE_DT_GET(inst)

#define NVMP_EEPROM_DEFINE(inst)						\
	const struct device * const nvmp_info_store_##inst =			\
		NVMP_EEPROM_DEVICE(inst);					\
	const off_t nvmp_info_store_offset_##inst = 0;				\
	NVMP_INFO_DEFINE(inst, EEPROM, NVMP_EEPROM_SIZE(inst), NVMP_RO(inst),	\
			 (void *)nvmp_info_store_##inst,			\
			 nvmp_info_store_offset_##inst, nvmp_eeprom_read,	\
			 nvmp_eeprom_write, nvmp_eeprom_erase);

DT_FOREACH_STATUS_OKAY(zephyr_nvmp_eeprom, NVMP_EEPROM_DEFINE)

NVMP_COMPATIBLE_PARTITIONS_DEFINE(zephyr_nvmp_eeprom, EEPROM, nvmp_eeprom_read,
				  nvmp_eeprom_write, nvmp_eeprom_erase)
