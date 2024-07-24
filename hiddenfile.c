#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

MODULE_AUTHOR("MikeyQwn");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("This module is created for an example of hiding a file in "
                   "a custom device");

#define DEVICE_NAME "hiddenfile"

static int dev_open(struct inode *, struct file *);
static int dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);
static ssize_t dev_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int major;

// Module load handler
static int __init hiddenfile_init(void) {
    // Get a major for cdev
    major = register_chrdev(0, DEVICE_NAME, &fops);

    // Handle error
    if (major < 0) {
        printk(KERN_ALERT "hiddenfiles module load failed\n");
        return major;
    }

    // Print major
    printk(KERN_INFO "hiddenfiles module loaded %d\n", major);
    return 0;
}

// Module unload handler
static void __exit hiddenfile_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    printk(KERN_INFO "hiddenfiles module unloaded\n");
}

// Handler for opening the device
static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "hiddenfiles device opened\n");
    return 0;
}

// Handler for writing to the device
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len,
                         loff_t *offset) {
    char kernel_buffer[128];

    // Buffer length check
    if (len > sizeof(kernel_buffer) - 1)
        len = sizeof(kernel_buffer) - 1;

    // Copy the userspace data
    if (copy_from_user(kernel_buffer, buffer, len)) {
        printk(KERN_ERR "Failed to copy data from user space\n");
        return -EFAULT;
    }

    printk(KERN_INFO "Received from user: %s\n", kernel_buffer);

    return len;
}

// Handler for reading from the device
static ssize_t dev_read(struct file *filep, char *buffer, size_t len,
                        loff_t *offset) {
    char *message = "some info here\n";
    int msg_len = strlen(message);

    // Bounds checks
    if (len > msg_len - *offset)
        len = msg_len - *offset;

    if (len <= 0)
        return 0;

    // Copy the message to user space
    if (copy_to_user(buffer, message + *offset, len)) {
        printk(KERN_ERR "Failed to copy data to user space\n");
        return -EFAULT;
    }

    *offset += len; // Offset update
    return len;
}

// Handler for closing the device
static int dev_release(struct inode *, struct file *) {
    printk(KERN_INFO "hiddenfiles device closed\n");
    return 0;
}

// Initialize a new module
module_init(hiddenfile_init);
module_exit(hiddenfile_exit);
