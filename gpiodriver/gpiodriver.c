#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio/consumer.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arnav Panjla");
MODULE_DESCRIPTION("A GPIO driver module for raspberry pi with LED control");
MODULE_VERSION("1.0");

#define GPIO_PIN 17
#define DEVICE_NAME "gpio_led"
#define CLASS_NAME "gpio_class"

static struct gpio_desc *led_gpio;
static struct class *gpio_class = NULL;
static struct device *gpio_device = NULL;
static struct cdev gpio_cdev;
static bool led_state = false;
static struct timer_list blink_timer;
static bool blink_mode = false;

static int gpio_open(struct inode *inode, struct file *file);
static int gpio_release(struct inode *inode, struct file *file);
static ssize_t gpio_read(struct file *file, char __user *buf, size_t count, loff_t *pos);
static ssize_t gpio_write(struct file *file, const char __user *buf, size_t count, loff_t *pos);
static void blink_timer_callback(struct timer_list *timer);

static const struct file_operations gpio_fops = {
    .owner = THIS_MODULE,
    .open = gpio_open,
    .release = gpio_release,
    .read = gpio_read,
    .write = gpio_write,
};

static void blink_timer_callback(struct timer_list *timer)
{
    if (blink_mode) {
        led_state = !led_state;
        gpiod_set_value(led_gpio, led_state);
        mod_timer(&blink_timer, jiffies + msecs_to_jiffies(500)); // Blink every 500ms
        printk(KERN_INFO "GPIO LED: Blinking - state: %s\n", led_state ? "ON" : "OFF");
    }
}

static int gpio_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "GPIO LED: Device opened\n");
    return 0;
}

static int gpio_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "GPIO LED: Device closed\n");
    return 0;
}

static ssize_t gpio_read(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
    char status[32];
    int len;
    
    if (*pos > 0)
        return 0; // EOF
    
    len = snprintf(status, sizeof(status), "LED State: %s\nMode: %s\n", 
                   led_state ? "ON" : "OFF",
                   blink_mode ? "BLINK" : "MANUAL");
    
    if (len > count)
        len = count;
    
    if (copy_to_user(buf, status, len)) {
        printk(KERN_ERR "GPIO LED: Failed to copy data to user\n");
        return -EFAULT;
    }
    
    *pos += len;
    return len;
}

static ssize_t gpio_write(struct file *file, const char __user *buf, size_t count, loff_t *pos)
{
    char command[16];
    int len = min(count, sizeof(command) - 1);
    
    if (copy_from_user(command, buf, len)) {
        printk(KERN_ERR "GPIO LED: Failed to copy data from user\n");
        return -EFAULT;
    }
    
    command[len] = '\0';
    
    // Remove newline
    if (len > 0 && command[len-1] == '\n')
        command[len-1] = '\0';
    
    printk(KERN_INFO "GPIO LED: Received command: %s\n", command);
    
    if (strcmp(command, "on") == 0) {
        blink_mode = false;
        del_timer_sync(&blink_timer);
        led_state = true;
        gpiod_set_value(led_gpio, 1);
        printk(KERN_INFO "GPIO LED: Turned ON\n");
    }
    else if (strcmp(command, "off") == 0) {
        blink_mode = false;
        del_timer_sync(&blink_timer);
        led_state = false;
        gpiod_set_value(led_gpio, 0);
        printk(KERN_INFO "GPIO LED: Turned OFF\n");
    }
    else if (strcmp(command, "blink") == 0) {
        blink_mode = true;
        mod_timer(&blink_timer, jiffies + msecs_to_jiffies(500));
        printk(KERN_INFO "GPIO LED: Started blinking\n");
    }
    else if (strcmp(command, "stop") == 0) {
        blink_mode = false;
        del_timer_sync(&blink_timer);
        led_state = false;
        gpiod_set_value(led_gpio, 0);
        printk(KERN_INFO "GPIO LED: Stopped blinking\n");
    }
    else {
        printk(KERN_WARNING "GPIO LED: Unknown command: %s\n", command);
        printk(KERN_INFO "GPIO LED: Valid commands: on, off, blink, stop\n");
        return -EINVAL;
    }
    
    return count;
}

static int __init my_init(void)
{
    int ret;
    
    printk(KERN_INFO "GPIO LED: Module loading\n");
    
    /* Request GPIO */
    led_gpio = gpio_to_desc(GPIO_PIN);
    if (!led_gpio) {
        printk(KERN_ERR "GPIO LED: Failed to get GPIO %d descriptor\n", GPIO_PIN);
        return -ENODEV;
    }
    
    ret = gpiod_direction_output(led_gpio, 0);
    if (ret) {
        printk(KERN_ERR "GPIO LED: Failed to set GPIO %d as output\n", GPIO_PIN);
        return ret;
    }
    
    major_number = register_chrdev(0, DEVICE_NAME, &gpio_fops);
    if (major_number < 0) {
        printk(KERN_ALERT "GPIO LED: Failed to register a major number\n");
        return major_number;
    }
    
    gpio_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(gpio_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "GPIO LED: Failed to register device class\n");
        return PTR_ERR(gpio_class);
    }
    
    gpio_device = device_create(gpio_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(gpio_device)) {
        class_destroy(gpio_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "GPIO LED: Failed to create the device\n");
        return PTR_ERR(gpio_device);
    }
    
    cdev_init(&gpio_cdev, &gpio_fops);
    ret = cdev_add(&gpio_cdev, MKDEV(major_number, 0), 1);
    if (ret < 0) {
        device_destroy(gpio_class, MKDEV(major_number, 0));
        class_destroy(gpio_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ALERT "GPIO LED: Failed to add cdev\n");
        return ret;
    }
    
    timer_setup(&blink_timer, blink_timer_callback, 0);
    
    gpiod_set_value(led_gpio, 0);
    led_state = false;
    
    printk(KERN_INFO "GPIO LED: Module loaded successfully\n");
    printk(KERN_INFO "GPIO LED: Device created at /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "GPIO LED: Commands: echo 'on' > /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "GPIO LED: Commands: echo 'off' > /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "GPIO LED: Commands: echo 'blink' > /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "GPIO LED: Commands: echo 'stop' > /dev/%s\n", DEVICE_NAME);
    printk(KERN_INFO "GPIO LED: Status: cat /dev/%s\n", DEVICE_NAME);
    
    return 0;
}

static void __exit my_exit(void)
{
    printk(KERN_INFO "GPIO LED: Module unloading\n");
    
    blink_mode = false;
    del_timer_sync(&blink_timer);
    gpiod_set_value(led_gpio, 0);
    
    cdev_del(&gpio_cdev);
    device_destroy(gpio_class, MKDEV(major_number, 0));
    class_destroy(gpio_class);
    unregister_chrdev(major_number, DEVICE_NAME);
    
    printk(KERN_INFO "GPIO LED: Module unloaded successfully\n");
}

module_init(my_init);
module_exit(my_exit);