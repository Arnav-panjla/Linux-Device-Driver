# Modular Linux Character Device Driver for Real-Time Sensor Input

This project implements a Linux kernel module that simulates real-time sensor data using a character device interface. It demonstrates system-level development with emphasis on kernel timing, device communication, and user-space interaction.

---

## 🧠 Overview

A periodic sensor signal is generated in the kernel using a high-resolution timer, exposed to user space via a custom character device. A user-space C program reads the sensor values and logs them with microsecond-precision timestamps to evaluate read latency.

---

## 🛠️ Features

- Linux character device (`/dev/my_sensor`) for sensor input
- Periodic kernel-side value updates every 100ms
- Spinlock-based concurrency protection for shared data
- User-space program logs timestamped sensor data to CSV
- Shell automation script for build, insert, test, and cleanup
- GitHub Actions CI for module compilation verification

---

## 🚀 Quick Start

### 1. Build and Insert the Driver

```bash
cd driver
make
sudo insmod sensor_driver.ko
```

### 2. Run the User Logger
```bash
Copy
Edit
cd ../user
gcc read_sensor.c -o read_sensor
./read_sensor
```