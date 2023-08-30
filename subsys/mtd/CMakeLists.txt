# SPDX-License-Identifier: Apache-2.0

zephyr_library()
zephyr_library_sources(mtd.c)
zephyr_library_sources_ifdef(CONFIG_MTD_FLASH mtd_flash.c)
zephyr_library_sources_ifdef(CONFIG_MTD_EEPROM mtd_eeprom.c)
zephyr_library_sources_ifdef(CONFIG_MTD_RETAINED_MEM mtd_retained_mem.c)
