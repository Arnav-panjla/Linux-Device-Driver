#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/random.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arnav Panjla");
MODULE_DESCRIPTION("A Dummy Sensor Driver with Multiple Random Readings");
MODULE_VERSION("0.3");

#define MAX_BUF_LEN 512
#define NUM_READINGS 10

static ssize_t my_proc_read(struct file *file, char __user *buf, size_t count, loff_t *offset) {
    static char message[MAX_BUF_LEN];
    static int finished = 0;
    unsigned int random_val;
    int len = 0, i;

    if (finished) {
        finished = 0;
        return 0;  // EOF
    }

    for (i = 0; i < NUM_READINGS; i++) {
        get_random_bytes(&random_val, sizeof(random_val));
        random_val = random_val % 1024; // Simulate 10-bit ADC (0–1023)
        len += snprintf(message + len, MAX_BUF_LEN - len, "Reading %d: %u\n", i + 1, random_val);
        if (len >= MAX_BUF_LEN)
            break;
    }

    if (copy_to_user(buf, message, len)) {
        return -EFAULT;
    }

    *offset += len;
    finished = 1;

    printk(KERN_INFO "Generated %d dummy sensor readings\n", NUM_READINGS);

    return len;
}

static struct proc_dir_entry *proc_entry;
static struct proc_ops my_proc_ops = {
    .proc_read = my_proc_read,
};

static int __init my_module_init(void) {
    printk(KERN_INFO "Dummy Sensor Driver Loaded\n");

    proc_entry = proc_create("my_dummy_sensor", 0, NULL, &my_proc_ops);
    if (proc_entry == NULL) {
        printk(KERN_ERR "Failed to create /proc/my_dummy_sensor\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "Proc entry created at /proc/my_dummy_sensor\n");
    return 0;
}

static void __exit my_module_exit(void) {
    proc_remove(proc_entry);
    printk(KERN_INFO "Dummy Sensor Driver Unloaded\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
