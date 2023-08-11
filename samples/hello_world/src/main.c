/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>

#ifdef CONFIG_RETENTION
#include <zephyr/retention/retention.h>

static const struct device *ret_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_boot_mode));
#endif

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);
#ifdef CONFIG_RETENTION
	uint8_t dummy[3];
	size_t bi_size = retention_size(ret_dev);
	int rc = retention_read(ret_dev, 0, dummy, sizeof(dummy));

	rc = retention_write(ret_dev, 0, dummy, sizeof(dummy));
#endif

	return 0;
}
