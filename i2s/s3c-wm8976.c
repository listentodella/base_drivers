
static void wm8976_write_reg(unsigned char reg, unsigned int data)
{
  int i;
  unsigned long flags;
  unsigned short val = (reg << 9) | (data & 0x1ff);

  s3c2410_gpio_setpin(S3C2410_GPB2, 1);
  s3c2410_gpio_setpin(S3C2410_GPB3, 1);
  s3c2410_gpio_setpin(S3C2410_GPB4, 1);

  local_irq_save(flags);

  for(i = 0; i < 16; i++) {
    if(val & (1 << 15)) {
      s3c2410_gpio_setpin(S3C2410_GPB4, 0);
      s3c2410_gpio_setpin(S3C2410_GPB3, 1);
      udelay(1);
      s3c2410_gpio_setpin(S3C2410_GPB4, 1);
    } else {
      s3c2410_gpio_setpin(S3C2410_GPB4, 0);
      s3c2410_gpio_setpin(S3C2410_GPB3, 0);
      udelay(1);
      s3c2410_gpio_setpin(S3C2410_GPB4, 1);
    }

    val = val << 1;
  }

  s3c2410_gpio_setpin(S3C2410_GPB2, 0);
  udelay(1);
  s3c2410_gpio_setpin(S3C2410_GPB2, 1);
  s3c2410_gpio_setpin(S3C2410_GPB3, 1);
  s3c2410_gpio_setpin(S3C2410_GPB4, 1);

  local_irq_restore(flags);
}


static void init_wm8976(void)
{

  /*根据手册操作寄存器*/
}





init_wm8976();
