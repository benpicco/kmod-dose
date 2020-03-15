// SPDX-License-Identifier: GPL-2.0+
// DOSE for Linux
// Copyright (c) 2020 by Benjamin Valentin <benjamin.valentin@ml-pa.com>

#include <linux/serdev.h>
#include <linux/module.h>

static int __init init_dose(void)
{
	printk(KERN_INFO "Hello, Linux!\n");
	return 0;
}

static void __exit exit_dose(void)
{
	printk(KERN_INFO "Goodbye, Linux!\n");
}

module_init(init_dose)
module_exit(exit_dose)

MODULE_LICENSE("GPL");
