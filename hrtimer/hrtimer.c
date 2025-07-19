#include <linux/module.h>
#include <linux/init.h>
#include <linux/hrtimer.h>
#include <linux/ktime.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Arnav Panjla");
MODULE_DESCRIPTION("A simple high resolution timer");

static int major_module;

static struct hrtimer my_hrtimer;

static ktime_t start_t;


static enum hrtimer_restart test_hrtimer_handler(struct hrtimer *timer) {
	u64 now_t = ktime_get();
	printk("start_t - now_t = %lld\n", ktime_to_ns(ktime_sub(now_t, start_t)));
	return HRTIMER_NORESTART;
}


static int __init my_init(void) {

	hrtimer_init(&my_hrtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	my_hrtimer.function = &test_hrtimer_handler;
	start_t = ktime_get();
	hrtimer_start(&my_hrtimer, ms_to_ktime(100), HRTIMER_MODE_REL);
	return 0;
}

static void __exit my_exit(void) {
	printk("Goodbye, Kernel\n");
	hrtimer_cancel(&my_hrtimer);

}

module_init(my_init);
module_exit(my_exit);


