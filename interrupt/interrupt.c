#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arnav Panjla");
MODULE_DESCRIPTION("A kernel module for interrupt");

unsigned int irq_number;

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
	printk("gpio_irq: Interrupt was triggered and ISR was called!\n");
	return IRQ_HANDLED;
}

static int __init ModuleInit(void) {
	printk("qpio_irq: Loading module... ");

	if(gpio_request(13, "rpi-gpio-13")) {
		printk("Error!\nCan not allocate GPIO 13\n");
		return -1;
	}

	if(gpio_direction_input(13)) {
		printk("Error!\nCan not set GPIO 13 to input!\n");
		gpio_free(13);
		return -1;
	}

	irq_number = gpio_to_irq(13);

	if(request_irq(irq_number, gpio_irq_handler, IRQF_TRIGGER_RISING, "my_gpio_irq", NULL) != 0){
		printk("Error!\nCan not request interrupt nr.: %d\n", irq_number);
		gpio_free(13);
		return -1;
	}

	printk("Done!\n");
	printk("GPIO 13 is mapped to IRQ Nr.: %d\n", irq_number);
	return 0;
}

static void __exit ModuleExit(void) {
	printk("gpio_irq: Unloading module... ");
	free_irq(irq_number, NULL);
	gpio_free(13);

}

module_init(ModuleInit);
module_exit(ModuleExit);


