/*
 * Copyright (c) 2024 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/zbus/zbus.h>
#include <zephyr/sys/util.h>
#include <zephyr/task_wdt/task_wdt.h>

#include "message_channel.h"

/* Register log module */
LOG_MODULE_REGISTER(watchdog, CONFIG_APP_WATCHDOG_LOG_LEVEL);

static void watchdog_task(void)
{
	int err;
	int unused;
	const struct device *const hw_wdt_dev = DEVICE_DT_GET_OR_NULL(WDT_NODE);

	LOG_DBG("Watchdog task.");

	if (!device_is_ready(hw_wdt_dev)) {
		LOG_ERR("Hardware watchdog not ready.");
		SEND_FATAL_ERROR();
		return;
	}

	err = task_wdt_init(hw_wdt_dev);
	if (err != 0) {
		LOG_ERR("task wdt init failure: %d.", err);
		SEND_FATAL_ERROR();
		return;
	}

	while (1) {
		err = zbus_chan_pub(&WATCHDOG_CHAN, &unused, K_FOREVER);
		if (err) {
			LOG_ERR("zbus_chan_pub, error: %d", err);
			SEND_FATAL_ERROR();
			return;
		}
		k_sleep(K_SECONDS(CONFIG_APP_GLOBAL_WATCHDOG_INTERVAL_SECONDS));
	}
}

K_THREAD_DEFINE(watchdog_task_id,
		CONFIG_APP_WATCHDOG_THREAD_STACK_SIZE,
		watchdog_task, NULL, NULL, NULL, 3, 0, 0);
