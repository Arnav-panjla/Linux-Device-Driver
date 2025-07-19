#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arnav Panjla");
MODULE_DESCRIPTION("A character file driver kernel module");

static int major_module;
static char buffer[256] = {0};

static ssize_t my_read(struct file *file, char __user *user_buffer, size_t len, loff_t *off)
{
	int not_copied, to_copy = (len < 256) ? len : 256;

	printk("my_cdev - my_Read called, *off: %lld\n", *off);

	if(*off >= to_copy) 
		return 0;

	not_copied = copy_to_user(user_buffer, buffer, to_copy);

	*off += to_copy - not_copied;

	return to_copy - not_copied;
}

static ssize_t my_write(struct file *file, const char __user *user_buffer, size_t len, loff_t *off)
{
	int not_copied, to_copy = (len < 256) ? len : 256;

	printk("my_cdev - my_write called\n");


	not_copied = copy_from_user(buffer, user_buffer, to_copy);


	return to_copy - not_copied;
}

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = my_read,
	.write = my_write
};

static int __init my_init(void) {
	major_module = register_chrdev(0, "hello-cdev", &fops);
	if (major_module < 0) {
		pr_err("hello-cdev - Some ERROR occured\n");
		return major_module;
	}
	printk("Hello, hello-cdev - Major device number: %d\n", major_module);
	return 0;
}

static void __exit my_exit(void) {
	unregister_chrdev(major_module,"hello-cdev");
	printk("Goodbye, hello-cdev\n");
}

module_init(my_init);
module_exit(my_exit);


