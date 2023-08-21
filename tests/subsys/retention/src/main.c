/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/ztest.h>
#include <zephyr/retention/retention.h>

static const struct device *ret_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_boot_mode));

static void *retention_api_setup(void)
{
	if (IS_ENABLED(CONFIG_USERSPACE)) {
		k_object_access_grant(ret_dev, k_current_get());
	}

	return NULL;
}

ZTEST_USER(retention_api, test_get_size)
{
	size_t bi_size = retention_size(ret_dev);

	zassert_not_equal(bi_size, 0U, "Get size returned invalid value");
}

ZTEST_USER(retention_api, test_get_set)
{
	uint8_t wr[retention_size(ret_dev)];
	uint8_t rd[retention_size(ret_dev)];
	int rc = 0;

	memset(wr, 0, sizeof(wr));
	memset(rd, 0, sizeof(rd));

	rc = retention_read(ret_dev, 0, wr, sizeof(wr));
	zassert_equal(rc, 0, "retention_read returned [%d]", rc);

	memset(wr, 0xa, sizeof(wr));
	rc = retention_write(ret_dev, 0, wr, sizeof(wr));
	zassert_equal(rc, 0, "retention_write returned [%d]", rc);

	rc = retention_read(ret_dev, 0, rd, sizeof(rd));
	zassert_equal(rc, 0, "retention_read returned [%d]", rc);

	zassert_equal(memcmp(rd, wr, sizeof(wr)), 0, "data mismatch");
}

ZTEST_SUITE(retention_api, NULL, retention_api_setup, NULL, NULL, NULL);
