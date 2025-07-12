#!/bin/bash

set -e

MODULE_NAME="sensor_driver"
DEVICE_NAME="my_sensor"
DEVICE_PATH="/dev/${DEVICE_NAME}"
LOGFILE="sensor_log.csv"

echo "[+] Building kernel module..."
make -C ../driver/

echo "[+] Inserting kernel module..."
sudo insmod driver/${MODULE_NAME}.ko || {
    echo "[-] Failed to insert module"
    exit 1
}

# Wait for /dev to create device node
sleep 1

# Check if device exists, else create manually
if [ ! -e "$DEVICE_PATH" ]; then
    echo "[!] /dev node not found, trying to create manually..."
    MAJOR=$(awk "\$2==\"${DEVICE_NAME}\" {print \$1}" /proc/devices)
    if [ -z "$MAJOR" ]; then
        echo "[-] Could not find major number for device."
        sudo rmmod $MODULE_NAME
        exit 1
    fi
    sudo mknod $DEVICE_PATH c $MAJOR 0
    sudo chmod 666 $DEVICE_PATH
fi

echo "[+] Running user-space program to read sensor..."
./user/read_sensor &

PID=$!
echo "[+] Reader running with PID $PID"
sleep 5  # Run for 5 seconds

echo "[+] Killing reader and cleaning up..."
kill $PID

echo "[+] Removing kernel module..."
sudo rmmod $MODULE_NAME

# Optional cleanup
# sudo rm -f $DEVICE_PATH

echo "[+] Done. Output stored in ${LOGFILE}"
