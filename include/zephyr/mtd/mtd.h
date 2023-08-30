/*
 * Copyright (c) 2023 Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Public API for memory technology devices (mtd) partitions
 */

#ifndef ZEPHYR_INCLUDE_MTD_H_
#define ZEPHYR_INCLUDE_MTD_H_

/**
 * @brief Abstraction over mtd devices and partitions on mtd devices and their
 *        drivers
 *
 * @defgroup mtd_part_api mtd interface
 * @{
 */

/*
 * This API makes it possible to operate on mtd devices and partitions on mtd
 * devices easily and effectively.
 */

#include <sys/types.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/flash.h>
#include <zephyr/drivers/eeprom.h>
#include <zephyr/drivers/retained_mem.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The MTD subsystem provide an abstraction layer between hardware-specific
 * device drivers and higher-level applications for mtd devices. On mtd devices
 * 3 routines for operation are used: read, write and erase. Before data can be
 * written the write location needs to be erased. The MTD subsystem also
 * provides an adaptation layer for devices that are generally not considered
 * mtd devices (e.g. eeprom and retained_mem).
 *
 * Both the MTD device as partitions on a MTD device are created by the
 * subsystem.
 *
 */

/**
 * @brief Retrieve a pointer to the MTD info struct of a MTD.
 *
 * A mtd with nodelabel "flash0" is retrieved as:
 * const struct mtd_info *mtd = MTD_GET(flash0)
 * A mtd partition nodelabel "storage_partition" is retrieved as:
 * const struct mtd_info *mtd = MTD_GET(storage_partition)
 *
 * @param nodelabel of the mtd device.
 */
#define MTD_GET(nodelabel) &UTIL_CAT(mtd_info_, DT_NODELABEL(nodelabel))

enum mtd_type {UNKNOWN, FLASH, EEPROM, RETAINED_MEM};

struct mtd_dev_cfg;

/** @brief mtd_dev_cfg represents the configuration of a mtd device */
struct mtd_dev_cfg {
	/** device used as mtd backend */
	const struct device *dev;
	/** type of device used as mtd backend */
	const enum mtd_type type;
<<<<<<< HEAD
	/** backend routines */
=======
>>>>>>> bb2d1df96c (mtd: api for mtd devices)
	int (* const read)(const struct mtd_dev_cfg *dev_cfg, off_t off,
			   void *dst, size_t len);
	int (* const write)(const struct mtd_dev_cfg *dev_cfg, off_t off,
			    const void *src, size_t len);
	int (* const erase)(const struct mtd_dev_cfg *dev_cfg, off_t off,
			    size_t len);
};

struct mtd_info;

/**
 * @brief mtd_info represents a mtd item.
 */
struct mtd_info {
	/** mtd device configuration (is NULL for a mtd partition) */
	const struct mtd_dev_cfg *dev_cfg;
	/** mtd parent (is NULL for a mtd device) */
	const struct mtd_info *parent;
	/** mtd offset on parent */
	const off_t off;
	const size_t size;
	const bool read_only;
};

/**
 * @brief Get the size of a mtd item.
 *
 * @param[in] info MTD item
 * @return the size.
 */
size_t mtd_get_size(const struct mtd_info *info);

/**
 * @brief Get the device that is used by a mtd item.
 *
 * @param[in] info MTD item
 * @return the device.
 */
const struct device *mtd_get_device(const struct mtd_info *info);

/**
 * @brief Get the offset of the mtd item on the device.
 *
 * @param[in] info MTD item
 * @return the offsetdevice.
 */
off_t mtd_get_device_offset(const struct mtd_info *info);

/**
 * @brief Get the device type of a mtd item.
 *
 * @param[in] info MTD item
 * @return the device type.
 */
enum mtd_type mtd_get_device_type(const struct mtd_info *info);

/**
 * @brief Read data from mtd item. Read boundaries are verified before read
 * request is executed.
 *
 * @param[in] info MTD item
 * @param[in] off Offset relative from beginning of mtd item to read
 * @param[out] dst Buffer to store read data
 * @param[in] len Number of bytes to read
 *
 * @return  0 on success, negative errno code on fail.
 */
int mtd_read(const struct mtd_info *info, off_t off, void *dst, size_t len);

/**
 * @brief Write data to mtd item. Write boundaries are verified before write
 * request is executed.
 *
 * @param[in] info MTD item
 * @param[in] off Offset relative from beginning of mtd item to write
 * @param[out] src Buffer with data to be written
 * @param[in] len Number of bytes to write
 *
 * @return  0 on success, negative errno code on fail.
 */
int mtd_write(const struct mtd_info *info, off_t off, const void *src,
	      size_t len);

/**
 * @brief Erase range on mtd item. Erase boundaries are verified before erase
 * is executed.
 *
 * @param[in] info MTD item
 * @param[in] off Offset relative from beginning of mtd item.
 * @param[in] len Number of bytes to be erase
 *
 * @return  0 on success, negative errno code on fail.
 */
int mtd_erase(const struct mtd_info *info, off_t off, size_t len);

/** @cond INTERNAL_HIDDEN */

#define MTD_DECLARE(n) extern const struct mtd_info mtd_info_##n;

#define MTD_FIXED_PARTITIONS_DECLARE(n, _compat)				\
	COND_CODE_0(DT_NODE_HAS_COMPAT(DT_PARENT(n), _compat), (),		\
		(DT_FOREACH_CHILD(n, MTD_DECLARE)))

#define MTD_COMPATIBLE_PARTITIONS_DECLARE(_compatible)				\
	DT_FOREACH_STATUS_OKAY_VARGS(fixed_partitions,                          \
		MTD_FIXED_PARTITIONS_DECLARE, _compatible)

#ifdef CONFIG_MTD_FLASH
DT_FOREACH_STATUS_OKAY(zephyr_mtd_flash, MTD_DECLARE)
MTD_COMPATIBLE_PARTITIONS_DECLARE(zephyr_mtd_flash)
#endif

#ifdef CONFIG_MTD_EEPROM
DT_FOREACH_STATUS_OKAY(zephyr_mtd_eeprom, MTD_DECLARE)
MTD_COMPATIBLE_PARTITIONS_DECLARE(zephyr_mtd_eeprom)
#endif

#ifdef CONFIG_MTD_RETAINED_MEM
DT_FOREACH_STATUS_OKAY(zephyr_mtd_retained_mem, MTD_DECLARE)
MTD_COMPATIBLE_PARTITIONS_DECLARE(zephyr_mtd_retained_mem)
#endif

/** @endcond */

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_MTD_H_ */
