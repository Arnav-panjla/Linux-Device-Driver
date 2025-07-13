#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arnav PAnjla");
MODULE_DESCRIPTION("A Dummy Sensor Driver");
MODULE_VERSION("0.1");

static struct proc_dir_entry *proc_entry;
static struct proc_ops my_proc_ops = {
    .proc_read = NULL, 
    .proc_write = NULL,
};


// constructor
static int my_module_init(void) {
    printk("Sensor driver loaded\n");

    printk("initializing proc entry\n");
    proc_entry = proc_create("my_dummy_sensor", 0, NULL, &my_proc_ops);
    if (proc_entry == NULL) {
        printk(KERN_ERR "Failed to create proc entry\n");
        return -1l
    }
    printk("Proc entry created at /proc/my_dummy_sensor\n");

    return 0;
}

// destructor
static void my_module_exit(void) {
    printk("Sensor driver unloaded\n");
    proc_remove(proc_entry);
    printk("Proc entry removed\n");
}

// macro which kernel will use to register the module
module_init(my_module_init);
module_exit(my_module_exit);
