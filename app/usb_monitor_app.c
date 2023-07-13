#include <stdio.h>
#include <string.h>

#define USB_INFO_SIZE 256
#define BUFFER_SIZE 512

struct usb_info_t {
    char name[USB_INFO_SIZE];
    unsigned long long insert_time;
    unsigned long long remove_time;
};

struct ring_buffer_t {
    struct usb_info_t entries[BUFFER_SIZE];
    int start;
    int count;
};

struct ring_buffer_t ring_buffer;

void ring_buffer_init(struct ring_buffer_t *buffer) {
    buffer->start = 0;
    buffer->count = 0;
}

void ring_buffer_add(struct ring_buffer_t *buffer, const struct usb_info_t *entry) {
    int index = (buffer->start + buffer->count) % BUFFER_SIZE;
    memcpy(&buffer->entries[index], entry, sizeof(struct usb_info_t));

    if (buffer->count < BUFFER_SIZE) {
        buffer->count++;
    } else {
        buffer->start = (buffer->start + 1) % BUFFER_SIZE;
    }
}

void read_usb_monitor_data() {
    FILE *fp;
    char line[USB_INFO_SIZE * 3];
    struct usb_info_t usb_info;

    fp = fopen("/proc/usb_monitor", "r");
    if (fp == NULL) {
        printf("Failed to open /proc/usb_monitor\n");
        return;
    }

    // Skip the header lines
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    // Read USB device information
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "  %255[^|] | %17llu | %17llu", usb_info.name, &usb_info.insert_time, &usb_info.remove_time) == 3) {
            ring_buffer_add(&ring_buffer, &usb_info);
        }
    }

    fclose(fp);
}

void print_usb_monitor_data() {
    int i;
    int index = ring_buffer.start;

    printf("USB Event Log:\n");
    printf("------------------------------------------------------------\n");
    printf("Device Name     |    Insert Time     |    Remove Time\n");
    printf("------------------------------------------------------------\n");

    for (i = 0; i < ring_buffer.count; i++) 
    {
        const struct usb_info_t *entry = &ring_buffer.entries[index];
        printf("  %-16s | %17llu | %17llu\n", entry->name, entry->insert_time, entry->remove_time);
        index = (index + 1) % BUFFER_SIZE;
    }

    printf("------------------------------------------------------------\n");
}

int main() {
    ring_buffer_init(&ring_buffer);
    read_usb_monitor_data();
    print_usb_monitor_data();

    return 0;
}
