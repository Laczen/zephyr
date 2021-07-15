/*
 * Copyright (c) 2021 Laczen
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <drivers/flash_partition.h>

/* Helper functions */
#define OUTSIDE_RANGE(val, min, max) (val < min || val > max)

static int read_arguments_check(const struct flp_info_cfg *cfg, off_t off,
				size_t len)
{

	if (OUTSIDE_RANGE(off, 0, cfg->size) ||
	    OUTSIDE_RANGE(off + len, 0, cfg->size)) {
		return -EINVAL;
	}

	return 0;
}

static int writeable_check(const struct flp_info_cfg *cfg)
{
	if (cfg->read_only) {
		return -EROFS;
	}

	return 0;
}

static int write_arguments_check(const struct flp_info_cfg *cfg, off_t off,
				 size_t len)
{
	int rc;

	rc = read_arguments_check(cfg, off, len);
	if (rc) {
		return rc;
	}

	rc = writeable_check(cfg);

	return rc;
}

static struct flp_info *flp_get_master(const struct flp_info *flp)
{
	struct flp_info *master = (struct flp_info *)flp;

	while (master->cfg->parent) {
		master = (struct flp_info *)master->cfg->parent;
	}

	return master;
}

static off_t flp_get_master_offset(const struct flp_info *flp, off_t off)
{
	struct flp_info *master = (struct flp_info *)flp;
	off_t ret = off;

	while (master->cfg->parent) {
		ret += master->cfg->off;
		master = (struct flp_info *)master->cfg->parent;
	}

	ret += master->cfg->off;

	return ret;
}
/* End helper functions */

/* Public functions */
int flp_read(const struct flp_info *flp, off_t off, void *dst, size_t len)
{
	if (flp == NULL) {
		return -EINVAL;
	}

	int rc;

	rc = read_arguments_check(flp->cfg, off, len);
	if (rc) {
		return rc;
	}

	struct flp_info *master = flp_get_master(flp);

	if (!device_is_ready(master->cfg->device)) {
		return -EIO;
	}

	off = flp_get_master_offset(flp, off);

	return flash_read(master->cfg->device, off, dst, len);
}

int flp_write(const struct flp_info *flp, off_t off, const void *src,
	      size_t len)
{
	int rc;

	if (flp == NULL) {
		return -EINVAL;
	}

	rc = write_arguments_check(flp->cfg, off, len);
	if (rc) {
		return rc;
	}

	struct flp_info *master = flp_get_master(flp);

	if (!device_is_ready(master->cfg->device)) {
		return -EIO;
	}

	off = flp_get_master_offset(flp, off);

	return flash_write(master->cfg->device, off, (void *)src, len);
}

int flp_erase(const struct flp_info *flp, off_t off, size_t len)
{
	int rc;

	if (flp == NULL) {
		return -EINVAL;
	}

	rc = write_arguments_check(flp->cfg, off, len);
	if (rc) {
		return rc;
	}

	struct flp_info *master = flp_get_master(flp);

	if (!device_is_ready(master->cfg->device)) {
		return -EIO;
	}
	off = flp_get_master_offset(flp, off);

	return flash_erase(master->cfg->device, off, len);
}

const struct flash_parameters *flp_get_parameters(const struct flp_info *flp)
{
	if (flp == NULL) {
		return NULL;
	}

	struct flp_info *master = flp_get_master(flp);

	return flash_get_parameters(master->cfg->device);
}

#if IS_ENABLED(CONFIG_FLASH_PAGE_LAYOUT)
struct foreachblock_ctx {
	off_t start;
	off_t end;
	struct flash_pages_info *info;
	flash_page_cb cb;
	void *cb_data;
};

static bool foreachblock_cb(const struct flash_pages_info *info, void *ctxp)
{
	struct foreachblock_ctx *ctx = (struct foreachblock_ctx *)ctxp;

	if (ctx->start > info->start_offset) {
		return true;
	}

	if (ctx->end <= info->start_offset) {
		return false;
	}

	ctx->info->size = info->size;

	if (!ctx->cb(ctx->info, ctx->cb_data)) {
		return false;
	}

	ctx->info->start_offset += info->size;
	ctx->info->index++;

	return true;
}

void flp_page_foreach(const struct flp_info *flp, flash_page_cb cb, void *data)
{
	if (flp == NULL) {
		return;
	}

	struct flash_pages_info flp_info = {
		.start_offset = 0,
		.index = 0U,
	};

	struct flp_info *master = flp_get_master(flp);
	struct foreachblock_ctx ctx = {
		.start = flp_get_master_offset(flp, 0),
		.end = flp_get_master_offset(flp, 0) + flp->cfg->size,
		.info = &flp_info,
		.cb = cb,
		.cb_data = data,
	};

	flash_page_foreach(master->cfg->device, foreachblock_cb, &ctx);
}

int flp_get_page_info_by_offs(const struct flp_info *flp, off_t offset,
			      struct flash_pages_info *info)
{
	if ((flp == NULL) || OUTSIDE_RANGE(offset, 0, flp->cfg->size)) {
		return -EINVAL;
	}

	int rc;
	struct flp_info *master = flp_get_master(flp);
	off_t off = flp_get_master_offset(flp, 0);
	struct flash_pages_info st_info, p_info;

	rc = flash_get_page_info_by_offs(master->cfg->device, off, &st_info);
	if (rc) {
		return rc;
	}

	off += offset;
	rc = flash_get_page_info_by_offs(master->cfg->device, off, &p_info);
	if (rc) {
		return rc;
	}

	info->start_offset = p_info.start_offset - st_info.start_offset;
	info->size = p_info.size;
	info->index = p_info.index - st_info.index;

	return 0;
}

int flp_get_page_info_by_idx(const struct flp_info *flp, uint32_t idx,
			     struct flash_pages_info *info)
{
	if (flp == NULL) {
		return -EINVAL;
	}

	int rc;
	struct flp_info *master = flp_get_master(flp);
	off_t off = flp_get_master_offset(flp, 0);
	struct flash_pages_info st_info, p_info;

	rc = flash_get_page_info_by_offs(master->cfg->device, off, &st_info);
	if (rc) {
		return rc;
	}

	idx += st_info.index;
	rc = flash_get_page_info_by_idx(master->cfg->device, idx, &p_info);
	if (rc) {
		return rc;
	}

	info->start_offset = p_info.start_offset - st_info.start_offset;
	info->size = p_info.size;
	info->index = p_info.index - st_info.index;

	return 0;
}

size_t flp_get_page_count(const struct flp_info *flp)
{
	if (flp == NULL) {
		return 0U;
	}

	int rc;
	struct flp_info *master = flp_get_master(flp);
	off_t off = flp_get_master_offset(flp, 0);
	struct flash_pages_info st_info, p_info;

	rc = flash_get_page_info_by_offs(master->cfg->device, off, &st_info);
	if (rc) {
		return 0U;
	}

	off += flp->cfg->size;
	rc = flash_get_page_info_by_offs(master->cfg->device, off, &p_info);
	if (rc) {
		return 0U;
	}

	return (p_info.index - st_info.index);
}
#endif
/* End public functions */

#define DT_DRV_COMPAT fixed_partitions

#define GEN_FLP_INFO(_name, _dev, _parent, _off, _size, _ro) \
	static const struct flp_info_cfg UTIL_CAT(_name, _cfg) = { \
		.device = _dev, \
		.parent = _parent, \
		.off = _off, \
		.size = _size, \
		.read_only = _ro, \
	}; \
	static struct flp_info_state UTIL_CAT(_name, _state); \
	const struct flp_info _name = { \
		.cfg = &UTIL_CAT(_name, _cfg), \
		.state = &UTIL_CAT(_name, _state), \
	};

#define GET_PART_SIZE(n) DT_PROP_OR(n, size, DT_REG_SIZE(n))

#define GET_PARTITION_STRUCT(n) UTIL_CAT(flp_, DT_NODELABEL(n))

#define GGPARENT(n) DT_PARENT(DT_GPARENT(n))

#define GET_PARTITION_DEVICE(n) \
	COND_CODE_1(DT_NODE_HAS_COMPAT(GGPARENT(n), fixed_partitions), \
		(NULL), (DEVICE_DT_GET(DT_MTD_FROM_FIXED_PARTITION(n))))

#define GET_PARTITION_PARENT_PTR(n) \
	COND_CODE_1(DT_NODE_HAS_COMPAT(GGPARENT(n), fixed_partitions), \
		(&GET_PARTITION_STRUCT(DT_GPARENT(n))), (NULL))

#define GEN_FLP_STRUCT(n) \
	GEN_FLP_INFO(UTIL_CAT(flp_, DT_NODELABEL(n)), \
		     GET_PARTITION_DEVICE(n), \
		     GET_PARTITION_PARENT_PTR(n), \
		     DT_REG_ADDR(n), GET_PART_SIZE(n), \
		     DT_PROP(n, read_only))

#define FOREACH_PARTITION(n) \
	DT_FOREACH_CHILD(DT_DRV_INST(n), GEN_FLP_STRUCT)

DT_INST_FOREACH_STATUS_OKAY(FOREACH_PARTITION)
