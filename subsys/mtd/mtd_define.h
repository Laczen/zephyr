/*
 * Copyright (c) 2023 Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef MTD_DEFINE_H_
#define MTD_DEFINE_H_

#define MTD_DEV_CFG_DEFINE(inst, _dev, _type, _read, _write, _erase)		\
	static const struct mtd_dev_cfg mtd_dev_cfg_##inst = {			\
		.dev = _dev,							\
		.type = _type,							\
		.read = _read,							\
		.write = _write,						\
		.erase = _erase,						\
	}

#define MTD_INFO_DEFINE(inst, _dev_cfg, _parent, _off, _size, _ro)		\
	const struct mtd_info mtd_info_##inst = {				\
		.dev_cfg = _dev_cfg,						\
		.parent = _parent,						\
		.off = _off,							\
		.size = _size,							\
		.read_only = _ro,						\
	}

#define MTD_RO(inst)								\
	COND_CODE_1(DT_NODE_HAS_PROP(inst, read_only),				\
		    (DT_PROP(inst, read_only)), (false))

#define MTD_PARTITION_SIZE(inst) DT_REG_SIZE(inst)
#define MTD_PARTITION_OFF(inst) DT_REG_ADDR(inst)
#define MTD_PARTITION_PARENT(inst) UTIL_CAT(mtd_info_, DT_GPARENT(inst))

#define MTD_PARTITION_DEFINE(inst)						\
	MTD_INFO_DEFINE(inst, NULL, &MTD_PARTITION_PARENT(inst),		\
			MTD_PARTITION_OFF(inst), MTD_PARTITION_SIZE(inst),	\
			MTD_RO(inst));

#define MTD_FIXED_PARTITIONS_DEFINE(n, _compat)					\
	COND_CODE_0(DT_NODE_HAS_COMPAT(DT_PARENT(n), _compat), (),		\
		(DT_FOREACH_CHILD(n, MTD_PARTITION_DEFINE)))

#define MTD_COMPATIBLE_PARTITIONS_DEFINE(_compatible)                           \
	DT_FOREACH_STATUS_OKAY_VARGS(fixed_partitions,                          \
		MTD_FIXED_PARTITIONS_DEFINE, _compatible)

#endif /* MTD_DEFINE_H_ */
