/*
 * Copyright (c) 2017 Nordic Semiconductor ASA
 * Copyright (c) 2015 Runtime Inc
 * Copyright (c) 2020 Gerson Fernando Budke <nandojve@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/ztest.h>
#include <zephyr/nvmp/nvmp.h>

/**
 * @brief Test nvmp_api()
 */
ZTEST(nvmp, test_nvmp_api_slot1)
{
	const struct nvmp_info *info = NVMP_GET(slot1_partition);
	uint8_t wd[256];
	uint8_t rd[256];
	off_t off = 0;
	int rc;

	(void)memset(wd, 0xa5, sizeof(wd));

	rc = nvmp_write(info, off, wd, sizeof(wd));
	zassert_true(rc == 0, "write() fail");

	/* read it back */
	rc = nvmp_read(info, off, rd, sizeof(rd));
	zassert_true(rc == 0, "read() fail");

	rc = memcmp(wd, rd, sizeof(wd));
	zassert_true(rc == 0, "read data != write data");

	/* erase it */
	rc = nvmp_erase(info, 0, nvmp_get_size(info));
	zassert_true(rc == 0, "read data != write data");
}

ZTEST(nvmp, test_nvmp_api_slot0)
{
	const struct nvmp_info *info = NVMP_GET(slot0_partition);
	uint8_t wd[256];
	uint8_t rd[256];
	off_t off = 0;
	int rc;

	(void)memset(wd, 0xa5, sizeof(wd));

	rc = nvmp_write(info, off, wd, sizeof(wd));
	zassert_true(rc == 0, "write() fail");

	/* read it back */
	rc = nvmp_read(info, off, rd, sizeof(rd));
	zassert_true(rc == 0, "read() fail");

	rc = memcmp(wd, rd, sizeof(wd));
	zassert_true(rc == 0, "read data != write data");

	/* erase it */
	rc = nvmp_erase(info, 0, nvmp_get_size(info));
	zassert_true(rc == 0, "read data != write data");
}

ZTEST_SUITE(nvmp, NULL, NULL, NULL, NULL, NULL);
