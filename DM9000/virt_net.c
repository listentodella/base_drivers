/**
 * 参考 drivers/net/cs89x0.c
 *
 * 测试：
 * 1.insmod
 * 2.ifconfig vnet0 3.3.3.3
 *  再ifconfig可以看到多了个vnet的配置
 * 3.ping 3.3.3.3 //成功-----而实际上本驱动什么都没有做（没有对硬件进行设置），说明IP地址是纯软件的操作
 *  ping 3.3.3.4 //死机-----需要注意的是，如果有多块网卡，不要分配在同一个网段
 */

#include <...>


static struct net_device *vnet_dev;

static int virt_net_init(void)
{
  /*1. 分配一个net_device结构体*/
  vnet_dev = alloc_netdev(0, "vnet%d", ether_setup);/*alloc_etherdev*/
  /*2. 设置*/

  /*3. 注册*/
  register_netdev(vnet_dev);
  return 0;
}
module_init(virt_net_init);

static void virt_net_exit(void)
{
  unregister_netdevice(vnet_dev);
  free_netdev(vnet_dev);
}
module_exit(virt_net_exit);

MODULE_LICENSE("GPL");
