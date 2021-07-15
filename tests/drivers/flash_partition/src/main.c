/*
 * Copyright (c) 2021 Laczen
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <ztest.h>
#include <drivers/flash_partition.h>

/* Helper functions */
/* Get the master partition of a partition (contains the device) */
static struct flp_info *flp_get_master(const struct flp_info *flp)
{
	struct flp_info *master = (struct flp_info *)flp;

	while (master->cfg->parent) {
		master = (struct flp_info *)master->cfg->parent;
	}

	return master;
}

/* Recalculate offset to master offset */
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
/* End Helper functions */

/**
 * @brief Test flp partition read/write
 */
void test_flp_rw_on(const struct flp_info *flp)
{
	int rc;
	struct flp_info *master = flp_get_master(flp);
	off_t off, off_m;
	size_t sec_size = 4096;
	uint8_t wd[256];
	uint8_t rd[256];

	/* First erase the area so it's ready for use. */
	off_m = flp_get_master_offset(flp, 0);
	rc = flash_erase(master->cfg->device, off_m, flp->cfg->size);
	zassert_true(rc == 0, "hal_flash_erase() fail [rc: %d]", rc);

	(void)memset(wd, 0xa5, sizeof(wd));

	/* write stuff to beginning of every sector */
	off = 0;
	while (off < flp->cfg->size) {
		rc = flp_write(flp, off, wd, sizeof(wd));
		zassert_true(rc == 0, "flp_write() fail [rc: %d]", rc);

		/* read it back via hal_flash_read() */
		off_m = flp_get_master_offset(flp, off);
		rc = flash_read(master->cfg->device, off_m, rd, sizeof(rd));
		zassert_true(rc == 0, "hal_flash_read() fail [rc: %d]", rc);

		rc = memcmp(wd, rd, sizeof(wd));
		zassert_true(rc == 0, "read data != write data");

		/* write stuff to end of area via hal_flash_write */
		off_m = flp_get_master_offset(flp, off + sec_size - sizeof(wd));
		rc = flash_write(master->cfg->device, off_m, wd, sizeof(wd));
		zassert_true(rc == 0, "hal_flash_write() fail [rc: %d]", rc);

		/* and read it back */
		(void)memset(rd, 0, sizeof(rd));
		rc = flp_read(flp, off + sec_size - sizeof(rd), rd, sizeof(rd));
		zassert_true(rc == 0, "flp_read() fail [rc: %d]", rc);

		rc = memcmp(wd, rd, sizeof(rd));
		zassert_true(rc == 0, "read data != write data");

		off += sec_size;
	}

	/* erase it */
	rc = flp_erase(flp, 0, flp->cfg->size);
	zassert_true(rc == 0, "flp_erase() fail");

	/* should read back ff all throughout*/
	(void)memset(wd, 0xff, sizeof(wd));
	for (off = 0; off < flp->cfg->size; off += sizeof(rd)) {
		rc = flp_read(flp, off, rd, sizeof(rd));
		zassert_true(rc == 0, "flp_read() fail");

		rc = memcmp(wd, rd, sizeof(rd));
		zassert_true(rc == 0, "area not erased");
	}
}

void test_flp_rw(void)
{
	test_flp_rw_on(FLASH_PARTITION_GET(image_0));
	test_flp_rw_on(FLASH_PARTITION_GET(mcu_sub));
	test_flp_rw_on(FLASH_PARTITION_GET(image_0_sub));
}

void test_flp_get_parameters(void)
{
	const struct flp_info *flp = FLASH_PARTITION_GET(image_0);
	const struct flash_parameters *param1, *param2;

	param1 = flp_get_parameters(flp);
	param2 = flash_get_parameters(flp->cfg->device);

	zassert_equal(param1->write_block_size, param2->write_block_size,
		      "write-block-size differs");
	zassert_equal(param1->erase_value, param2->erase_value,
		      "erase-value differs");

}

static bool get_page_cb(const struct flash_pages_info *info, void *data)
{
	uint32_t *counter = (uint32_t *)data;

	(*counter) += 1;
	return true;
}

void test_foreach_page_on(const struct flp_info *flp)
{
	uint32_t page_counter = 0;

	flp_page_foreach(flp, get_page_cb, &page_counter);
	zassert_true(page_counter, "No pages found [%d]", page_counter);
}

void test_foreach_page(void)
{
	test_foreach_page_on(FLASH_PARTITION_GET(image_0));
	test_foreach_page_on(FLASH_PARTITION_GET(image_0_sub));
}

void test_get_pages_info_on(const struct flp_info *flp)
{
	int rc;
	uint32_t pages;
	struct flash_pages_info info;
	uint32_t first_page_size;

	pages = flp_get_page_count(flp);
	zassert_true(pages, "No pages found [%d]", pages);

	rc = flp_get_page_info_by_offs(flp, 0, &info);
	zassert_false(rc, "get_page_info_by_offs returned [%d]", rc);
	zassert_equal(info.start_offset, 0, "wrong start offset");
	zassert_equal(info.index, 0, "wrong index");

	first_page_size = info.size;
	rc = flp_get_page_info_by_offs(flp, first_page_size, &info);
	zassert_false(rc, "get_page_info_by_offs returned [%d]", rc);
	zassert_equal(info.start_offset, first_page_size, "wrong start offset");
	zassert_equal(info.index, 1, "wrong index");

	rc = flp_get_page_info_by_idx(flp, 0, &info);
	zassert_false(rc, "get_page_info_by_offs returned [%d]", rc);
	zassert_equal(info.start_offset, 0, "wrong start offset");
	zassert_equal(info.index, 0, "wrong index");

	first_page_size = info.size;
	rc = flp_get_page_info_by_idx(flp, 1, &info);
	zassert_false(rc, "get_page_info_by_offs returned [%d]", rc);
	zassert_equal(info.start_offset, first_page_size, "wrong start offset");
	zassert_equal(info.index, 1, "wrong index");
}

void test_get_pages_info(void)
{
	test_get_pages_info_on(FLASH_PARTITION_GET(image_0));
	test_get_pages_info_on(FLASH_PARTITION_GET(image_0_sub));
}
void test_main(void)
{
	ztest_test_suite(test_flp,
			 ztest_unit_test(test_flp_rw),
			 ztest_unit_test(test_flp_get_parameters),
			 ztest_unit_test(test_foreach_page),
			 ztest_unit_test(test_get_pages_info)
	);
	ztest_run_test_suite(test_flp);
}
