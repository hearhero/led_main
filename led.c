#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/io.h>

#include "led.h"

int led_major = 249;
int led_minor = 0;

struct class *led_class;
struct cdev cdev;

unsigned long *GPFCON = NULL;
unsigned long *GPFDAT = NULL;

static DEFINE_SPINLOCK(led_lock);

int led_count = 0;

static int led_open(struct inode *inode, struct file *filp)
{
	spin_lock(&led_lock);

	if (0 < led_count)
	{
		spin_unlock(&led_lock);
		return -EBUSY;
	}

	led_count++;
	spin_unlock(&led_lock);

	*GPFCON &= ~(0xff<<8);
	*GPFCON |= 0x55<<8;

	return 0;
}

static int led_release(struct inode *inode, struct file *filp)
{
	spin_lock(&led_lock);
	led_count--;
	spin_unlock(&led_lock);

	*GPFDAT |= 0xf<<4;

	return 0;
}

static int led_ioctl(struct inode *inode, struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;

	switch (cmd) 
	{
	case LED_ON:
		switch (arg) 
		{
		case 0:
			*GPFDAT &= ~(0xf<<4);
			break;
		case 1:
			*GPFDAT &= ~(0xf<<4);
			*GPFDAT |= 0xe<<4;
			break;
		case 2:
			*GPFDAT &= ~(0xf<<4);
			*GPFDAT |= 0xd<<4;
			break;
		case 3:
			*GPFDAT &= ~(0xf<<4);
			*GPFDAT |= 0xb<<4;
			break;
		case 4:
			*GPFDAT &= ~(0xf<<4);
			*GPFDAT |= 0x7<<4;
			break;
		default:
			ret = -EINVAL;
			break;
		}
		break;
	case LED_OFF:
		switch (arg)
		{
		case 0:
			*GPFDAT |= 0xf<<4;
		default:
			ret = -EINVAL;
			break;
		}
		break;
	default:
		ret = -EINVAL;
		break;
	}

	return ret;
}

struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.open = led_open,
	.release = led_release,
	.ioctl = led_ioctl,
};

static void char_reg_setup_cdev(void)
{
	int error;
	dev_t dev;

	dev = MKDEV(led_major, led_minor);

	cdev_init(&cdev, &led_fops);
	cdev.owner = THIS_MODULE;
	cdev.ops = &led_fops;
	error = cdev_add(&cdev, dev, 1);

	if (0 > error)
	{
		printk(KERN_NOTICE "Error %d adding char_reg_setup_cdev\n", error);
		return;
	}

	led_class = class_create(THIS_MODULE, "led_class");

	if (IS_ERR(led_class))
	{
		printk("Failed to create led_class\n");
		return;
	}

	device_create(led_class, NULL, dev, NULL, "LED");
}

static int __init led_init(void)
{
	int ret = 0;
	dev_t dev;

	dev = MKDEV(led_major, led_minor);

	ret = register_chrdev_region(dev, 1, "LED");

	if (0 > ret)
	{
		printk(KERN_WARNING "led: cat't get the major number %d\n", led_major);
		return ret;
	}

	char_reg_setup_cdev();

	GPFCON = (unsigned long *)ioremap(0x56000050, 4);
	GPFDAT = (unsigned long *)ioremap(0x56000054, 4);

	return ret;
}

static void __exit led_exit(void)
{
	dev_t dev;

	dev = MKDEV(led_major, led_minor);

	device_destroy(led_class, dev);
	class_destroy(led_class);

	cdev_del(&cdev);
	unregister_chrdev_region(dev, 1); 

	iounmap(GPFCON);
	iounmap(GPFDAT);
}

module_init(led_init);
module_exit(led_exit);

MODULE_LICENSE("GPL");
