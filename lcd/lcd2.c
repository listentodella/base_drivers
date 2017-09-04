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

/*lcd控制器，对照数据手册，注意成员与成员之间的字节差，要与手册上的一致 */
struct lcd_regs {
	unsigned long lcdcon1;
	unsigned long lcdcon2;
	unsigned long lcdcon3;
	unsigned long lcdcon4;
	unsigned long lcdcon5;
	unsigned long lcdaddr1;
	unsigned long lcdaddr2;
	unsigned long lcdaddr3;
	unsigned long redlut;
	unsigned long greenlut;
	unsigned long bluelut;
	unsigned long reserved[9];
	unsigned long dithmode;
	unsigned long tpal;
	unsigned long lcdintpnd;
	unsigned long lcdsrcpnd;
	unsigned long lcdintmsk;
	unsigned long lpcsel;
};


static struct fb_ops s3c_lcdfb_ops = {
	.owner		= THIS_MODULE,
	.fb_check_var	= atmel_lcdfb_check_var,
	.fb_set_par	= atmel_lcdfb_set_par,
	.fb_setcolreg	= s3c_lcdfb_setcolreg,
	.fb_pan_display	= atmel_lcdfb_pan_display,
	.fb_fillrect	= cfb_fillrect,
	.fb_copyarea	= cfb_copyarea,
	.fb_imageblit	= cfb_imageblit,
};


static struct fb_info *s3c_lcd;
static volatile unsigned long *gpbcon;
static volatile unsigned long *gpbdat;
static volatile unsigned long *gpccon;
static volatile unsigned long *gpdcon;
static volatile unsigned long *gpgcon;
static volatile struct lcd_regs *lcd_regs;
static u32 pseudo_palette[16];/*调色板*/



/* from pxafb.c */
static inline unsigned int chan_to_field(unsigned int chan,
					 struct fb_bitfield *bf)
{
	chan &= 0xffff;
	chan >>= 16 - bf->length;
	return chan << bf->offset;
}

static int s3c_lcdfb_setcolreg(unsigned int regno, unsigned int red,
			     unsigned int green, unsigned int blue,
			     unsigned int transp, struct fb_info *info)
{
	unsigned int val;
	if (regno > 16)
		return 1;

	/*用 red green blue三原色构造出val*/
	val  = chan_to_field(red, &info->var.red);
	val |= chan_to_field(green, &info->var.green);
	val |= chan_to_field(blue, &info->var.blue);
	pseudo_palette[regno] = val;
	//or ((u32 *)(info->pseudo_palette))[regno] = val;这样更通用一点

	return 0;

}


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
	s3c_lcd->pseudo_palette = pseudo_palette;//调色板
	//s3c_lcd->screen_base = ;//显存的虚拟地址
	s3c_lcd->screen_size = 240 * 320 * 2;//显存大小

	/*3.硬件相关的设置*/
		/*3.1 配置GPIO用于LCD*/
	gpbcon = ioremap(0x56000010, 8);
	gpbdat = gpbcon + 1;
	gpccon = ioremap(0x56000020, 4);
	gpdcon = ioremap(0x56000030, 4);
	gpgcon = ioremap(0x56000060, 4);

	*gpccon = 0xaaaaaaa;/*GPIO管脚，用于VD[7:0],LCDVF[2:0],VM,VFRAME,VLINE,VCLK,LEND*/
	*gpdcon = 0xaaaaaaa;/*GPIO管脚，用于VD[23:8]*/

	*gpbcon &= ~(3);/*GPB0设为输出引脚*/
	*gpbcon |= 1;
	*gpbdat &= ~1;/*输出低电平*/

	*gpgcon |= (3 << 8);/*GPG4用作LCD_PWREN*/


		/*3.2 根据LCD手册设置LCD控制器，比如VCLK的频率等*/
	lcd_regs = ioremap(0x4d000000, sizeof(struct lcd_regs));

		/*lcdcon1
		 * bit[17:8]:VCLK = HCLK / [(CLKVAL + 1) * 2]
		 *     lcd手册要求至少100ns,即 10MHz = 100MHz / [(CLKVAL + 1) * 2]
		 *     则 CLKVAL = 4
		 * bit[6:5] : 0b11 TFT屏
		 * bit[4:1] : 0b1100, 16 bpp for TFT
		 * bit[0] : 0,disable the video output and the LCD control signal
		 */
	lcd_regs->lcdcon1 = (4 << 8) | (3 << 5) | (0x0c << 1);

		/*lcdcon2
		 * 垂直方向的时间参数
		 * 		bit[31:24]:VBPD,VSYNC之后再过多长时间才能发出第一行数据？
		 * 					LCD手册 T0-T2-T1=4
		 * 					所以VBPD = 3
		 *      bit[23:14]:多少行？320行，所以LINEVAL=320-1=319
		 *      bit[13:6]:VFPD,发出最后一行数据之后，再过多长时间才发出VSYNC信号？
		 *      			LCD手册 T2-T5=322-320=2,所以VFPD=2-1=1
		 *      bit[5:0]:VSPW,VSYNC信号的脉冲宽度，LCD手册T1=1,所以VSPW=1-1=0
		 */
	lcd_regs->lcdcon2 = (3 << 24) | (319 << 14) | (1 << 6) | (0 <<0);

		/*lcdcon3
		 * 水平方向的时间参数
		 * 		bit[25:19]:HBPD,HSYNC之后再过多长时间才能发出第一个像素的数据？
		 * 					LCD手册 T6-T7-T8=17
		 * 					所以HBPD = 16
		 *      bit[18:8]:多少列？240，所以HOZVAL=240-1=239
		 *      bit[7:0]:HFPD,发出最后一行里最后一个像素数据之后，再过多长时间才发出HSYNC信号？
		 *      			LCD手册 T8-T11=251-240=11,所以VFPD=11-1=10
		 */
	lcd_regs->lcdcon3 = (16 << 19) | (239 << 8) | (10 << 0);

		/*lcdcon4
		 *水平方向的同步信号
  		 *bit[7:0]:	HSPW,HSYNC信号的脉冲宽度，LCD手册T7=5,所以HSPW=5-1=4
		 */
	lcd_regs->lcdcon4 = 4;

		/*lcdcon5
		 * 信号的极性
		 * bit[11]: 1  565 format
		 * bit[10]: 0  the  video data is fetched at VCLK falling edge
		 * bit[9]:  1  HSYNC信号要反转，即低电平有效
		 * bit[8]:  1  VSYNC信号要反转，即低电平有效
		 * bit[6]:  0  VDEN信号不需要反转
		 * bit[3]:  0  PWREN输出低电平
		 * bit[1]:  0  BSWP
		 * bit[0]:  1  HWSWP 2440手册
		 */
	lcd_regs->lcdcon5 = (1 << 11) | (0 << 10) | (1 << 9) | (1 << 8) | (1 << 0);


		/*3.3 分配Framebuffer（显存），并把地址告诉LCD控制器*/
 	s3c_lcd->screen_base = dma_alloc_writecombine(NULL, s3c_lcd->fix.smem_len,
 											&s3c_lcd->fix.smem_start, GFP_KERNEL);
 	lcd_regs->lcdaddr1 = (s3c_lcd->fix.smem_start << 1) & ~(3 << 30);
 	lcd_regs->lcdaddr2 = ((s3c_lcd->fix.smem_start + s3c_lcd->fix.smem_len) >> 1) & 0x1fffff;
 	lcd_regs->lcdaddr3 = (240 * 16 / 16);/*一行的长度（单位：半字，2字节）*/

	//s3c_lcd->fix.smem_start = xxx;//显存的物理地址

	/*启动LCD*/
 	lcd_regs->lcdcon1 |= (1 << 0);/*使能lcd 控制器*/
 	lcd_regs->lcdcon5 |= (1 << 3);/*使能lcd本身（lcd的电源，或者说上电）*/
 	*gpbdat |= 1;/*输出高电平，使能背光*/


	/*4.注册*/
	register_framebuffer(s3c_lcd);
	return 0;
}
module_init(lcd_init);

static void lcd_exit(void)
{
	unregister_framebuffer(s3c_lcd);
	/*关闭LCD*/
 	lcd_regs->lcdcon1 &= ~(1 << 0);/*关闭lcd本身*/
 	*gpbdat &= ~1;/*关闭背光*/
 	dma_free_writecombine(NULL, s3c_lcd->fix.smem_len,
 							s3c_lcd->fix.screen_base, s3c_lcd->fix.smem_start);
 	iounmap(lcd_regs);
 	iounmap(gpbcon);
 	iounmap(gpccon);
 	iounmap(gpdcon);
 	iounmap(gpgcon);
 	framebuffer_release(s3c_lcd);
}
module_exit(lcd_exit);

MODULE_LICENSE("GPL");
