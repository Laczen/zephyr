/*
 * Copyright (c) 2023, Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/mtd/mtd.h>

#define MTD_PARTITION_SIZE(inst) DT_REG_SIZE(inst)
#define MTD_PARTITION_OFF(inst) DT_REG_ADDR(inst)
#define MTD_PARTITION_PARENT(inst) UTIL_CAT(mtd_info_, DT_GPARENT(inst))

#define MTD_PARTITION_DEFINE(inst)						\
	MTD_INFO_DEFINE(inst, NULL, &MTD_PARTITION_PARENT(inst),		\
			MTD_PARTITION_OFF(inst), MTD_PARTITION_SIZE(inst),	\
			MTD_RO(inst));

#define MTD_FIXEDPARTITION_DEFINE(n) DT_FOREACH_CHILD(n, MTD_PARTITION_DEFINE)

DT_FOREACH_STATUS_OKAY(fixed_partitions, MTD_FIXEDPARTITION_DEFINE)
