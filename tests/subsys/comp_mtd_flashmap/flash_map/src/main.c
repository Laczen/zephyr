/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <zephyr/storage/flash_map.h>

#define SLOT1_PARTITION		slot1_partition
#define SLOT1_PARTITION_ID	FIXED_PARTITION_ID(SLOT1_PARTITION)
#define SLOT1_PARTITION_DEV	FIXED_PARTITION_DEVICE(SLOT1_PARTITION)

#define SLOT0_PARTITION		slot0_partition
#define SLOT0_PARTITION_ID	FIXED_PARTITION_ID(SLOT0_PARTITION)
#define SLOT0_PARTITION_DEV	FIXED_PARTITION_DEVICE(SLOT0_PARTITION)

/**
 * @brief Test flash_area_api()
 */
ZTEST(flash_map, test_flash_area_api_slot1)
{
	const struct flash_area *fa;
	uint8_t wd[256];
	uint8_t rd[256];
	off_t off = 0;
	int rc;

	rc = flash_area_open(SLOT1_PARTITION_ID, &fa);
	zassert_true(rc == 0, "open fail");

	(void)memset(wd, 0xa5, sizeof(wd));

	rc = flash_area_write(fa, off, wd, sizeof(wd));
	zassert_true(rc == 0, "write() fail");

	/* read it back */
	rc = flash_area_read(fa, off, rd, sizeof(rd));
	zassert_true(rc == 0, "read() fail");

	rc = memcmp(wd, rd, sizeof(wd));
	zassert_true(rc == 0, "read data != write data");

	/* erase it */
	rc = flash_area_erase(fa, 0, fa->fa_size);
	zassert_true(rc == 0, "erase() fail");

	flash_area_close(fa);
}

ZTEST(flash_map, test_flash_area_api_slot0)
{
	const struct flash_area *fa;
	uint8_t wd[256];
	uint8_t rd[256];
	off_t off = 0;
	int rc;

	rc = flash_area_open(SLOT0_PARTITION_ID, &fa);
	zassert_true(rc == 0, "open fail");

	(void)memset(wd, 0xa5, sizeof(wd));

	rc = flash_area_write(fa, off, wd, sizeof(wd));
	zassert_true(rc == 0, "write() fail");

	/* read it back */
	rc = flash_area_read(fa, off, rd, sizeof(rd));
	zassert_true(rc == 0, "read() fail");

	rc = memcmp(wd, rd, sizeof(wd));
	zassert_true(rc == 0, "read data != write data");

	/* erase it */
	rc = flash_area_erase(fa, 0, fa->fa_size);
	zassert_true(rc == 0, "erase() fail");

	flash_area_close(fa);
}

ZTEST_SUITE(flash_map, NULL, NULL, NULL, NULL, NULL);
