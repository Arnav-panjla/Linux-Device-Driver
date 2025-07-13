#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arnav PAnjla");
MODULE_DESCRIPTION("A Dummy Sensor Driver");
MODULE_VERSION("0.1");

// constructor
static int my_module_init(void) {
    pr_info("Sensor driver loaded\n");
    return 0;
}

// destructor
static void my_module_exit(void) {
    pr_info("Sensor driver unloaded\n");
}

// macro which kernel will use to register the module
module_init(my_module_init);
module_exit(my_module_exit);
