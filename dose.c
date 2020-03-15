// SPDX-License-Identifier: GPL-2.0+
// DOSE for Linux
// Copyright (c) 2020 by Benjamin Valentin <benjamin.valentin@ml-pa.com>

#include <linux/mod_devicetable.h>
#include <linux/serdev.h>
#include <linux/module.h>

static int dose_probe(struct serdev_device *serdev)
{
	printk(KERN_INFO "Hello, DOSE!\n");

	return 0;
}

static const struct of_device_id dose_of_match[] = {
	{ .compatible = "riot,dose" },
	{ }
};
MODULE_DEVICE_TABLE(of, dose_of_match);

static struct serdev_device_driver dose_driver = {
	.driver = {
		.name = "dose",
		.of_match_table = dose_of_match,
	},
	.probe = dose_probe,
};
module_serdev_device_driver(dose_driver);

MODULE_DESCRIPTION("DOSE");
MODULE_AUTHOR("Benjamin Valentin <benjamin.valentin@ml-pa.com>");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");
