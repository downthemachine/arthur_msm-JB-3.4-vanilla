#include <linux/i2c.h>
#include <linux/i2c/sx150x.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <asm/mach-types.h>
#include <mach/msm_iomap.h>
#include <mach/board.h>
//#include <mach/irqs-7xxx.h>
//#include "devices-msm7x2xa.h"
//#include "board-msm7627a.h"
#include <mach/vreg.h>
//#include "board-msm7x30-regulator.h"


#ifdef CONFIG_TOUCHSCREEN_MXT224
#include <linux/input/atmel_qt602240.h>
#endif
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS
#include <linux/input/synaptics_rmi.h> 
#endif
#ifdef CONFIG_TOUCHSCREEN_FOCALTECH
#include <linux/input/focaltech_ft5x0x.h>
#endif




//
//  touchscreen gpio definition
//
//#if defined (CONFIG_MACH_WARP2)
//#define GPIO_8X55_TS_EN
#define VREG_VDD_3V				"gp6"
#define VREG_VBUS				"s3"
#define GPIO_7X30_TS_IRQ		55
#define GPIO_7X30_TS_RST		41

//
//	touchscreen firmware file name
//
#if defined (CONFIG_MACH_WARP2)
#define FTC_FW_NAME ""
#define SYN_FW_NAME "N970_N861_PR1183396_s2202_32313037.img"
#define ATM_FW_NAME ""
#else
#define FTC_FW_NAME ""
#define SYN_FW_NAME ""
#define ATM_FW_NAME ""
#endif



//
//	touchscreen virtual key definition
//

#ifdef CONFIG_TOUCHSCREEN_VIRTUAL_KEYS
//#define CAP_TS_VKEY_CYPRESS "virtualkeys.cyttsp-i2c"
#define CAP_TS_VKEY_SYNAPTICS "virtualkeys.syna-touchscreen"
#define CAP_TS_VKEY_ATMEL "virtualkeys.atmel-touchscreen"
#define CAP_TS_VKEY_FTS "virtualkeys.Fts-touchscreen"

// board warp2
#if defined (CONFIG_MACH_WARP2)
#define SYNAPTICS_MAX_Y_POSITION	1885
static ssize_t cap_ts_vkeys_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
	//printk("%s, %s\n",__func__,attr->attr.name);
	return sprintf(
		buf,__stringify(EV_KEY) ":" __stringify(KEY_BACK) ":80:1020:150:80"
		":" __stringify(EV_KEY) ":" __stringify(KEY_HOME) ":270:1020:150:80"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":460:1020:150:80"
		"\n");	
}

// board arthur
#elif defined (CONFIG_MACH_ARTHUR)
#define SYNAPTICS_MAX_Y_POSITION	1885
static ssize_t cap_ts_vkeys_show(struct device *dev,	struct device_attribute *attr, char *buf)
{
	//printk("%s, %s\n",__func__,attr->attr.name);
	return sprintf(
		buf,__stringify(EV_KEY) ":" __stringify(KEY_HOME) ":60:850:100:50"
		":" __stringify(EV_KEY) ":" __stringify(KEY_MENU) ":180:850:100:50"
		":" __stringify(EV_KEY) ":" __stringify(KEY_BACK) ":300:850:100:50"
		":" __stringify(EV_KEY) ":" __stringify(KEY_SEARCH) ":420:850:100:50"
		"\n");	
}
#endif

static struct device_attribute cap_ts_device_attr[] = {
#if defined(CONFIG_TOUCHSCREEN_FOCALTECH)
	{
		.attr = {
			.name = CAP_TS_VKEY_FTS,
			.mode = S_IRUGO,
		},
		.show	= &cap_ts_vkeys_show,
		.store	= NULL,
	},
#endif
#if defined(CONFIG_TOUCHSCREEN_SYNAPTICS)
{
	.attr = {
		.name = CAP_TS_VKEY_SYNAPTICS,
		.mode = S_IRUGO,
	},
	.show	= &cap_ts_vkeys_show,
	.store	= NULL,
},
#endif
#if defined(CONFIG_TOUCHSCREEN_MXT224)
	{
		.attr = {
			.name = CAP_TS_VKEY_ATMEL,
			.mode = S_IRUGO,
		},
		.show	= &cap_ts_vkeys_show,
		.store	= NULL,
	},
#endif
};

struct kobject *android_touch_kobj;
static int cap_ts_vkeys_init(void)
{
	int rc,i;
	struct kobject * cap_ts_properties_kobj=NULL;

	cap_ts_properties_kobj = kobject_create_and_add("board_properties", NULL);
	if (cap_ts_properties_kobj == NULL) {
		printk("%s: subsystem_register failed\n", __func__);
		rc = -ENOMEM;
		return rc;
	}
	android_touch_kobj = cap_ts_properties_kobj;

	for ( i=0; i < ARRAY_SIZE(cap_ts_device_attr); i++ ){
		rc = sysfs_create_file(cap_ts_properties_kobj, &cap_ts_device_attr[i].attr);
		if (rc) {
			printk("%s: sysfs_create_file failed\n", __func__);
			return rc;
		}
	}

	return 0;
}
#else

#define SYNAPTICS_MAX_Y_POSITION	0

#endif

//
// i2c device definition
//

#if defined (CONFIG_TOUCHSCREEN_SYNAPTICS)
static struct synaptics_rmi_data synaptics_ts_data = {
	.gpio_init = touchscreen_gpio_init,
	.power	= touchscreen_power,
	.reset	= touchscreen_reset,
	.irq	= touchscreen_irq,
	.max_y_position = SYNAPTICS_MAX_Y_POSITION,	// 0 - no vkey, do nothing
	.fwfile = SYN_FW_NAME,
};
#endif

#if defined (CONFIG_TOUCHSCREEN_FOCALTECH)
static struct focaltech_ts_platform_data focaltech_ts_data = {
	.gpio_init = touchscreen_gpio_init,
	.power	= touchscreen_power,
	.reset	= touchscreen_reset,
	.irq	= touchscreen_irq,
	.fwfile	 = FTC_FW_NAME,
};
#endif

#if defined (CONFIG_TOUCHSCREEN_MXT224)
static struct atmel_platform_data atmel_ts_data = {
	.gpio_init = touchscreen_gpio_init,
	.power	= touchscreen_power,
	.reset	= touchscreen_reset,
	.irq	= touchscreen_irq,
	.fwfile	 = ATM_FW_NAME,
};
#endif


static struct i2c_board_info i2c_touch_devices[] = {
#ifdef CONFIG_TOUCHSCREEN_FOCALTECH	
	{				
		I2C_BOARD_INFO("ft5x0x_ts", 0x3E ),
		.irq = MSM_GPIO_TO_INT(GPIO_7X30_TS_IRQ),
		.platform_data = &focaltech_ts_data,
	},	
#endif
#ifdef CONFIG_TOUCHSCREEN_SYNAPTICS
	{
		I2C_BOARD_INFO("syna-touchscreen", 0x22 ),
		.irq = MSM_GPIO_TO_INT(GPIO_7X30_TS_IRQ),
		.platform_data = &synaptics_ts_data,
	},
#endif
#ifdef CONFIG_TOUCHSCREEN_MXT224
	{    
		I2C_BOARD_INFO("atmel_qt602240", 0x4a ),
		.platform_data = &atmel_ts_data,
		.irq = MSM_GPIO_TO_INT(GPIO_7X30_TS_IRQ),
	},   
#endif
};


void __init msm7x30_ts_init(void)
{

#ifdef CONFIG_TOUCHSCREEN_VIRTUAL_KEYS
	cap_ts_vkeys_init();
#endif

	i2c_register_board_info(0, i2c_touch_devices,ARRAY_SIZE(i2c_touch_devices));

	return ;
}

