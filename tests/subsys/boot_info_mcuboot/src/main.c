/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/crc.h>

#define BOOT_INFO 0

/* This is needlessly complex but otherwise comparison with retention
 * would be invalid. In practice the address and size would be added
 * in the mcuboot_config.h file using the devicetree macros below.
 */
#define BOOT_INFO_NODE DT_PARENT(DT_NODELABEL(retainedmem0))
#define BOOT_INFO_ADDR DT_REG_ADDR(BOOT_INFO_NODE)
#define BOOT_INFO_SIZE DT_REG_SIZE(BOOT_INFO_NODE)

#define boot_info_get_size(_bootinfo) BOOT_INFO_SIZE

/* This is needlessly complex but otherwise comparison with retention
 * would be invalid. In practice mcuboot would just use memcpy().
 */
int boot_info_set(int bootinfo, void *data)
{
	memcpy((void *)BOOT_INFO_ADDR, data, BOOT_INFO_SIZE);
	return 0;
}

/* This is needlessly complex but otherwise comparison with retention
 * would be invalid. In practice mcuboot would just use memcpy().
 */
int boot_info_get(int bootinfo, void *data)
{
	memcpy(data, (void *)BOOT_INFO_ADDR, BOOT_INFO_SIZE);
	return 0;
}

const uint8_t *hdr = "\x08\x04";

static void make_valid(uint8_t *data, size_t len)
{
	uint8_t crc8;

	memcpy(data, hdr, sizeof(hdr));
	crc8 = crc8_ccitt(0U, data, len - sizeof(crc8));
	memcpy(data + len - sizeof(crc8), &crc8, sizeof(crc8));
}

static bool is_valid(uint8_t *data, size_t len)
{
	if (memcmp(data, hdr, sizeof(hdr)) != 0) {
		return false;
	}

	uint8_t crc8 = crc8_ccitt(0U, data, len - sizeof(crc8));

	if (memcmp(data + len - sizeof(crc8), &crc8, sizeof(crc8)) != 0) {
		return false;
	}

	return true;
}

static void *boot_info_api_setup(void)
{
	return NULL;
}

ZTEST_USER(boot_info_api, test_get_size)
{
	size_t bi_size = boot_info_get_size(BOOT_INFO);

	zassert_not_equal(bi_size, 0U, "Get size returned invalid value");
}

ZTEST_USER(boot_info_api, test_get_set)
{
	uint8_t wr[boot_info_get_size(BOOT_INFO)];
	uint8_t rd[boot_info_get_size(BOOT_INFO)];
	int rc = 0;

	memset(wr, 0, sizeof(wr));
	memset(rd, 0, sizeof(rd));

	rc = boot_info_get(BOOT_INFO, wr);
	zassert_equal(rc, 0, "boot_info_get returned [%d]", rc);

	memset(wr + sizeof(hdr), 0xa, sizeof(wr) - sizeof(hdr) - sizeof(uint32_t));
	make_valid(wr, sizeof(wr));

	rc = boot_info_set(BOOT_INFO, wr);
	zassert_equal(rc, 0, "boot_info_set returned [%d]", rc);

	rc = boot_info_get(BOOT_INFO, rd);
	zassert_equal(rc, 0, "boot_info_get returned [%d]", rc);

	zassert_equal(is_valid(rd, sizeof(rd)), true, "boot_info data is invalid");

	zassert_equal(memcmp(rd, wr, sizeof(wr)), 0, "data mismatch");
}

ZTEST_SUITE(boot_info_api, NULL, boot_info_api_setup, NULL, NULL, NULL);
