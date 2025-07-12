// driver/sensor_driver.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#define DEVICE_NAME "my_sensor"
#define CLASS_NAME  "sensor_class"

#define SENSOR_INTERVAL_MS 100
#define SENSOR_BUFFER_SIZE 64

static int major;
static struct class* sensor_class = NULL;
static struct device* sensor_device = NULL;
static struct cdev sensor_cdev;

static char sensor_data[SENSOR_BUFFER_SIZE];
static int data_ready = 0;
static struct timer_list sensor_timer;
static spinlock_t lock;

// Simulate new data from sensor every 100ms
static void sensor_timer_callback(struct timer_list *t)
{
    unsigned long flags;
    spin_lock_irqsave(&lock, flags);
    snprintf(sensor_data, SENSOR_BUFFER_SIZE, "SensorValue: %d\n", jiffies % 1000);
    data_ready = 1;
    spin_unlock_irqrestore(&lock, flags);

    mod_timer(&sensor_timer, jiffies + msecs_to_jiffies(SENSOR_INTERVAL_MS));
}

static ssize_t sensor_read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    ssize_t ret;
    unsigned long flags;

    spin_lock_irqsave(&lock, flags);

    if (!data_ready) {
        spin_unlock_irqrestore(&lock, flags);
        return 0; // No new data yet
    }

    if (len > strlen(sensor_data))
        len = strlen(sensor_data);

    if (copy_to_user(buffer, sensor_data, len)) {
        spin_unlock_irqrestore(&lock, flags);
        return -EFAULT;
    }

    data_ready = 0;
    spin_unlock_irqrestore(&lock, flags);
    return len;
}

static int sensor_open(struct inode *inodep, struct file *filep)
{
    return 0;
}

static int sensor_release(struct inode *inodep, struct file *filep)
{
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = sensor_read,
    .open = sensor_open,
    .release = sensor_release,
};

static int __init sensor_init(void)
{
    dev_t dev;

    spin_lock_init(&lock);

    // Allocate major number
    if (alloc_chrdev_region(&dev, 0, 1, DEVICE_NAME) < 0) {
        printk(KERN_ALERT "Sensor: Failed to allocate major number\n");
        return -1;
    }

    major = MAJOR(dev);
    cdev_init(&sensor_cdev, &fops);
    if (cdev_add(&sensor_cdev, dev, 1) < 0) {
        unregister_chrdev_region(dev, 1);
        printk(KERN_ALERT "Sensor: Failed to add cdev\n");
        return -1;
    }

    sensor_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(sensor_class)) {
        cdev_del(&sensor_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(sensor_class);
    }

    sensor_device = device_create(sensor_class, NULL, dev, NULL, DEVICE_NAME);
    if (IS_ERR(sensor_device)) {
        class_destroy(sensor_class);
        cdev_del(&sensor_cdev);
        unregister_chrdev_region(dev, 1);
        return PTR_ERR(sensor_device);
    }

    // Init timer
    timer_setup(&sensor_timer, sensor_timer_callback, 0);
    mod_timer(&sensor_timer, jiffies + msecs_to_jiffies(SENSOR_INTERVAL_MS));

    printk(KERN_INFO "Sensor: Initialized successfully\n");
    return 0;
}

static void __exit sensor_exit(void)
{
    dev_t dev = MKDEV(major, 0);

    del_timer_sync(&sensor_timer);
    device_destroy(sensor_class, dev);
    class_destroy(sensor_class);
    cdev_del(&sensor_cdev);
    unregister_chrdev_region(dev, 1);

    printk(KERN_INFO "Sensor: Unloaded successfully\n");
}

module_init(sensor_init);
module_exit(sensor_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arnav Panjla");
MODULE_DESCRIPTION("Character Device Driver for Simulated Sensor");
