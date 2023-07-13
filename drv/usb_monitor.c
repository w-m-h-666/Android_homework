#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/errno.h>
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/time.h>

#define LOGI(...)   pr_info(__VA_ARGS__)
#define LOGE(...)   pr_err(__VA_ARGS__)

#define USB_INFO_SIZE 256
#define BUFFER_SIZE 512

struct usb_info_t {
    char name[USB_INFO_SIZE];
    u16 vendor_id;
    u16 product_id;
    unsigned long long insert_time;
    unsigned long long remove_time;
};

struct ring_buffer_t {
    struct usb_info_t entries[BUFFER_SIZE];
    int start;
    int count;
};

static struct ring_buffer_t ring_buffer;

// 初始化环形缓冲区
void ring_buffer_init(struct ring_buffer_t *buffer)
{
    buffer->start = 0;
    buffer->count = 0;
}

// 向环形缓冲区添加USB信息条目
void ring_buffer_add(struct ring_buffer_t *buffer, const struct usb_info_t *entry)
{
    int index = (buffer->start + buffer->count) % BUFFER_SIZE;
    memcpy(&buffer->entries[index], entry, sizeof(struct usb_info_t));

    if (buffer->count < BUFFER_SIZE)
    {
        buffer->count++;
    }
    else
    {
        buffer->start = (buffer->start + 1) % BUFFER_SIZE;
    }
}

// 显示USB信息的回调函数
static int usb_proc_show(struct seq_file *m, void *v)
{
    int i;
    int index = ring_buffer.start;

    seq_printf(m, "USB Event Log:\n");
    seq_printf(m, "------------------------------------------------------------\n");
    seq_printf(m, "Device Name     |    Insert Time     |    Remove Time\n");
    seq_printf(m, "------------------------------------------------------------\n");

    // 遍历环形缓冲区，输出USB信息条目
    for (i = 0; i < ring_buffer.count; i++)
    {
        const struct usb_info_t *entry = &ring_buffer.entries[index];
        seq_printf(m, "  %-16s | %17llu | %17llu\n", entry->name, entry->insert_time, entry->remove_time);
        index = (index + 1) % BUFFER_SIZE;
    }

    seq_printf(m, "------------------------------------------------------------\n");

    return 0;
}

// 打开/proc/usb_monitor文件时的回调函数
static int usb_proc_open(struct inode *inode, struct file *file)
{
    return single_open(file, usb_proc_show, NULL);
}

// 定义/proc/usb_monitor文件的操作结构
static const struct proc_ops usb_proc_fops = {
    .proc_open = usb_proc_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// USB设备状态改变的通知回调函数
static int usb_notify(struct notifier_block *self, unsigned long action, void *dev)
{
    struct usb_device *usb_dev = (struct usb_device *)dev;
    struct usb_info_t usb_info;

    switch (action)
    {
    case USB_DEVICE_ADD:                                            // USB设备插入
        strncpy(usb_info.name, usb_dev->product, sizeof(usb_info.name) - 1);// 获取USB设备名称
        usb_info.vendor_id = usb_dev->descriptor.idVendor;
        usb_info.product_id = usb_dev->descriptor.idProduct;
        usb_info.insert_time = ktime_get_real_seconds();
        usb_info.remove_time = 0;//ktime_get_real_seconds();        // 0表示USB设备还未拔出

        ring_buffer_add(&ring_buffer, &usb_info);                   // 添加USB信息条目

        printk(KERN_INFO "USB device inserted: Vendor ID = 0x%04X, Product ID = 0x%04X\n,%llu\n",
               usb_info.vendor_id, usb_info.product_id, usb_info.insert_time);
        break;

    case USB_DEVICE_REMOVE:                                         // USB设备拔出
        strncpy(usb_info.name, usb_dev->product, sizeof(usb_info.name) - 1);// 获取USB设备名称
        usb_info.vendor_id = usb_dev->descriptor.idVendor;
        usb_info.product_id = usb_dev->descriptor.idProduct;
        usb_info.insert_time = 0;                                   //0表示USB设备已经拔出
        usb_info.remove_time = ktime_get_real_seconds();

        ring_buffer_add(&ring_buffer, &usb_info);                   // 添加USB信息条目

        printk(KERN_INFO "USB device removed: Vendor ID = 0x%04X, Product ID = 0x%04X\n,%llu\n",
               usb_info.vendor_id, usb_info.product_id, usb_info.remove_time);
        break;
    }

    return NOTIFY_OK;
}

static struct notifier_block usb_nb = {
    .notifier_call = usb_notify,
};

// 模块初始化函数
static int __init usb_monitor_init(void)
{
    ring_buffer_init(&ring_buffer); // 初始化环形缓冲区

    usb_register_notify(&usb_nb); // 注册USB设备状态改变的通知

    proc_create("usb_monitor", 0, NULL, &usb_proc_fops); // 创建/proc/usb_monitor文件

    return 0;
}

// 模块退出函数
static void __exit usb_monitor_exit(void)
{
    usb_unregister_notify(&usb_nb); // 取消USB设备状态改变的通知

    remove_proc_entry("usb_monitor", NULL); // 删除/proc/usb_monitor文件
}

module_init(usb_monitor_init);
module_exit(usb_monitor_exit);

MODULE_LICENSE("GPL");             // 模块许可证
MODULE_AUTHOR("Minghui,Wang");     // 模块作者信息
MODULE_DESCRIPTION("USB driver");  // 模块描述信息
