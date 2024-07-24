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
#define SECRET_KEY "4mu7xa3r0wmsl97xrgfpk5ycyprwlezc"
#define KEY_LENGTH (sizeof(SECRET_KEY) - 1)
#define DATA_LENGTH 128

static char hidden_data[DATA_LENGTH];

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
    char kernel_buffer[DATA_LENGTH + KEY_LENGTH];

    if (len < KEY_LENGTH) {
        printk(KERN_INFO "Invalid key length written\n");
        return -EINVAL;
    }

    // Cap the length to the length of the kernel buffer
    if (len > sizeof(kernel_buffer)) {
        len = sizeof(kernel_buffer);
    }

    if (copy_from_user(kernel_buffer, buffer, len)) {
        printk(KERN_INFO "Failed to copy data from user space\n");
        return -EFAULT;
    }

    if (strncmp(kernel_buffer, SECRET_KEY, KEY_LENGTH) != 0) {
        printk(KERN_INFO "Invalid key supplied\n");
        return -EINVAL;
    }

    size_t rest_length = len - KEY_LENGTH;

    // Buffer length sanity check
    if (rest_length >= DATA_LENGTH) {
        rest_length = DATA_LENGTH - 1;
    }

    memcpy(hidden_data, kernel_buffer + KEY_LENGTH, rest_length);

    hidden_data[rest_length] = '\0';

    printk(KERN_INFO "Received from user: %s\n", hidden_data);

    return len;
}

// Handler for reading from the device
static ssize_t dev_read(struct file *filep, char *buffer, size_t len,
                        loff_t *offset) {
    if (len < KEY_LENGTH) {
        printk(KERN_INFO "Invalid key length written\n");
        return -EINVAL;
    }

    char key_buffer[KEY_LENGTH];
    if (copy_from_user(key_buffer, buffer, KEY_LENGTH)) {
        printk(KERN_INFO "Failed to copy data from user space\n");
        return -EFAULT;
    }

    if (strncmp(key_buffer, SECRET_KEY, KEY_LENGTH) != 0) {
        printk(KERN_INFO "Invalid key supplied\n");
        return -EINVAL;
    }

    size_t hiddenfile_length = strlen(hidden_data);
    len = min(len, hiddenfile_length);

    // Copy the message to user space
    if (copy_to_user(buffer, hidden_data, len)) {
        printk(KERN_ERR "Failed to copy data to user space\n");
        return -EFAULT;
    }

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
