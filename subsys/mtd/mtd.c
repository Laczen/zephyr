/*
 * Copyright (c) 2023, Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#include <errno.h>
#include <zephyr/mtd/mtd.h>

static bool region_bounds_error(size_t region_size, off_t off, size_t len)
{
	if ((region_size < len) || ((region_size - len) < (size_t)off)) {
		return true;
	}

	return false;
}

static const struct mtd_dev_cfg *mtd_get(const struct mtd_info *info,
					 off_t *off)
{
	struct mtd_info *wlk = (struct mtd_info *)info;
	off_t wlk_off = 0;

	while (wlk->dev_cfg == NULL) {
		wlk_off += wlk->off;
		wlk = (struct mtd_info *)wlk->parent;
	}

	wlk_off += wlk->off;
	if (off != NULL) {
		*off += wlk_off;
	}

	return wlk->dev_cfg;
}

size_t mtd_get_size(const struct mtd_info *info)
{
	return info->size;
}

const struct device *mtd_get_device(const struct mtd_info *info)
{
	const struct mtd_dev_cfg *dev_cfg = mtd_get(info, NULL);

	return dev_cfg->dev;
}

enum mtd_type mtd_get_device_type(const struct mtd_info *info)
{
	const struct mtd_dev_cfg *dev_cfg = mtd_get(info, NULL);

	return dev_cfg->type;
}

off_t mtd_get_device_offset(const struct mtd_info *info)
{
	off_t off = 0;
	const struct mtd_dev_cfg *dev_cfg = mtd_get(info, &off);

	ARG_UNUSED(dev_cfg);
	return off;
}

int mtd_read(const struct mtd_info *info, off_t off, void *dst, size_t len)
{
	if ((info == NULL) || (region_bounds_error(info->size, off, len))) {
		return -EINVAL;
	}

	off_t dev_off = 0;
	const struct mtd_dev_cfg *dev_cfg = mtd_get(info, &dev_off);

	return dev_cfg->read(dev_cfg, dev_off, dst, len);
}

int mtd_write(const struct mtd_info *info, off_t off, const void *src,
	      size_t len)
{
	if ((info == NULL) || (region_bounds_error(info->size, off, len))) {
		return -EINVAL;
	}

	if (info->read_only) {
		return -EACCES;
	}

	off_t dev_off = 0;
	const struct mtd_dev_cfg *dev_cfg = mtd_get(info, &dev_off);

	return dev_cfg->write(dev_cfg, dev_off, src, len);
}

int mtd_erase(const struct mtd_info *info, off_t off, size_t len)
{
	if ((info == NULL) || (region_bounds_error(info->size, off, len))) {
		return -EINVAL;
	}

	if (info->read_only) {
		return -EACCES;
	}

	off_t dev_off = 0;
	const struct mtd_dev_cfg *dev_cfg = mtd_get(info, &dev_off);

	return dev_cfg->erase(dev_cfg, dev_off, len);
}
