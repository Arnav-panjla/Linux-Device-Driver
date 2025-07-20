# Linux Device Driver Collection

```bash
cd <dirver_directory>
make
sudo insmod sensor_driver.ko
sudo dmesg | tail 
```

## Modules

### simple_LKM
Basic "Hello World" loadable kernel module demonstrating module initialization and cleanup.

### chardriver
Character device driver with read/write operations using a 256-byte buffer.

### gpiodriver
GPIO driver for Raspberry Pi LED control with on/off/blink commands via device file.

### hrtimer
High-resolution timer module measuring precise timing intervals using kernel timer mechanisms.

### interrupt
GPIO interrupt handler module demonstrating hardware interrupt processing for pin 13.

### driver
Sensor device driver template for hardware sensor integration and data access.


