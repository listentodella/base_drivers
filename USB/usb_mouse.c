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


static struct usb_device_id usbmouse_as_key_id_table[] = {
  	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT, USB_INTERFACE_PROTOCOL_MOUSE) },
    //{USB_DEVICE(XXX, XXX)},可以作为只支持某个产品
      { }						/* Terminating entry */
};


static int usbmouse_as_key_probe(struct usb_interface * intf, const struct usb_device_id * id)
{
  printk("found usbmouse...\n");
  return 0;
}

static void usb_mouse_disconnect(struct usb_interface *intf)
{
  printk("disconnect usbmouse...\n");
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
