#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/init.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/hrtimer.h>
#include <linux/sysfs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>         /* for kmalloc() */
#include <linux/workqueue.h>


typedef struct
{
    unsigned int profile_type;
    unsigned int color_Value;
    unsigned int interval;    
}profile_t;


extern unsigned long simple_strtoul(const char *,char **,unsigned int);

static const unsigned char rb_table[] = {
        0x00, 0x80, 0x40, 0xc0, 0x20, 0xa0, 0x60, 0xe0,
        0x10, 0x90, 0x50, 0xd0, 0x30, 0xb0, 0x70, 0xf0,
        0x08, 0x88, 0x48, 0xc8, 0x28, 0xa8, 0x68, 0xe8,
        0x18, 0x98, 0x58, 0xd8, 0x38, 0xb8, 0x78, 0xf8,
        0x04, 0x84, 0x44, 0xc4, 0x24, 0xa4, 0x64, 0xe4,
        0x14, 0x94, 0x54, 0xd4, 0x34, 0xb4, 0x74, 0xf4,
        0x0c, 0x8c, 0x4c, 0xcc, 0x2c, 0xac, 0x6c, 0xec,
        0x1c, 0x9c, 0x5c, 0xdc, 0x3c, 0xbc, 0x7c, 0xfc,
        0x02, 0x82, 0x42, 0xc2, 0x22, 0xa2, 0x62, 0xe2,
        0x12, 0x92, 0x52, 0xd2, 0x32, 0xb2, 0x72, 0xf2,
        0x0a, 0x8a, 0x4a, 0xca, 0x2a, 0xaa, 0x6a, 0xea,
        0x1a, 0x9a, 0x5a, 0xda, 0x3a, 0xba, 0x7a, 0xfa,
        0x06, 0x86, 0x46, 0xc6, 0x26, 0xa6, 0x66, 0xe6,
        0x16, 0x96, 0x56, 0xd6, 0x36, 0xb6, 0x76, 0xf6,
        0x0e, 0x8e, 0x4e, 0xce, 0x2e, 0xae, 0x6e, 0xee,
        0x1e, 0x9e, 0x5e, 0xde, 0x3e, 0xbe, 0x7e, 0xfe,
        0x01, 0x81, 0x41, 0xc1, 0x21, 0xa1, 0x61, 0xe1,
        0x11, 0x91, 0x51, 0xd1, 0x31, 0xb1, 0x71, 0xf1,
        0x09, 0x89, 0x49, 0xc9, 0x29, 0xa9, 0x69, 0xe9,
        0x19, 0x99, 0x59, 0xd9, 0x39, 0xb9, 0x79, 0xf9,
        0x05, 0x85, 0x45, 0xc5, 0x25, 0xa5, 0x65, 0xe5,
        0x15, 0x95, 0x55, 0xd5, 0x35, 0xb5, 0x75, 0xf5,
        0x0d, 0x8d, 0x4d, 0xcd, 0x2d, 0xad, 0x6d, 0xed,
        0x1d, 0x9d, 0x5d, 0xdd, 0x3d, 0xbd, 0x7d, 0xfd,
        0x03, 0x83, 0x43, 0xc3, 0x23, 0xa3, 0x63, 0xe3,
        0x13, 0x93, 0x53, 0xd3, 0x33, 0xb3, 0x73, 0xf3,
        0x0b, 0x8b, 0x4b, 0xcb, 0x2b, 0xab, 0x6b, 0xeb,
        0x1b, 0x9b, 0x5b, 0xdb, 0x3b, 0xbb, 0x7b, 0xfb,
        0x07, 0x87, 0x47, 0xc7, 0x27, 0xa7, 0x67, 0xe7,
        0x17, 0x97, 0x57, 0xd7, 0x37, 0xb7, 0x77, 0xf7,
        0x0f, 0x8f, 0x4f, 0xcf, 0x2f, 0xaf, 0x6f, 0xef,
        0x1f, 0x9f, 0x5f, 0xdf, 0x3f, 0xbf, 0x7f, 0xff,
};

#define T0L 180  //240//580-1600 ns
#define T0H 45   //120//220-380 ns
#define T1L 45   //120//220-420 ns
#define T1H 180  //240//580-1600 ns


static struct platform_device *led_device;

#define DEMO_NAME "ws2812c_dev"
static struct device *ws2812c_device;

/*virtual FIFO device's buffer*/
static char *device_buffer;

#define MAX_DEVICE_BUFFER_SIZE 64

unsigned char bit_count = 24;

static void nano_delay(unsigned long delay_count){
  mb();
  for(; delay_count--; ){
    //mb();
    asm("nop");
  }
  mb();
}


static void sendRst(void){

  udelay(300);
}

static void sendbit0(void){

  nano_delay(T0H);
  nano_delay(T0L);
}


static void sendbit1(void){

  nano_delay(T1H);
  nano_delay(T1L);
}

static void led_light(unsigned char *color){

  unsigned int count = bit_count;
  unsigned int color_rgb; // B->R->G->
  
  color_rgb =  rb_table[color[2]]<<16|rb_table[color[1]]<<8|rb_table[color[0]]; // B->R->G->
  printk(KERN_INFO"\n[B:R:G]->%08x\n", color_rgb);

  while(count--){
    if(color_rgb&0x1){
      sendbit1();
    }else{
      sendbit0();
    }
    color_rgb>>=1;
  }

}

static ssize_t show_coloring(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t len)
{
	return 1;
}

static ssize_t store_coloring(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t len)
{
  unsigned long val = 0;

  val = simple_strtoul(buf, NULL, 16);

  unsigned char color_cg[3] = {0x0,0x0,0x0};

  color_cg[0] = (unsigned char)((val&0xff0000)>>16);
  color_cg[1] = (unsigned char)((val&0xff00)>>8);
  color_cg[2] = (unsigned char)((val&0xff)>>0);

  sendRst();

  led_light(color_cg);

  return printk("[G%02x:R02%x:B%02x]\n",  color_cg[0], color_cg[1], color_cg[2] );
}

static DEVICE_ATTR(led_coloring, S_IRUGO | S_IWUSR , show_coloring, store_coloring);

static struct attribute *led_sysfs_entries[] = {
  &dev_attr_led_coloring.attr,
  NULL
};

static struct attribute_group led_attr_group = {
	.name = "ws2812-led-ctl",
	.attrs = led_sysfs_entries,
};



static int ws2812c_open(struct inode *inode, struct file *file)
{
	int major = MAJOR(inode->i_rdev);
	int minor = MINOR(inode->i_rdev);

	printk("%s: major=%d, minor=%d\n", __func__, major, minor);

	return 0;
}

static int ws2812c_close(struct inode *inode, struct file *file)
{
  return 0;
}

static ssize_t ws2812c_read(struct file *file, char __user *buf, size_t lbuf, loff_t *ppos)
{
  int actual_readed;

  actual_readed = 0;

  return actual_readed;
}

static ssize_t ws2812c_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	int actual_write;
	int free;
	int need_write;
	int ret;

	printk("%s enter\n", __func__);

	free = MAX_DEVICE_BUFFER_SIZE - *ppos;
	need_write = free > count ? count : free;
	if (need_write == 0) printk( "no space for write");

		
	ret = copy_from_user(device_buffer + *ppos, buf, need_write);
	if (ret == need_write) return -EFAULT;

	actual_write = need_write - ret;
	*ppos += actual_write;
	printk("%s: actual_write =%d, ppos=%d\n", __func__, actual_write, *ppos);

	return actual_write;
}

static ssize_t ws2812c_ioctl(struct file *file, unsigned int profile_type, unsigned long arg)
{
	profile_t lighting;

    if( copy_from_user( &lighting , (profile_t *)arg, sizeof(profile_t) ) ){
      return -EFAULT;
    }


    printk(" %d %x %x %x\n",profile_type, lighting.profile_type,lighting.color_Value,lighting.interval);

	return 1;
}


static const struct file_operations ws2812c_fops = {
	.owner          = THIS_MODULE,
	.open           = ws2812c_open,
	.read           = ws2812c_read,
	.write          = ws2812c_write,
	.release        = ws2812c_close,
    .unlocked_ioctl = ws2812c_ioctl,
};

static struct miscdevice ws2812c_misc_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEMO_NAME,
	.fops = &ws2812c_fops,
};



static int __init led_char_init(void)
{

  int ret;	

  device_buffer = kmalloc(MAX_DEVICE_BUFFER_SIZE, GFP_KERNEL);

  if (!device_buffer) return -ENOMEM;

  ret = misc_register(&ws2812c_misc_device);

  if (ret) {
    printk("failed register misc device\n");
    kfree(device_buffer);
    return ret;
  }

  ws2812c_device = ws2812c_misc_device.this_device;

  printk("succeeded register char device: %s\n", DEMO_NAME);

  led_device = platform_device_register_simple("ws2812c_led", -1, NULL, 0);

  if (IS_ERR(led_device)) {
    printk("platfrom device register fail\n");
    ret = PTR_ERR(led_device);
    goto proc_fail;
  }

  ret = sysfs_create_group(&led_device->dev.kobj, &led_attr_group);

  if (ret) {
    printk("create sysfs group fail\n");
    goto register_fail;
  }

  pr_info("create sysfs node done\n");

  return 0;
	
register_fail:
  platform_device_unregister(led_device);

proc_fail:
  return ret;
}

static void __exit led_char_exit(void)
{

  sysfs_remove_group(&led_device->dev.kobj, &led_attr_group);

  platform_device_unregister(led_device);

  kfree(device_buffer);

  misc_deregister(&ws2812c_misc_device);

  pr_info("removing device\n");	
}

module_init(led_char_init);
module_exit(led_char_exit);

MODULE_AUTHOR("led_ws2812");
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("led character device");
