#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>

#include <asm/io.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>

#define WINDOW_ADDR          CONFIG_MTD_PHYSMAP_START
#define WINDOW_SIZE          CONFIG_MTD_PHYSMAP_LEN
#define BUSWIDTH             CONFIG_MTD_PHYSMAP_BUSWIDTH


static struct mtd_info *rt2880_mtd;

static struct map_info rt2880_map = {
	.name = "RT2880 SOC Physically mapped flash",
	.bankwidth = BUSWIDTH,
	.size = WINDOW_SIZE,
	.phys = WINDOW_ADDR,
};

static struct mtd_partition rt2880_partitions[] = {
	/* Put your own partition definitions here */
        {
                name:           "Bootloader",  /* mtdblock0 */
                size:           0x30000,  /* 192K */
                offset:         0,
        }, {
                name:           "Config ", /* mtdblock1 */
                size:           0x10000,  /* 64K */
                offset:         MTDPART_OFS_APPEND,
                mask_flags:     MTD_WRITEABLE  /* force read-only */
        }, {
                name:           "Factory", /* mtdblock2 */
                size:           0x10000,  /* 64K */
                offset:         MTDPART_OFS_APPEND,
                mask_flags:     MTD_WRITEABLE  /* force read-only */
        }, {
#ifdef CONFIG_RT2880_ROOTFS_IN_FLASH
                name:           "Kernel", /* mtdblock3 */
                size:           CONFIG_MTD_KERNEL_PART_SIZ,
                offset:         MTDPART_OFS_APPEND,
        }, {
                name:           "RootFS", /* mtdblock4 */
                size:           MTDPART_SIZ_FULL,
                offset:         MTDPART_OFS_APPEND,
        }
#else //CONFIG_RT2880_ROOTFS_IN_RAM
                name:           "Kernel", /* mtdblock3 */
                size:           MTDPART_SIZ_FULL,
                offset:         MTDPART_OFS_APPEND,
        }
#endif

};


int __init rt2880_mtd_init(void)
{
	int ret = -ENXIO;

	simple_map_init(&rt2880_map);

	printk("physmap flash device: %x at %x\n", rt2880_map.phys, rt2880_map.size);
	rt2880_map.virt = ioremap(rt2880_map.phys, rt2880_map.size);

	if (!rt2880_map.virt){
		printk("ioremap fail\n"); 
		return -EIO;
	}

	rt2880_mtd = do_map_probe("cfi_probe", &rt2880_map);

	if (!rt2880_mtd){
		goto err;
	}

	rt2880_mtd->owner = THIS_MODULE;
	ret = add_mtd_partitions(rt2880_mtd, rt2880_partitions,
			ARRAY_SIZE(rt2880_partitions));

	if (ret){
		goto err;
	}

	return 0;

err:
       iounmap(rt2880_map.virt);
       return ret;
}

static void __exit rt2880_mtd_cleanup(void)
{
	if (rt2880_mtd) {
		del_mtd_partitions(rt2880_mtd);
		map_destroy(rt2880_mtd);
	}
	if (rt2880_map.virt) {
		iounmap(rt2880_map.virt);
		rt2880_map.virt = NULL;
	}
}

module_init(rt2880_mtd_init);
module_exit(rt2880_mtd_cleanup);

MODULE_AUTHOR("Steven Liu <steven_liu@ralinktech.com.tw>");
MODULE_DESCRIPTION("RT2880 Flash Map");
MODULE_LICENSE("GPL");
