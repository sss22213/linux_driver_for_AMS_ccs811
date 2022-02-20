#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/ioctl.h>
#include <linux/i2c.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include "ccs811_sensor.h"

#define CCS811_DEV_NAME "ccs811"
#define	IOCTL_WRITE	_IOW('c', 1, struct _ccs811_ioctl)
#define CCS811_UNUSED(x) if(x){}

static struct class *ccs811_class = NULL;

struct mutex ccs811_clients_lock;

static dev_t dev = 0;

struct _ccs811_device ccs811_device;

// For sensor control
typedef enum {
    _CCS811_SET_MODE = 1,
    _CCS811_SET_TEMPERATURE,
    _CCS811_SET_HUMIDITY,
} _CCS811_SET;


struct _ccs811_ioctl {
    uint8_t ccs811_cmd;
    uint8_t low_val;
    uint8_t high_val;
};

// ccs811 character device struct
struct ccs811_i2c_cdev {
    struct i2c_client *client;
    struct cdev cdev;
};

static long int ccs811_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
    struct _ccs811_ioctl ccs811_ioctl;

    switch (ioctl_num) {
    case IOCTL_WRITE:
        CCS811_UNUSED(copy_from_user(&ccs811_ioctl, (int __user*)ioctl_param, sizeof(struct _ccs811_ioctl)));
        switch (ccs811_ioctl.ccs811_cmd) {
        case _CCS811_SET_MODE:
            ccs881_set_measure_mode(&ccs811_device, ccs811_ioctl.low_val);
            break;
        
        case _CCS811_SET_TEMPERATURE:
            ccs881_set_temperature(&ccs811_device, ccs811_ioctl.high_val, ccs811_ioctl.low_val);
            break;
        
        case _CCS811_SET_HUMIDITY:
            ccs881_set_humidity(&ccs811_device, ccs811_ioctl.high_val, ccs811_ioctl.low_val);
            break;

        default:
            break;
        }
        break;
    }

    return 0;
}

static int ccs811_i2c_open(struct inode *inode, struct file *filp)
{
    struct ccs811_i2c_cdev *ccs811 = container_of(inode->i_cdev, struct ccs811_i2c_cdev, cdev);

    pr_info("Open device.\n");

    filp->private_data = ccs811->client;

    ccs811_startup(&ccs811_device);

    return 0;
}

static ssize_t ccs811_i2c_read(struct file *filp, char __user *buf, size_t count, loff_t *offset)
{

    u8 buff[8] = {0};

    struct i2c_client *client = filp->private_data;
    if (!client) {
    	pr_err("Failed to get struct i2c_client.\n");
	    return -EINVAL;
    }
 
    // Mutex for prevent mutiuser
    mutex_lock(&ccs811_clients_lock);

    // Read temperature from sensor
    ccs811_get_tovc(&ccs811_device, buff);

    CCS811_UNUSED(copy_to_user(buf, &buff, 8));

    count = 8;

    mutex_unlock(&ccs811_clients_lock);

    return 8;
}

struct file_operations ccs811_i2c_fops = {
    .open = ccs811_i2c_open,
    .read = ccs811_i2c_read,
    .unlocked_ioctl = ccs811_ioctl,
};


static int ccs811_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    struct ccs811_i2c_cdev *ccs811 = NULL;

    struct device *device = NULL;

    pr_info("Probe device of ccs811\n");

    /* Alloc space for cdev */
    alloc_chrdev_region(&dev, 0, 1, CCS811_DEV_NAME);

    /* Create class for ccs811 */
    ccs811_class = class_create(THIS_MODULE, CCS811_DEV_NAME);

    ccs811 = kzalloc(sizeof(struct ccs811_i2c_cdev), GFP_KERNEL);

    ccs811->client = client;

    /* Initialize character */
    cdev_init(&(ccs811->cdev), &ccs811_i2c_fops);

    ccs811->cdev.owner = THIS_MODULE;

    /* Add character to system */
    cdev_add(&(ccs811->cdev), dev, 1);

    /* Create device */
    device = device_create(ccs811_class, NULL, dev, NULL, CCS811_DEV_NAME);

    i2c_set_clientdata(client, ccs811);

    /* Initialize mutex */
    mutex_init(&ccs811_clients_lock);

    ccs811_init(&ccs811_device, client);

    return 0;
  
}

static int ccs811_remove(struct i2c_client *client)
{
    pr_info("Remove device of ccs811.\n");

    device_destroy(ccs811_class, dev);

    class_destroy(ccs811_class);

    unregister_chrdev_region(dev, 1);

    return 0;
}

static struct of_device_id ccs811_id_tables[] = {
    { .compatible="ccs811", },
    { }
};

MODULE_DEVICE_TABLE(of, ccs811_id_tables);

static struct i2c_driver ccs811_drv = {
    .probe = ccs811_probe,
    .remove = ccs811_remove,
    .driver = {
    	.name = "ccs811 device 0.1",
	.owner = THIS_MODULE,
	.of_match_table = ccs811_id_tables,
    },
};

MODULE_AUTHOR("Bohung Nian <n0404.n0404 at="" gmail.com="">");
module_i2c_driver(ccs811_drv);
MODULE_LICENSE("GPL");
