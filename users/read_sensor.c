#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#define DEVICE_PATH "/dev/my_sensor"
#define LOG_FILE "sensor_log.csv"
#define BUF_SIZE 64

// Get monotonic timestamp in microseconds
long long current_timestamp_us() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long long)ts.tv_sec * 1000000LL + ts.tv_nsec / 1000;
}

int main() {
    int fd = open(DEVICE_PATH, O_RDONLY);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    FILE *log = fopen(LOG_FILE, "w");
    if (!log) {
        perror("Failed to open log file");
        close(fd);
        return 1;
    }

    fprintf(log, "Timestamp (us),Sensor Data\n");

    char buf[BUF_SIZE];
    ssize_t bytes_read;

    printf("Reading sensor data. Press Ctrl+C to stop...\n");

    while (1) {
        long long ts = current_timestamp_us();
        memset(buf, 0, BUF_SIZE);

        bytes_read = read(fd, buf, BUF_SIZE - 1);
        if (bytes_read < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                usleep(10000); // wait 10ms and retry
                continue;
            }
            perror("Read failed");
            break;
        }

        if (bytes_read > 0) {
            buf[bytes_read] = '\0';
            printf("[%lld us] %s", ts, buf);
            fprintf(log, "%lld,%s", ts, buf);
        }

        usleep(100000); // Sleep for 100ms to match driver update rate
    }

    fclose(log);
    close(fd);
    return 0;
}
