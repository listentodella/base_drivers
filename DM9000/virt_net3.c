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

static int emulator_rx_packet(struct sk_buff *skb, struct net_device *dev)
{
  /*参考LDD3*/
  unsigned char *type;
  char *data;
  int len;
  struct iphdat

}

static int virt_net_sendpacket(struct sk_buff *skb, struct net_device *dev)
{
  static int cnt = 0;
  printk("%s cnt = %d\n", __func__, cnt);
  /*对于真实的网卡，把skb里的数据通过网卡发送出去*/

  /*构造一个假的sk_buff, 上报*/
  emulator_rx_packet(skb, dev);

  netif_stop_queue(dev);  /*停止该网卡的队列*/
  /* ..... */             /*把skb的数据写入网卡*/
  dev_kfree_skb(skb);     /*释放skb*/
  netif_wake_queue(dev);  /*数据全部发送出去后，唤醒网卡的队列*/

  /*更新统计信息*/
  dev->stats.tx_packets++;
  dev->stats.tx_bytes += skb->len;

  return 0;
}


static int virt_net_init(void)
{
  /*1. 分配一个net_device结构体*/
  vnet_dev = alloc_netdev(0, "vnet%d", ether_setup);/*alloc_etherdev*/
  /*2. 设置*/
  vnet_dev->hard_start_xmit = virt_net_sendpacket;

  /*设置MAC地址*/
  vnet_dev->dev_addr[0] = 0x08;
  vnet_dev->dev_addr[1] = 0x89;
  vnet_dev->dev_addr[2] = 0x89;
  vnet_dev->dev_addr[3] = 0x89;
  vnet_dev->dev_addr[4] = 0x89;
  vnet_dev->dev_addr[5] = 0x11;

  /*设置下面两项才能ping通*/
  vnet_dev->flags |= IFF_NOARP;
  vnet_dev->features |= NETIF_F_NO_CSUM;

  /*3. 注册*/
  register_netdev(vnet_dev);
  return 0;
}
module_init(virt_net_init);

static void virt_net_exit(void)
{
  unregister_netdev(vnet_dev);
  free_netdev(vnet_dev);
}
module_exit(virt_net_exit);

MODULE_LICENSE("GPL");
