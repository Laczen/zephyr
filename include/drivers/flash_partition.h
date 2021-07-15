/*
 * Copyright (c) 2021 Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Public API for partitions on flash devices
 */

#ifndef ZEPHYR_INCLUDE_FLASH_PARTITION_H_
#define ZEPHYR_INCLUDE_FLASH_PARTITION_H_

/**
 * @brief Abstraction over flash partitions
 *
 * @defgroup flash_partition_api flash partition interface
 * @{
 */

/*
 * This API makes it possible to operate on flash partitions easily and
 * effectively.
 */

/**
 *
 */
#include <zephyr/types.h>
#include <stddef.h>
#include <sys/types.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/flash.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The flash partition subsystem provide an abstraction layer between
 * hardware-specific device drivers and higher-level applications for flash
 * partitions. Flash partitions provides 3 routines for operation: read, write
 * and erase. Flash partition erases typically operate on blocks and the erase
 * can only be peformed on a complete block (of erase-block-size).
 */

/**
 * @brief Retrieve a pointer to a flash partition.
 *
 * A partition labeled "image-1" is retrieved as:
 * const struct flash_partition *flp = FLASH_PARTITION_GET(image_1)
 *
 * @param label Label of the partition.
 */
#define FLASH_PARTITION_GET(label) &UTIL_CAT(flp_, \
		DT_NODELABEL(DT_NODE_BY_FIXED_PARTITION_LABEL(label)))

struct flp_info;

/**
 * @brief flp_info_cfg represents the configuration of a partition. A partition
 * can be the child of a partition (then the parent pointer is defined and the
 * device pointer is NULL) or a child of a device (then the parent pointer is
 * NULL and the device pointer is defined).
 */
struct flp_info_cfg {
	const struct device *device;
	const struct flp_info *parent;
	const off_t off;
	const size_t size;
	const bool read_only;
};

/**
 * @brief flp_info_state represent the state of a partition.
 */
struct flp_info_state {};

/**
 * @brief flp_info represents a partition.
 */
struct flp_info {
	const struct flp_info_cfg *cfg;
	const struct flp_info_state *state;
};

/**
 * @brief Read data from flash partition. Read boundaries are verified before
 * read request is executed.
 *
 * @param[in]  flp Flash partition
 * @param[in]  off Read offset from beginning of flash partition
 * @param[out] dst Buffer to store read data
 * @param[in]  len Number of bytes to read
 *
 * @return  0 on success, negative errno code on fail.
 */
int flp_read(const struct flp_info *flp, off_t off, void *dst, size_t len);

/**
 * @brief Write data to flash partition. Write boundaries are verified before
 * write request is executed.
 *
 * @param[in]  flp Flash partition
 * @param[in]  off Write offset from beginning of flash partition
 * @param[out] src Buffer with data to be written
 * @param[in]  len Number of bytes to write
 *
 * @return  0 on success, negative errno code on fail.
 */
int flp_write(const struct flp_info *flp, off_t off, const void *src,
	      size_t len);

/**
 * @brief Erase range on flash partition. Erase boundaries are verified before
 * erase is executed.
 *
 * @param[in] flp Flash partition
 * @param[in] off Erase offset from beginning of flash partition
 * @param[in] len Number of bytes to be erase (should be a multiple of
 *                erase-block-size)
 *
 * @return  0 on success, negative errno code on fail.
 */
int flp_erase(const struct flp_info *flp, off_t off, size_t len);

/**
 * @brief Get flash parameters (write-block-size and erase-value) of the device
 * the partition is on.
 *
 * @param[in] flp Flash partition
 *
 * @return 0 on success, negative errno code on fail.
 */
const struct flash_parameters *flp_get_parameters(const struct flp_info *flp);

/**
 * @brief Iterate over all pages on a partition
 *
 * This routine iterates over all pages on the given partition, ordered by
 * increasing offset. For each page, it invokes the given callback, passing it
 * the page information and a private data object. The iteration stops when
 * the callback returns false.
 *
 * @param flp Flash partition whose blocks to iterate over
 * @param cb Callback to invoke for each page
 * @param data Private data for callback function
 */
void flp_page_foreach(const struct flp_info *flp, flash_page_cb cb, void *data);

/**
 * @brief Get flash partition page info by offset.
 *
 * @param[in] flp Flash partition
 * @param[in] offset Offset in flash partition
 * @param[out] info The flash partition page info (start_offset, size and index)
 *
 * @return 0 on success, negative errno code on fail.
 */
int flp_get_page_info_by_offs(const struct flp_info *flp, off_t offset,
			      struct flash_pages_info *info);

/**
 * @brief Get flash partition page info by index.
 *
 * @param[in] flp Flash partition
 * @param[in] idx Index of page in flash partition
 * @param[out] info The flash partition page info (start_offset, size and index)
 *
 * @return 0 on success, negative errno code on fail.
 */
int flp_get_page_info_by_idx(const struct flp_info *flp, uint32_t idx,
			     struct flash_pages_info *info);

/**
 *  @brief Get flash partition page count.
 *
 *  @param[in] flp Flash partition
 *
 *  @return  Number of pages.
 */
size_t flp_get_page_count(const struct flp_info *flp);

/** @cond INTERNAL_HIDDEN */
#define DT_DRV_COMPAT fixed_partitions

#define FLASH_PARTITION_DECLARE(n) \
	extern const struct flp_info UTIL_CAT(flp_, DT_NODELABEL(n));

#define FOREACH_FLASH_PARTITION_DECLARE(n) \
	DT_FOREACH_CHILD(DT_DRV_INST(n), FLASH_PARTITION_DECLARE)

DT_INST_FOREACH_STATUS_OKAY(FOREACH_FLASH_PARTITION_DECLARE)

/** @endcond */

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_FLASH_PARTITION_H_ */
