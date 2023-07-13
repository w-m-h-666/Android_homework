#include <stdio.h>
#include <string.h>

#define USB_INFO_SIZE 256
#define BUFFER_SIZE 512

// USB设备信息结构体
struct usb_info_t {
    char name[USB_INFO_SIZE];                // 设备名称
    unsigned long long insert_time;          // 插入时间
    unsigned long long remove_time;          // 移除时间
};

// 环形缓冲区结构体
struct ring_buffer_t {
    struct usb_info_t entries[BUFFER_SIZE];  // 存储USB设备信息的数组
    int start;                              // 缓冲区的起始索引
    int count;                              // 缓冲区中的元素数量
};

struct ring_buffer_t ring_buffer;           // 声明环形缓冲区对象

// 初始化环形缓冲区
void ring_buffer_init(struct ring_buffer_t *buffer) {
    buffer->start = 0;                      // 起始索引为0
    buffer->count = 0;                      // 元素数量为0
}

// 向环形缓冲区添加USB设备信息
void ring_buffer_add(struct ring_buffer_t *buffer, const struct usb_info_t *entry) {
    int index = (buffer->start + buffer->count) % BUFFER_SIZE;   // 计算要插入的索引位置
    memcpy(&buffer->entries[index], entry, sizeof(struct usb_info_t));   // 将USB设备信息复制到缓冲区

    if (buffer->count < BUFFER_SIZE) {
        buffer->count++;                    // 元素数量增加
    } else {
        buffer->start = (buffer->start + 1) % BUFFER_SIZE;   // 缓冲区已满，更新起始索引
    }
}

// 读取USB监视器数据
void read_usb_monitor_data() {
    FILE *fp;
    char line[USB_INFO_SIZE * 3];
    struct usb_info_t usb_info;

    fp = fopen("/proc/usb_monitor", "r");    // 打开USB监视器文件
    if (fp == NULL) {
        printf("Failed to open /proc/usb_monitor\n");
        return;
    }

    // 跳过头部行
    fgets(line, sizeof(line), fp);
    fgets(line, sizeof(line), fp);

    // 读取USB设备信息
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (sscanf(line, "  %255[^|] | %17llu | %17llu", usb_info.name, &usb_info.insert_time, &usb_info.remove_time) == 3) {
            ring_buffer_add(&ring_buffer, &usb_info);    // 将USB设备信息添加到环形缓冲区
        }
    }

    fclose(fp);                               // 关闭文件
}

// 打印USB监视器数据
void print_usb_monitor_data() {
    int i;
    int index = ring_buffer.start;

    printf("USB Event Log:\n");
    printf("------------------------------------------------------------\n");
    printf("Device Name     |    Insert Time     |    Remove Time\n");
    printf("------------------------------------------------------------\n");

    for (i = 0; i < ring_buffer.count; i++) {
        const struct usb_info_t *entry = &ring_buffer.entries[index];
        printf("  %-16s | %17llu | %17llu\n", entry->name, entry->insert_time, entry->remove_time);
        index = (index + 1) % BUFFER_SIZE;     // 更新索引以遍历下一个元素
    }

    printf("------------------------------------------------------------\n");
}

int main() {
    ring_buffer_init(&ring_buffer);           // 初始化环形缓冲区
    read_usb_monitor_data();                   // 读取USB监视器数据
    print_usb_monitor_data();                  // 打印USB监视器数据

    return 0;
}
