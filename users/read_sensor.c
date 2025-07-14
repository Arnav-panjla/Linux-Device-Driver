#include <stdio.h>
#include <stdlib.h>

#define PROC_FILE "/proc/my_dummy_sensor"

int main() {
    FILE *fp;
    char buffer[512];

    fp = fopen(PROC_FILE, "r");
    if (fp == NULL) {
        perror("Failed to open proc file");
        return EXIT_FAILURE;
    }

    printf("Reading from dummy sensor:\n\n");

    while (fgets(buffer, sizeof(buffer), fp) != NULL) {
        printf("%s", buffer);
    }

    fclose(fp);
    return EXIT_SUCCESS;
}
