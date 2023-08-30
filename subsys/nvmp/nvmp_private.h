/*
 * Copyright (c) 2023 Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NVMP_PRIVATE_H_
#define NVMP_PRIVATE_H_

#include <zephyr/nvmp/nvmp.h>

static bool nvmp_bounds_error(const struct nvmp_info *info, off_t off,
			      size_t len)
{
	if ((info->size < len) || ((info->size - len) < (size_t)off)) {
		return true;
	}

	return false;
}

static int nvmp_item_read(const struct nvmp_info *info, off_t off, void *data,
		  size_t len)
{
	if ((info == NULL) || (nvmp_bounds_error(info, off, len))) {
		return -EINVAL;
	}

	off_t dev_off = 0;
	const struct nvmp_backend *be = nvmp_get_be(info, &dev_off);

	return be->read(be, dev_off, data, len);
}

static int nvmp_item_write(const struct nvmp_info *info, off_t off,
			   const void *data, size_t len)
{
	if ((info == NULL) || (nvmp_bounds_error(info, off, len))) {
		return -EINVAL;
	}

	if (info->read_only) {
		return -EACCES;
	}

	off_t dev_off = 0;
	const struct nvmp_backend *be = nvmp_get_be(info, &dev_off);

	return be->write(be, dev_off, data, len);
}

static int nvmp_item_erase(const struct nvmp_info *info, off_t off, size_t len)
{
	if ((info == NULL) || (nvmp_bounds_error(info, off, len))) {
		return -EINVAL;
	}

	if (info->read_only) {
		return -EACCES;
	}

	off_t dev_off = 0;
	const struct nvmp_backend *be = nvmp_get_be(info, &dev_off);

	return be->erase(be, dev_off, len);
}

#define NVMP_BACKEND_DEFINE(inst, _ss, _type, _read, _write, _erase)            \
	static const struct nvmp_backend nvmp_backend_##inst = {		\
		.ss = _ss,							\
		.type = _type,							\
		.read = _read,							\
		.write = _write,						\
		.erase = _erase,						\
	}

#define NVMP_INFO_DEFINE(inst, _backend, _parent, _off, _size, _ro, _read,	\
			 _write, _erase)		                        \
	const struct nvmp_info nvmp_info_##inst = {				\
		.backend = _backend,						\
		.parent = _parent,						\
		.off = _off,							\
		.size = _size,							\
		.read_only = _ro,						\
		.read = _read,							\
		.write = _write,						\
		.erase = _erase,						\
	}

#define NVMP_RO(inst)								\
	COND_CODE_1(DT_NODE_HAS_PROP(inst, read_only),				\
		    (DT_PROP(inst, read_only)), (false))

#define NVMP_PARTITION_SIZE(inst) DT_REG_SIZE(inst)
#define NVMP_PARTITION_OFF(inst) DT_REG_ADDR(inst)
#define NVMP_PARTITION_PARENT(inst) UTIL_CAT(nvmp_info_, DT_GPARENT(inst))

#define NVMP_PARTITION_DEFINE(inst, _read, _write, _erase)			\
	NVMP_INFO_DEFINE(inst, NULL, &NVMP_PARTITION_PARENT(inst),		\
			 NVMP_PARTITION_OFF(inst), NVMP_PARTITION_SIZE(inst),	\
			 NVMP_RO(inst), _read, _write, _erase);

#define NVMP_PARTITIONS_DEFINE(n, _compat, _read, _write, _erase)		\
	COND_CODE_0(DT_NODE_HAS_COMPAT(DT_PARENT(n), _compat), (),		\
		(DT_FOREACH_CHILD_VARGS(n, NVMP_PARTITION_DEFINE, _read, _write,\
			_erase)))

#define NVMP_COMPATIBLE_PARTITIONS_DEFINE(_compatible, _read, _write, _erase)   \
	DT_FOREACH_STATUS_OKAY_VARGS(zephyr_nvmp_partitions,			\
		NVMP_PARTITIONS_DEFINE, _compatible, _read, _write, _erase)

#endif /* NVMP_PRIVATE_H_ */
