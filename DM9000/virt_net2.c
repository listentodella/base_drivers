/**
 * 参考 drivers/net/cs89x0.c
 *
 * 测试：
 * 1.insmod
 * 2.ifconfig vnet0 3.3.3.3
 *  再ifconfig可以看到多了个vnet的配置
 * 3.ping 3.3.3.3 //成功-----而实际上本驱动什么都没有做（没有对硬件进行设置），说明IP地址是纯软件的操作
 *  ping 3.3.3.4 //不会死机----hard_start_xmit
 */

#include <...>


static struct net_device *vnet_dev;

static int virt_net_sendpacket(struct sk_buff *skb, struct net_device *dev)
{
  static int cnt = 0;
  printk("%s cnt = %d\n", __func__, cnt);
  /*对于真实的网卡，把skb里的数据通过网卡发送出去*/
  return 0;
}


static int virt_net_init(void)
{
  /*1. 分配一个net_device结构体*/
  vnet_dev = alloc_netdev(0, "vnet%d", ether_setup);/*alloc_etherdev*/
  /*2. 设置*/
  vnet_dev->hard_start_xmit = virt_net_sendpacket;

  /*3. 注册*/
  register_netdev(vnet_dev);
  return 0;
}
module_init(virt_net_init);

static void virt_net_exit(void)
{
  // unregister_netdevice(vnet_dev);
  unregister_netdev(vnet_dev);
  free_netdev(vnet_dev);
}
module_exit(virt_net_exit);

MODULE_LICENSE("GPL");
