/*
 * Copyright (c) 2023 Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/**
 * @file
 * @brief Public API for non volatile memory and partitions
 */

#ifndef ZEPHYR_INCLUDE_NVMP_H_
#define ZEPHYR_INCLUDE_NVMP_H_

/**
 * @brief Abstraction over non volatile memory and partitions on non volatile
 *        memory and their drivers
 *
 * @defgroup nvmp_part_api nvmp interface
 * @{
 */

/*
 * This API makes it possible to operate on non volatile memory and partitions
 * on non volatile memory easily and effectively.
 */

#include <errno.h>
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
 * The nvmp subsystem provide an abstraction layer between non volatile memory
 * specific drivers and higher-level applications for non volatile memory. On
 * non volatile memory 3 routines for operation are used: read, write and erase.
 *
 * Both the non volatile memory and partitions on non volatile memory are
 * created by the subsystem.
 *
 */

/**
 * @brief Retrieve a pointer to the non volatile info struct of non volatile
 *        memory.
 *
 * A non volatile memory with nodelabel "flash0" is retrieved as:
 * const struct nvmp_info *nvmp = NVMP_GET(flash0)
 * A non volatile memory partition nodelabel "storage_partition" is retrieved as:
 * const struct nbmp_info *nvmp = NVMP_GET(storage_partition)
 *
 * @param nodelabel of the non volatile memory.
 */
#define NVMP_GET(nodelabel) &UTIL_CAT(nvmp_info_, DT_NODELABEL(nodelabel))

enum nvmp_type {UNKNOWN, FLASH, EEPROM, RETAINED_MEM};

struct nvmp_info;

/**
 * @brief nvmp_info represents a nvmp item.
 */
struct nvmp_info {
	/** nvmp type */
	const enum nvmp_type type;
	/** nvmp size */
	const size_t size;
	/** nvmp read-only */
	const bool read_only;
	/** nvmp storage (is NULL for a nvmp partition) */
	const void *store;
	/** nvmp offset on store */
	const off_t store_off;
	/** nvmp routines */
	int (* const read)(const struct nvmp_info *info, off_t off, void *data,
			   size_t len);
	int (* const write)(const struct nvmp_info *info, off_t off,
			    const void *data, size_t len);
	int (* const erase)(const struct nvmp_info *info, off_t off,
			    size_t len);
};

/**
 * @brief Get the size of a nvmp item.
 *
 * @param[in] info nvmp item
 * @return the size.
 */
size_t nvmp_get_size(const struct nvmp_info *info);

/**
 * @brief Get the type of a nvmp item.
 *
 * @param[in] info nvmp item
 * @return the non volatile memory type.
 */
enum nvmp_type nvmp_get_type(const struct nvmp_info *info);

/**
 * @brief Get the storage of a nvmp item.
 *
 * @param[in] info nvmp item
 * @return the storage.
 */
const void *nvmp_get_store(const struct nvmp_info *info);

/**
 * @brief Get the offset on storage of a nvmp item.
 *
 * @param[in] info nvmp item
 * @return the offset.
 */
off_t nvmp_get_store_offset(const struct nvmp_info *info);

/**
 * @brief Read data from nvmp item. Read boundaries are verified before read
 * request is executed.
 *
 * @param[in] info nvmp item
 * @param[in] off Offset relative from beginning of nvmp item to read
 * @param[out] data Buffer to store read data
 * @param[in] len Number of bytes to read
 *
 * @return  0 on success, negative errno code on fail.
 */
int nvmp_read(const struct nvmp_info *info, off_t off, void *data, size_t len);

/**
 * @brief Write data to nvmp item. Write boundaries are verified before write
 * request is executed.
 *
 * @param[in] info nvmp item
 * @param[in] off Offset relative from beginning of nvmp item to write
 * @param[out] data Buffer with data to be written
 * @param[in] len Number of bytes to write
 *
 * @return  0 on success, negative errno code on fail.
 */
int nvmp_write(const struct nvmp_info *info, off_t off, const void *data,
	       size_t len);

/**
 * @brief Erase range on nvmp item. Erase boundaries are verified before erase
 * is executed.
 *
 * @param[in] info nvmp item
 * @param[in] off Offset relative from beginning of nvmp item.
 * @param[in] len Number of bytes to be erase
 *
 * @return  0 on success, negative errno code on fail.
 */
int nvmp_erase(const struct nvmp_info *info, off_t off, size_t len);

/**
 * @brief Helper macro to find out the compatibility of a node, this can be
 * useful when aiming for minimal code size when multiple types of non
 * volatile memory are enabled.
 *
 * @param node of the non volatile memory.
 * @param compatible of the non volatile memory
 */
#define NVMP_HAS_COMPATIBLE(node, compatible) COND_CODE_1(			\
	DT_NODE_HAS_COMPAT(node, compatible), (true),				\
	(COND_CODE_1(DT_NODE_HAS_COMPAT(DT_PARENT(node),			\
					zephyr_nvmp_partitions),		\
		(DT_NODE_HAS_COMPAT(DT_GPARENT(node), compatible)), (false))))

/** @cond INTERNAL_HIDDEN */

#define NVMP_DECLARE(n) extern const struct nvmp_info nvmp_info_##n;

#define NVMP_PARTITIONS_DECLARE(n, _compat)					\
	COND_CODE_0(DT_NODE_HAS_COMPAT(DT_PARENT(n), _compat), (),		\
		(DT_FOREACH_CHILD(n, NVMP_DECLARE)))

#define NVMP_COMPATIBLE_PARTITIONS_DECLARE(_compatible)				\
	DT_FOREACH_STATUS_OKAY_VARGS(zephyr_nvmp_partitions,			\
		NVMP_PARTITIONS_DECLARE, _compatible)

#ifdef CONFIG_NVMP_FLASH
DT_FOREACH_STATUS_OKAY(zephyr_nvmp_flash, NVMP_DECLARE)
NVMP_COMPATIBLE_PARTITIONS_DECLARE(zephyr_nvmp_flash)
#endif

#ifdef CONFIG_NVMP_EEPROM
DT_FOREACH_STATUS_OKAY(zephyr_nvmp_eeprom, NVMP_DECLARE)
NVMP_COMPATIBLE_PARTITIONS_DECLARE(zephyr_nvmp_eeprom)
#endif

#ifdef CONFIG_NVMP_RETAINED_MEM
DT_FOREACH_STATUS_OKAY(zephyr_nvmp_retained_mem, NVMP_DECLARE)
NVMP_COMPATIBLE_PARTITIONS_DECLARE(zephyr_nvmp_retained_mem)
#endif

/** @endcond */

#ifdef __cplusplus
}
#endif

/**
 * @}
 */

#endif /* ZEPHYR_INCLUDE_NVMP_H_ */
