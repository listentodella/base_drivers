新内核2.6之后字符设备驱动的写法

2.4及之前的，习惯使用register_chrdev(...)到chrdevs中找到之前注册的file_operations，
但是这种方法，以主设备号为下标，（主，0）~（主，255）都被霸占了，内核只能有255个字符设备驱动。

2.6及之后的，则将其展开，以主设备号和次设备号，来找到file_operations
①register_chrdev_region/alloc_chrdev_region,这里的region为（主，次）~（主，次+n）
②cdev_init
③cdev_add
