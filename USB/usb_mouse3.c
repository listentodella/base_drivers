/*目的：将USB鼠标用作按键
左键当作L
右键当作S
滚轮按下当作回车
=====>>>>相当于输入子系统
1.分配input_dev
2.设置
3.注册
4.硬件操作------USB读写函数

内核里有一个usbmouse.c
*/

/*该宏包含在头文件中
#define USB_INTERFACE_INFO(cl,sc,pr) \
	.match_flags = USB_DEVICE_ID_MATCH_INT_INFO, .bInterfaceClass = (cl), .bInterfaceSubClass = (sc), .bInterfaceProtocol = (pr)
*/

/**
 * USB_DEVICE - macro used to describe a specific usb device
 * @vend: the 16 bit USB Vendor ID
 * @prod: the 16 bit USB Product ID
 *
 * This macro is used to create a struct usb_device_id that matches a
 * specific device.

#define USB_DEVICE(vend,prod) \
	.match_flags = USB_DEVICE_ID_MATCH_DEVICE, .idVendor = (vend), .idProduct = (prod)
*/

static struct input_dev *uk_dev;
static char *usb_buf;
static dma_addr_t *usb_buf_phy;
static int len;
static struct urb *uk_urb;


static struct usb_device_id usbmouse_as_key_id_table[] = {
  	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT, USB_INTERFACE_PROTOCOL_MOUSE) },
    //{USB_DEVICE(XXX, XXX)},可以作为只支持某个产品
      { }						/* Terminating entry */
};

/*值得注意的是，USB设备本身并不会产生中断，CPU也不会一直轮询USB设备
 *轮询USB设备的是USB主机（控制器），产生中断的也是它
 *可以先测试这个驱动，观察打印出来的数据信息，进而判断数据含义
 */
static void usbmouse_as_key_irq(struct urb *urb, struct pt_regs *regs)
{
  int i;
  static int cnt = 0;
  printk("data cnt %d: ", ++cnt);
  for(i = 0; i < len; i++)
    printk("%02x ",usb_buf[i]);
  printk("\n");

  /*重新提交urb*/
  usb_submit_urb(uk_urb, GFP_KERNEL);

}

	input_regs(dev, regs);

	input_report_key(dev, BTN_LEFT,   data[0] & 0x01);
	input_report_key(dev, BTN_RIGHT,  data[0] & 0x02);
	input_report_key(dev, BTN_MIDDLE, data[0] & 0x04);
	input_report_key(dev, BTN_SIDE,   data[0] & 0x08);
	input_report_key(dev, BTN_EXTRA,  data[0] & 0x10);

	input_report_rel(dev, REL_X,     data[1]);
	input_report_rel(dev, REL_Y,     data[2]);
	input_report_rel(dev, REL_WHEEL, data[3]);

	input_sync(dev);
resubmit:
	status = usb_submit_urb (urb, SLAB_ATOMIC);
	if (status)
		err ("can't resubmit intr, %s-%s/input0, status %d",
				mouse->usbdev->bus->bus_name,
				mouse->usbdev->devpath, status);
}


/*
 *	struct usb_host_interface struct usb_endpoint_descriptor
 *	这两个结构体成员可以看看，值得注意的是端口是除了端口0之外的，因为0是每个USB设备都有的
 */
static int usbmouse_as_key_probe(struct usb_interface * intf, const struct usb_device_id * id)
{
  struct usb_device *dev = interface_to_usbdev(intf);
  struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
  int pipe;
  interface = intf->cur_altsetting;
  endpoint = &interface->endpoint[0].desc;//这里是指除了端点0之外的第一个端点

	struct usb_mouse *mouse;
	int pipe;
	char path[64];

  /*a.分配一个input_dev*/
  uk_dev = input_allocate_device();

  /*b.设置*/
    /*b.1 能产生哪类事件*/
  set_bit(EV_KEY, uk_dev->evbit);
  set_bit(EV_REP, uk_dev->evbit);
    /*b.2 能产生哪些事件*/
  set_bit(KEY_L, uk_dev->keybit);
  set_bit(KEY_S, uk_dev->keybit);
  set_bit(KEY_ENTER, uk_dev->keybit);

  /*c.注册*/
  input_register_device(uk_dev);

  /*d.硬件相关操作*/
  /*数据传输3要素:源、目的、长度*/
  //源：USB设备的某个端点
  pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
  //长度
  len = endpoint->MaxPacketSize;
  //目的
  usb_buf = usb_buffer_alloc(dev, len, GFP_ATOMIC, &usb_buf_phy);

  /*使用3要素*/
  //分配usb request block
  uk_urb = usb_alloc_urb(0, GFP_KERNEL);
  //使用 3要素 设置urb
  usb_fill_int_urb(uk_urb, dev, pipe, usb_buf,
      len, usbmouse_as_key_irq, NULL, endpoint->bInterval);
  usb_buf->irq->transfer_dma = usb_buf_phy;
  usb_buf->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

  /*使用urb*/
  usb_submit_urb(uk_urb, GFP_KERNEL);

  return 0;
}

static void usb_mouse_disconnect(struct usb_interface *intf)
{
  struct usb_device *dev = interface_to_usbdev(intf);
  printk("disconnect usbmouse...\n");
  usb_kill_urb(uk_urb);
  urb_free_urb(uk_urb);

  usb_buffer_free(struct usb_device *dev, len, usb_buf, usb_buf_phy);
  input_unregister_device(uk_dev);
  input_free_device(uk_dev);
}

/*1. 分配/设置 usb_driver*/
static struct usb_driver usbmouse_as_key_driver = {
	.owner		= THIS_MODULE,
	.name		= "usbmouse_as_key",
	.probe		= usbmouse_as_key_probe,
	.disconnect	= usbmouse_as_key_disconnect,
	.id_table	= usbmouse_as_key_id_table,
};

static int usbmouse_as_key_init(void)
{
  /*2. 注册*/
  usb_register(&usbmouse_as_key_driver);
  return 0;
}

static void usbmouse_as_key_exit(void)
{
  usb_deregister(&usbmouse_as_key_driver);
}

module_init(usbmouse_as_key_init);
module_exit(usbmouse_as_key_exit);
MODULE_LICENSE("GPL");
