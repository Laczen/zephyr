/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/mtd/mtd.h>

#define MTD_FLASH_NODE flash_sim0
#define MTD_FLASH_PARTITION_NODE storage_partition
#define MTD_EEPROM_NODE eeprom0
#define MTD_RETAINED_MEM_NODE retainedmem0

ZTEST_BMEM static const struct mtd_info *mtd;

static void *mtd_api_setup(void)
{
	return NULL;
}

ZTEST_USER(mtd_api, test_read_write_erase)
{
	uint8_t *wr = "/a9/9a/a9/9a";
	uint8_t rd[sizeof(wr)];
	enum mtd_type type = mtd_get_device_type(mtd);
	int rc = 0;

	memset(rd, 0, sizeof(rd));

	zassert_false(type == UNKNOWN, "mtd has bad type");

	rc = mtd_read(mtd, 0, rd, sizeof(rd));
	zassert_equal(rc, 0, "mtd_read returned [%d]", rc);

	rc = mtd_write(mtd, 0, wr, sizeof(wr));
	zassert_equal(rc, 0, "mtd_write returned [%d]", rc);

	rc = mtd_read(mtd, 0, rd, sizeof(rd));
	zassert_equal(rc, 0, "mtd_read returned [%d]", rc);

	zassert_equal(memcmp(wr, rd, sizeof(wr)), 0, "read/write data differ");



	if ((IS_ENABLED(CONFIG_MTD_FLASH)) && (type == FLASH)) {
		const struct device *dev = mtd_get_device(mtd);
		const struct flash_parameters *flparam =
			flash_get_parameters(dev);

		rc = mtd_erase(mtd, 0, mtd_get_size(mtd));
		zassert_equal(rc, 0, "mtd_erase returned [%d]", rc);

		rc = mtd_read(mtd, 0, rd, sizeof(rd));
		zassert_equal(rc, 0, "read returned [%d]", rc);

		for (int i = 0; i < sizeof(rd); i++) {
			zassert_true(rd[i] == flparam->erase_value,
				     "erase failed");
		}
	}

	if ((IS_ENABLED(CONFIG_MTD_EEPROM) && (type == EEPROM)) ||
	    (IS_ENABLED(CONFIG_MTD_RETAINED_MEM) && (type == RETAINED_MEM))) {
		rc = mtd_erase(mtd, 0, sizeof(rd));
		zassert_equal(rc, 0, "mtd_erase returned [%d]", rc);

		rc = mtd_read(mtd, 0, rd, sizeof(rd));
		zassert_equal(rc, 0, "read returned [%d]", rc);

		for (int i = 0; i < sizeof(rd); i++) {
			zassert_true(rd[i] == CONFIG_MTD_ERASE_VALUE,
				     "erase failed");
		}
	}
}

ZTEST_SUITE(mtd_api, NULL, mtd_api_setup, NULL, NULL, NULL);

/* Run all of our tests on the given mtd */
static void run_tests_on_mtd(const struct mtd_info *info)
{
	mtd = info;
	k_object_access_grant(mtd_get_device(info), k_current_get());
	ztest_run_all(NULL);
}

void test_main(void)
{
#ifdef CONFIG_MTD_FLASH
	run_tests_on_mtd(MTD_GET(MTD_FLASH_NODE));
	run_tests_on_mtd(MTD_GET(MTD_FLASH_PARTITION_NODE));
#endif

#ifdef CONFIG_MTD_EEPROM
	run_tests_on_mtd(MTD_GET(MTD_EEPROM_NODE));
#endif

#ifdef CONFIG_MTD_RETAINED_MEM
	run_tests_on_mtd(MTD_GET(MTD_RETAINED_MEM_NODE));
#endif

	ztest_verify_all_test_suites_ran();
}
