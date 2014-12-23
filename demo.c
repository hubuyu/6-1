#include <linux/module.h> 
#include <linux/init.h>   
#include <linux/fs.h>   
#include <linux/i2c.h> 
#include <asm/uaccess.h>    
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/delay.h>

#define DEMO_MAJOR		256	//need to be check 
#define DEMO_ADDR		0x27

#define COMMAND1		1
#define COMMAND2		3

#define BUF_SIZE		256

struct demo_data {
	dev_t					devt ;
	struct i2c_client		*i2c_device ;
	struct list_head		device_entry ;
	unsigned char			addr ;
	unsigned char			buffer[BUF_SIZE];
	char					test ;
};

static const struct i2c_device_id demo_ids[] = {
	{"demo" , 8 },
	{}
};
MODULE_DEVICE_TABLE(i2c , demo_ids);

static LIST_HEAD( device_list );
static struct class *demo_class ;

static int demo_open(struct inode *inode , struct file *filp )
{	
	struct demo_data *demo ;
	int status = -ENXIO ;

	printk("Drv info : %s()\n", __FUNCTION__);

	list_for_each_entry(demo , &device_list , device_entry ){
		if(demo->devt == inode->i_rdev){
			status = 0 ;
			printk("demo test char = %c \n", demo->test);
			break ;
		}
	}

	if (status != 0){
		printk("get the demo structure failured ");
		return status ;	
	}
		
	filp->private_data = demo ;
	demo->addr = DEMO_ADDR ;
#if 0
	//initialize the device
#endif
	return status ;
}

static int demo_release(struct inode *inode , struct file *filp )
{
	struct demo_data *demo ;
	demo = filp->private_data ;
	printk("Drv info : %s()\n", __FUNCTION__);
	if ( demo == NULL )
		return -ENODEV ;
	filp->private_data = NULL ;
	return 0 ;
}

static long demo_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int status=1 ;
	struct demo_data *demo ;
	demo = filp->private_data ;

	switch (cmd){
		case	COMMAND1 :
			printk("Case - Command 1\n" );
			break;
		case	COMMAND2 :
			printk("Case - Command 2\n" );
			break;
		default :
			printk("%s() cmd-%d is not supprt at this moment\n" , __FUNCTION__ , cmd);
			break;
	}
	
	return status ;
}

static ssize_t demo_write(struct file *filp ,const char __user *buf , size_t count , loff_t *f_ops )
{
	int err ;
	struct demo_data *demo ;
	demo = filp->private_data ;
	printk("Drv info : %s()\n", __FUNCTION__);
	err = copy_from_user(demo->buffer+*f_ops , buf ,count );
	if(err )
		return -EFAULT ;
	
	return 0 ;
}

static ssize_t demo_read( struct file *filp , char __user *buf , size_t count , loff_t *f_ops )
{
	int err ;
	struct demo_data *demo ;
	demo = filp->private_data ;
	printk("Drv info : %s()\n", __FUNCTION__);
	err = copy_to_user(buf, demo->buffer+*f_ops , count );
	if (err)
		return -EFAULT ;
	return err ;
}

static const struct file_operations demo_ops = {
	.open			= demo_open ,
	.release		= demo_release ,
	.write			= demo_write ,
	.read			= demo_read ,
	.unlocked_ioctl = demo_ioctl ,
};

static int demo_probe (struct i2c_client *i2cdev , const struct i2c_device_id *id)
{
	struct demo_data *demo = NULL;
	unsigned long minor ;

	printk("Drv info : %s()\n", __FUNCTION__);
	demo = kzalloc(sizeof(*demo), GFP_KERNEL);
	if(!demo)
		return -ENOMEM ;

	demo->i2c_device = i2cdev ;
	INIT_LIST_HEAD(&demo->device_entry);

	minor = 0 ;
	demo->devt = MKDEV(DEMO_MAJOR , minor);
	device_create(demo_class , &i2cdev->dev,demo->devt , 
				  demo , "hubuyu");
	printk("device number is %d \n", demo->devt);

	demo->test = 'X';
	list_add( &demo->device_entry , &device_list );
	i2c_set_clientdata( i2cdev , demo);
	printk("Drv info : %s()\n", __FUNCTION__);
	return 0 ;
}

static int demo_remove (struct i2c_client *i2cdev)
{
	struct demo_data *demo ;
	demo = i2c_get_clientdata(i2cdev);
	demo->i2c_device = NULL ;
	i2c_set_clientdata(i2cdev , NULL);
	device_destroy(demo_class , demo->devt);
	kfree(demo);
	return 0 ;
}

static struct i2c_driver demo_driver = {
	.driver = {
		.name = "demo",
		.owner = THIS_MODULE ,	
	},
	.probe = demo_probe ,
	.remove = demo_remove ,
	.id_table = demo_ids ,
};

static int __init demo_init(void)
{	int status ;
	status = register_chrdev(DEMO_MAJOR , "demo" , &demo_ops);
	if (status < 0 )
		return status ;

	demo_class = class_create(THIS_MODULE , "demo-class");
	if (IS_ERR(demo_class)){
		unregister_chrdev(DEMO_MAJOR , demo_driver.driver.name);
	}

	printk("hello , Kernel-%s()\n", __FUNCTION__);
	return i2c_add_driver(&demo_driver);
	return 0 ;
}

static void __exit demo_exit(void)
{
	printk("Goodbye , Kernel-%s()\n", __FUNCTION__);
	i2c_del_driver(&demo_driver);
	class_destroy(demo_class);
	unregister_chrdev(DEMO_MAJOR , demo_driver.driver.name);
}

module_init(demo_init);
module_exit(demo_exit);

MODULE_AUTHOR("hubuyu");
MODULE_LICENSE("GPL");
MODULE_ALIAS("i2c:drver_demo");
MODULE_DESCRIPTION("This is I2c device driver architecture");
