#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <asm/io.h>
#include <asm/div64.h>

#include <asm/mach/map.h>
#include <mach/regs-lcd.h>
#include <mach/regs-gpio.h>
#include <mach/fb.h>



static struct fb_ops s3c_lcdfb_ops = {
	.owner		= THIS_MODULE,
	.fb_check_var	= atmel_lcdfb_check_var,
	.fb_set_par	= atmel_lcdfb_set_par,
	.fb_setcolreg	= atmel_lcdfb_setcolreg,
	.fb_pan_display	= atmel_lcdfb_pan_display,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};


static struct fb_info *s3c_lcd;

static int lcd_init(void)
{
	/*1.分配一个fb_info*/
	s3c_lcd = framebuffer_alloc(0, NULL);

	/*2.设置*/
		/*2.1 设置固定的参数*/
	strcpy(s3c_lcd->fix.id, "mylcd");
	s3c_lcd->fix.smem_len = 240 * 320 * 16 / 8;
	s3c_lcd->fix.type = FB_TYPE_PACKED_PIXELS;//屏类型
	s3c_lcd->fix.visual = FB_VISUAL_TRUECOLOR;//真彩色,tft屏
	s3c_lcd->fix.line_length = 240 * 2;


		/*2.2 设置可变的参数*/
	s3c_lcd->var.xres = 240;
	s3c_lcd->var.yres = 320;
	s3c_lcd->var.xres_virtual = 240;
	s3c_lcd->var.yres_virtual = 320;
	s3c_lcd->var.bits_per_pixel = 16;
	/*RGB 565*/
	s3c_lcd->var.red.offset = 11;
	s3c_lcd->var.red.length = 5;
	s3c_lcd->var.green.offset = 5;
	s3c_lcd->var.green.length = 6;	
	s3c_lcd->var.blue.offset = 0;
	s3c_lcd->var.blue.length = 5;

	s3c_lcd->var.active = FB_ACTIVE_NOW;

		/*2.3 设置操作函数*/
	s3c_lcd->fbops = &s3c_lcdfb_ops;
		/*2.4 其他设置*/
	s3c_lcd->pseudo_palette = ;//调色板
	s3c_lcd->screen_base = ;//显存的虚拟地址
	s3c_lcd->screen_size = 240 * 320 * 2;//显存大小  

	/*3.硬件相关的设置*/
		/*3.1 配置GPIO用于LCD*/
		/*3.2 根据LCD手册设置LCD控制器，比如VCLK的频率等*/
		/*3.3 分配Framebuffer（显存），并把地址告诉LCD控制器*/
	//s3c_lcd->fix.smem_start = xxx;//显存的物理地址
	

	/*4.注册*/
	register_framebuffer(s3c_lcd);
	return 0;
}
module_init(lcd_init);

static void lcd_exit(void)
{

}
module_exit(lcd_exit);

MODULE_LICENSE("GPL");