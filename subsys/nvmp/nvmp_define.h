/*
 * Copyright (c) 2023 Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef NVMP_DEFINE_H_
#define NVMP_DEFINE_H_

#define NVMP_INFO_DEFINE(inst, _type, _size, _ro, _store, _store_off, _read,	\
			 _write, _erase)		                        \
	const struct nvmp_info nvmp_info_##inst = {				\
		.type = _type,							\
		.size = _size,							\
		.read_only = _ro,						\
		.store = _store,						\
		.store_off = _store_off,					\
		.read = _read,							\
		.write = _write,						\
		.erase = _erase,						\
	}

#define NVMP_RO(inst)								\
	COND_CODE_1(DT_NODE_HAS_PROP(inst, read_only),				\
		    (DT_PROP(inst, read_only)), (false))

#define NVMP_PARTITION_SIZE(inst) DT_REG_SIZE(inst)
#define NVMP_PARTITION_PARENT(inst) UTIL_CAT(nvmp_info_, DT_GPARENT(inst))
#define NVMP_PARTITION_STORE(inst) UTIL_CAT(nvmp_info_store_, DT_GPARENT(inst))
#define NVMP_PARTITION_STORE_OFF(inst)						\
	UTIL_CAT(nvmp_info_store_offset_, DT_GPARENT(inst))
#define NVMP_PARTITION_OFF(inst)						\
	NVMP_PARTITION_STORE_OFF(inst) + DT_REG_ADDR(inst)
#define NVMP_PARTITION_RO(inst) NVMP_RO(inst) || NVMP_RO(DT_GPARENT(inst))

#define NVMP_PARTITION_DEFINE(inst, _type, _read, _write, _erase)		\
	NVMP_INFO_DEFINE(inst, _type, NVMP_PARTITION_SIZE(inst),		\
			 NVMP_PARTITION_RO(inst),				\
			 (void *)NVMP_PARTITION_STORE(inst),			\
			 NVMP_PARTITION_OFF(inst), _read, _write, _erase);

#define NVMP_PARTITIONS_DEFINE(n, _compat, _type, _read, _write, _erase)	\
	COND_CODE_0(DT_NODE_HAS_COMPAT(DT_PARENT(n), _compat), (),		\
		    (DT_FOREACH_CHILD_VARGS(n, NVMP_PARTITION_DEFINE, _type,	\
					    _read, _write, _erase)))

#define NVMP_COMPATIBLE_PARTITIONS_DEFINE(_compatible, _type, _read, _write,	\
					  _erase)				\
	DT_FOREACH_STATUS_OKAY_VARGS(zephyr_nvmp_partitions,			\
				     NVMP_PARTITIONS_DEFINE, _compatible, _type,\
				     _read, _write, _erase)

#endif /* NVMP_DEFINE_H_ */
