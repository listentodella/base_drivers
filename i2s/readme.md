cpu：i2s控制器----i2s----codec(音频解码芯片)
      GPIO   ---控制接口--



============================================
声音采集：声音模拟信号转换为数字信号，ADC
  采样频率：越高越逼真，但是也没必要太高，人耳分辨能力有限

  采集：
    ADC:左声道、右声道
    ADC精度：16bit 24bit...
    DAC：按照采集的速度播放出来

============================================
i2s：硬件接口，用来传输声音数据

CDCLK: SYS CLK
I2S LRCLK:0-左声道 1-右声道
I2S SCLK:BIT CLK
I2S SDI:接收声音数据
I2S SDO:发出声音数据

===========================================
控制接口：有多种方案
  L3接口：不同于3线接口
  i2c
  3线接口

L3接口：用到了3条线
  L3MODE：0-地址模式       |   1-数据模式
  L3CLK:每一个CLK传一位    |  每一个CLK传一位
  L3DAT:    线上是地址     |   线上是数据

  L3DAT的数据，地址模式下是8bit，高6bit代表设备地址，低2bit代表传输类型：00-控制音量 01：DATA1 读回一些信息 10：STATUS CLK 数据位宽


sound\soc\s3c24xx\s3c2410-uda1341.c
sound_core.c   

uda1341_init
  driver_register(&s3c2410iis_driver);
    ...
      probe
          使能时钟
          配置GPIO

          设置CPU的I2S控制器
          init_s3c2410_iis_bus

          使能L3接口，初始化uda1341芯片
          init_uda1341()

          设置2个DMA通道，一个用于播放，一个用于录音

          register_sound_dsp(...)
            sound_insert_unit(&chains[3], fops, dev, major, minor, "dsp", S_xxx, NULL);// /dev/dsp

          register_sound_mixer(...)
            sound_insert_unit(&chains[0], fops, dev, major, minor, "dsp", S_xxx, NULL);//  /dev/mixer


/dev/dsp: 用于播放、录音
/dev/mixer: 调整音量


声音设备看似复杂，实际也脱离不了字符设备的框架

1.主设备号
2.file_operations
3.register_chrdev

app:open() //假设主设备号为4
-------------------------------------------------------
  soundcore_open
    int unit = iminor(inode);
    s = __look_for_unit(chain, unit);
            //从chains数组里得到，谁来设置这个数组？
    new_fops = fops_get(s->unit_fops);
    file->f_op = new_fops;
    err = file->f_op->open(inode, file);


录音：
app：read
------------------------------------
  file->f_op->read

播放：
app: write
------------------------------------
  file->f_op->write


测试方法：
1.确认内核已经配置
2.make uImage并使用新内核启动
3. ls -l /dev/dsp /dev/mixer
4.播放：
  找一个wav文件，放到开发板根文件系统里
  cat xxx.wav > /dev/dsp
5.录音：
  cat /dev/dsp > sound.bin
  然后对着麦克风说话
  ctrl + c 退出
  cat sound.bin > /dev/dsp  就可以听到录下的声音



怎么写WM9076驱动程序？
1.i2s部分一样，保持不变
2.控制部分（L3）不一样，重写
