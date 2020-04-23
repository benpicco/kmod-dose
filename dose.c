// SPDX-License-Identifier: GPL-2.0+
// DOSE for Linux
// Copyright (c) 2020 by Benjamin Valentin <benjamin.valentin@ml-pa.com>

#include <linux/mod_devicetable.h>
#include <linux/etherdevice.h>
#include <linux/serdev.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/of.h>

struct dose {
	struct net_device *net_dev;
	spinlock_t lock;			/* transmit lock */
	struct work_struct tx_work;		/* Flushes transmit buffer   */

	struct serdev_device *serdev;
};

static int
dose_tty_receive(struct serdev_device *serdev, const unsigned char *data,
		 size_t count)
{
	dev_info(&serdev->dev, "got %d bytes: '%s'\n", count, data);

	return count;
}

/* Write out any remaining transmit buffer. Scheduled when tty is writable */
static void dose_transmit(struct work_struct *work)
{
	struct dose *dose = container_of(work, struct dose, tx_work);

	spin_lock_bh(&dose->lock);

	serdev_device_write_buf(dose->serdev, "Hello from Linux\n", 19);

	spin_unlock_bh(&dose->lock);
}

/* Called by the driver when there's room for more data.
 * Schedule the transmit.
 */
static void dose_tty_wakeup(struct serdev_device *serdev)
{
	struct dose *dose = serdev_device_get_drvdata(serdev);

	schedule_work(&dose->tx_work);
}

static struct serdev_device_ops dose_serdev_ops = {
	.receive_buf = dose_tty_receive,
    .write_wakeup = serdev_device_write_wakeup,
//	.write_wakeup = dose_tty_wakeup,
};

static int dose_probe(struct serdev_device *serdev)
{
	struct net_device *dose_dev = alloc_etherdev(sizeof(struct dose));
	struct dose *dose;
	int ret;
	u32 speed = 115200;

	if (!dose_dev)
		return -ENOMEM;

	SET_NETDEV_DEV(dose_dev, &serdev->dev);

	dose = netdev_priv(dose_dev);
	if (!dose) {
		pr_err("dose: Fail to retrieve private structure\n");
		ret = -ENOMEM;
		goto free;
	}

	dose->net_dev = dose_dev;
	dose->serdev = serdev;

	spin_lock_init(&dose->lock);
	INIT_WORK(&dose->tx_work, dose_transmit);

	of_property_read_u32(serdev->dev.of_node, "current-speed", &speed);

	serdev_device_set_drvdata(serdev, dose);
	serdev_device_set_client_ops(serdev, &dose_serdev_ops);

	ret = serdev_device_open(serdev);
	if (ret) {
		dev_err(&serdev->dev, "Unable to open device\n");
		goto free;
	}

	speed = serdev_device_set_baudrate(serdev, speed);
	dev_info(&serdev->dev, "Using baudrate: %u\n", speed);

	serdev_device_set_flow_control(serdev, false);

	ret = serdev_device_write(serdev, "Hello from Linux\n", 19, MAX_SCHEDULE_TIMEOUT);
	dev_info(&serdev->dev, "Wrote %d bytes\n", ret);

	return 0;
free:
	free_netdev(dose_dev);
	return ret;
}

static void dose_uart_remove(struct serdev_device *serdev)
{
	struct dose *dose = serdev_device_get_drvdata(serdev);

//	unregister_netdev(dose->net_dev);

	/* Flush any pending characters in the driver. */
	serdev_device_close(serdev);
	cancel_work_sync(&dose->tx_work);

	free_netdev(dose->net_dev);
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
    .remove = dose_uart_remove,
};
module_serdev_device_driver(dose_driver);

MODULE_DESCRIPTION("DOSE");
MODULE_AUTHOR("Benjamin Valentin <benjamin.valentin@ml-pa.com>");
MODULE_VERSION("0.1");
MODULE_LICENSE("GPL");
