/*
 * Copyright (c) 2023, Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/nvmp/nvmp.h>

size_t nvmp_get_size(const struct nvmp_info *info)
{
	if (info == NULL) {
		return 0U;
	}

	return info->size;
}

enum nvmp_type nvmp_get_type(const struct nvmp_info *info)
{
	if (info == NULL) {
		return UNKNOWN;
	}

	return info->type;
}

off_t nvmp_get_store_offset(const struct nvmp_info *info)
{
	if (info == NULL) {
		return -1;
	}

	return info->store_off;
}

const void *nvmp_get_store(const struct nvmp_info *info)
{
	if (info == NULL) {
		return NULL;
	}

	return info->store;
}

int nvmp_read(const struct nvmp_info *info, off_t off, void *data, size_t len)
{
	if (info == NULL) {
		return -EINVAL;
	}

	return info->read(info, off, data, len);
}

int nvmp_write(const struct nvmp_info *info, off_t off, const void *data,
	       size_t len)
{
	if (info == NULL) {
		return -EINVAL;
	}

	return info->write(info, off, data, len);
}

int nvmp_erase(const struct nvmp_info *info, off_t off, size_t len)
{
	if (info == NULL) {
		return -EINVAL;
	}

	return info->erase(info, off, len);
}
