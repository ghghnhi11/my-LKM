#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/random.h>

#define DEVICE_NAME "numGenerator"
#define CLASS_NAME "numGen"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("790and792");
MODULE_DESCRIPTION("Random number. Linux character driver");
MODULE_VERSION("1.0");

static int majorNumber; // Device major number 
static char numberGen[5];
static int count = 0;
static struct class* generClass = NULL;
static struct device* generDev = NULL;

static int device_Open(struct inode *, struct file *); // Called when device is opened
static int device_Close(struct inode *, struct file *); // Called when device is closed
static ssize_t device_Read(struct file *, char* , size_t, loff_t *); //Data send from device to user space
// Don't need to send data from device to user space
static struct file_operations fops = 
{
    .open = device_Open,
    .read = device_Read,
    .release = device_Close,
};

//LKM init function
static int __init _init_numGenerator(void)
{
    printk(KERN_INFO "Initializing the Generator Device LKM\n");
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0)
    {
        printk(KERN_ALERT "Failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "Registered correctly with major number %d\n", majorNumber);
    generClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(generClass)) // Check for errors
    {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(generClass);
    }
    printk(KERN_INFO "Device class registered correctly\n");
    generDev = device_create(generClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(generDev))
    {
        class_destroy(generClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(generDev);
    }
    printk(KERN_INFO "Device class created correctly\n");
    return 0;
}

static void __exit _exit_numGenerator(void)
{
    device_destroy(generClass, MKDEV(majorNumber, 0)); // Remove device
    class_unregister(generClass);
    class_destroy(generClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "Generator: Goodbye from the LKM!\n");
}

// inodep: pointer to an inode object (linux/fs.h)
// filep  : pointer to an file object (linux/fs.h)
static int device_Open(struct inode *inodep, struct file *filep)
{
    count++;
    printk(KERN_INFO "Generator: Device has been open %d time(s)\n", count);
    return 0;
}

static int device_Close(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "Generator: Device sucessfully closed\n");
    return 0;
}

/*
 * Function copy_to_user send numberGen that is number generated by module kernel to user 
 * and captures any errors
*/
static ssize_t device_Read(struct file *filep, char *buffer, size_t len, loff_t *offset)
{
    int errors = 0;
    //Generate number randomly
    get_random_bytes(numberGen, sizeof(int));
    errors = copy_to_user(buffer, numberGen, sizeof(int));
    if (errors == 0)
    {
        printk(KERN_INFO "Generator: Sent a number to the user\n");
        return 0;
    }
    else
    {   
        printk(KERN_INFO "Generator: Failed to sent a number to the user\n");
        return -EFAULT;
    }
}

module_init(_init_numGenerator);
module_exit(_exit_numGenerator);
