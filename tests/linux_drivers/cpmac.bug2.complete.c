#include "langinc.h"

conditional_t cond_irq_can_happen;
;
;
static int dumb_switch;
u32 cpmac_mii_phy_mask;
struct cpmac_desc {
    u32 hw_next;
    u32 hw_data;
    u16 buflen;
    u16 bufflags;
    u16 datalen;
    u16 dataflags;
    struct sk_buff *skb;
    struct cpmac_desc *next;
    struct cpmac_desc *prev;
    dma_addr_t mapping;
    dma_addr_t data_mapping;
};
lock_t cplock;
lock_t rx_lock;
struct cpmac_desc *rx_head;
int ring_size;
struct cpmac_desc *desc_ring;
dma_addr_t dma_ring;
void *regs;
struct phy_device *phy;
char phy_name[20];
int oldlink;
int oldspeed;
int oldduplex;
u32 msg_enable;
struct net_device dev;
struct work_struct reset_work;
atomic_t reset_pending;
;
u32 cpmac_int_vector;
u32 cpmac_rx_ptr;
struct plat_cpmac_data {
    int reset_bit;
    int power_bit;
    u32 phy_mask;
    char dev_addr[6];
};
struct plat_cpmac_data pdata;
static int mii_irqs[32] = { -1 };
static struct mii_bus cpmac_mii;
static int external_switch;
conditional_t cond_platform_driver_not_in_use;
lock_t l1;

int ar7_gpio_disable(unsigned int gpio) {
    ioval = nondet_int;
    return nondet_int;
}


void ar7_device_reset(u32 bit) {
    ioval = nondet_int;
}


void cpmac_write(u32 reg, u32 val) {
    *(int *)regs = nondet_int;
    ioval = nondet_int;
}


u32 cpmac_read(u32 reg) {
    return ioval + (*(int *)regs);
}


u32 cpmac_read_CPMAC_MAC_INT_VECTOR() {
    return cpmac_int_vector;
}


void cpmac_write_CPMAC_RX_PTR(u32 i, u32 val) {
    cpmac_rx_ptr = val;
}


u32 cpmac_read_CPMAC_RX_PTR(u32 i) {
    return cpmac_rx_ptr;
}


static void cpmac_hw_start()
static void cpmac_hw_stop()
static int cpmac_mdio_reset() {
    ar7_device_reset(22);
    cpmac_write(4, 1073741824 | ((nondet_int / 2200000 - 1) & 255));
    return 0;
}


static int cpmac_start_xmit(struct sk_buff *skb) {
    int queue, len;
    if (({
        skb = (struct sk_buff *)(addr_t)nondet_int;
        nondet_int;
    }))
        return NETDEV_TX_OK;
    len = ({
        typeof (skb->len) _x = (skb->len);
        typeof (60) _y = (60);
        _x > _y ? _x : _y;
    });
    netif_stop_subqueue();
    if (desc_ring[queue].dataflags & 8192) {
        return NETDEV_TX_BUSY;
    }
    int_lock(cplock);
    int_unlock(cplock);
    desc_ring[queue].dataflags = 32768 | 16384 | 8192;
    desc_ring[queue].skb = skb;
    desc_ring[queue].data_mapping = ({
        dma_addr_t x;
        x;
    });
    desc_ring[queue].hw_data = (u32)desc_ring[queue].data_mapping;
    desc_ring[queue].datalen = len;
    desc_ring[queue].buflen = len;
    int_notify(cond_irq_can_happen);
    return NETDEV_TX_OK;
}


static void cpmac_end_xmit(int queue) {
    cpmac_write((1600 + (queue) * 4), (u32)desc_ring[queue].mapping);
    if (desc_ring[queue].skb) {
        int_lock(cplock);
        netdev.stats.tx_packets++;
        netdev.stats.tx_bytes += desc_ring[queue].skb->len;
        int_unlock(cplock);
        dma_unmap_single(desc_ring[queue].data_mapping, desc_ring[queue].skb->len, DMA_TO_DEVICE);
        {
            desc_ring[queue].skb = (struct sk_buff *)(addr_t)nondet_int;
        }
        ;
        desc_ring[queue].skb = ((void *)0);
        netif_wake_subqueue();
    } else {
        netif_wake_subqueue();
    }
}


static void cpmac_hw_stop() {
    int i;
    ar7_device_reset(pdata.reset_bit);
    cpmac_write(20, cpmac_read(20) & ~1);
    cpmac_write(4, cpmac_read(4) & ~1);
    cpmac_write((1536 + (i) * 4), 0);
    cpmac_write_CPMAC_RX_PTR(i, 0);
    cpmac_write(264, 255);
    cpmac_write(412, 255);
    cpmac_write(380, 255);
    cpmac_write(428, 255);
    cpmac_write(352, cpmac_read(352) & ~32);
}


static void cpmac_hw_start() {
    int i;
    ar7_device_reset(pdata.reset_bit);
    cpmac_write((1536 + (i) * 4), 0);
    cpmac_write_CPMAC_RX_PTR(i, 0);
    cpmac_write_CPMAC_RX_PTR(0, rx_head->mapping);
    cpmac_write(256, 8388608 | 8192 | 32);
    cpmac_write(272, 0);
    cpmac_write((432 + (i) * 4), netdev.dev_addr[5]);
    cpmac_write(464, netdev.dev_addr[4]);
    cpmac_write(468, netdev.dev_addr[0] | (netdev.dev_addr[1] << 8) | (netdev.dev_addr[2] << 16) | (netdev.dev_addr[3] << 24));
    cpmac_write(268, (1514 + 4 + 4));
    cpmac_write(264, 255);
    cpmac_write(412, 255);
    cpmac_write(380, 255);
    cpmac_write(428, 255);
    cpmac_write(260, 1);
    cpmac_write(408, 1);
    cpmac_write(376, 255);
    cpmac_write(424, 3);
    cpmac_write(20, cpmac_read(20) | 1);
    cpmac_write(4, cpmac_read(4) | 1);
    cpmac_write(352, cpmac_read(352) | 32 | 1);
}


static irqreturn_t cpmac_irq(int irq) {
    int queue;
    u32 status;
    status = cpmac_read_CPMAC_MAC_INT_VECTOR();
    if (status & 65536)
        cpmac_end_xmit((status & 7));
    if (status & 131072) {
        queue = (status >> 8) & 7;
    }
    int_reset(cond_irq_can_happen);
    return IRQ_HANDLED;
}


static int cpmac_open() {
    int i, size, res;
    struct resource *mem;
    struct cpmac_desc *desc;
    struct sk_buff *skb;
    mem = platform_get_resource_byname(512, "regs");
    if (!request_mem_region(mem->start, resource_size(mem), netdev.name)) {
        res = -6;
        goto fail_reserve;
    }
    regs = ioremap(mem->start, resource_size(mem));
    if (!regs) {
        res = -6;
        goto fail_remap;
    }
    size = ring_size + 8;
    desc_ring = ({
        dma_addr_t x;
        void *y;
        *(&dma_ring) = x;
        y;
    });
    if (!desc_ring) {
        res = -12;
        goto fail_alloc;
    }
    desc_ring[i].mapping = dma_ring + sizeof (*desc) * i;
    rx_head = &desc_ring[8];
    skb = netdev_alloc_skb_ip_align((1514 + 4 + 4));
    if (!skb) {
        res = -12;
        goto fail_desc;
    }
    rx_head[i].skb = skb;
    rx_head[i].data_mapping = ({
        dma_addr_t x;
        x;
    });
    rx_head[i].hw_data = (u32)rx_head[i].data_mapping;
    rx_head[i].buflen = (1514 + 4 + 4);
    rx_head[i].dataflags = 8192;
    rx_head[i].next = &rx_head[(i + 1) % ring_size];
    rx_head[i].next->prev = &rx_head[i];
    rx_head[i].hw_next = (u32)rx_head[i].next->mapping;
    rx_head->prev->hw_next = (u32)0;
    request_irq();
    cpmac_hw_start();
    return 0;
  fail_irq:
  fail_desc:
    if (rx_head[i].skb) {
        dma_unmap_single(rx_head[i].data_mapping, (1514 + 4 + 4), DMA_FROM_DEVICE);
        {
            rx_head[i].skb = (struct sk_buff *)(addr_t)nondet_int;
        }
        ;
    }
  fail_alloc:
    {
        *(int *)(desc_ring) = 0;
    }
    ;
    {
        *(int *)regs = nondet_int;
        ioval = nondet_int;
    }
    ;
  fail_remap:
    release_mem_region(mem->start, resource_size(mem));
  fail_reserve:
    return res;
}


static int cpmac_stop() {
    int i;
    struct resource *mem;
    netif_tx_stop_all_queues();
    cancel_work_sync();
    int_lock(l1);
    cpmac_hw_stop();
    cpmac_write((1536 + (i) * 4), 0);
    cpmac_write_CPMAC_RX_PTR(0, 0);
    cpmac_write(256, 0);
    {
        *(int *)regs = nondet_int;
        ioval = nondet_int;
    }
    ;
    mem = platform_get_resource_byname(512, "regs");
    release_mem_region(mem->start, resource_size(mem));
    rx_head = &desc_ring[8];
    if (rx_head[i].skb) {
        dma_unmap_single(rx_head[i].data_mapping, (1514 + 4 + 4), DMA_FROM_DEVICE);
        {
            rx_head[i].skb = (struct sk_buff *)(addr_t)nondet_int;
        }
        ;
    }
    {
        desc_ring = ((void *)0);
        dma_ring = 0;
    }
    ;
    free_irq();
    int_unlock(l1);
    return 0;
}


static int cpmac_probe() {
    int rc, phy_id;
    char mdio_bus_id[17];
    struct resource *mem;
    if (external_switch || dumb_switch) {
        ({
            (mdio_bus_id)[(20 - 3) - (20 - 3)] = ("fixed-0")[(20 - 3) - (20 - 3)];
            mdio_bus_id;
        });
        phy_id = nondet_int;
    } else {
        for (phy_id = 0; phy_id < 32; phy_id++) {
            if (!(pdata.phy_mask & (1 << phy_id)))
                continue;
            if (!cpmac_mii.phy_map[phy_id])
                continue;
            ({
                (mdio_bus_id)[(20 - 3) - (20 - 3)] = (cpmac_mii.id)[(20 - 3) - (20 - 3)];
                mdio_bus_id;
            });
            break;
        }
    }
    if (phy_id == 32) {
        ({
            (mdio_bus_id)[(20 - 3) - (20 - 3)] = ("fixed-0")[(20 - 3) - (20 - 3)];
            mdio_bus_id;
        });
        phy_id = nondet_int;
    }
    mdio_bus_id[sizeof (mdio_bus_id) - 1] = '\x00';
    mem = platform_get_resource_byname(512, "regs");
    if (!mem) {
        rc = -19;
        goto out;
    }
    netdev.irq = platform_get_irq_byname("irq");
    ring_size = 64;
    ({
        ((char *)(netdev.dev_addr))[sizeof (pdata.dev_addr) - 1] = ((char *)(pdata.dev_addr))[sizeof (pdata.dev_addr) - 1];
        netdev.dev_addr;
    });
    rc = register_netdev();
    if (rc) {
        goto fail;
    }
    return 0;
  fail:
  out:
    return rc;
}


static int cpmac_remove() {
    unregister_netdev();
    return 0;
}


int cpmac_init() {
    u32 mask;
    int i, res;
    cpmac_mii.priv = ioremap((140574720 + 7680), 256);
    if (!cpmac_mii.priv) {
        {
        }
        ;
        res = -6;
        goto fail_alloc;
    }
    ar7_gpio_disable(26);
    ar7_gpio_disable(27);
    ar7_device_reset(17);
    ar7_device_reset(21);
    ar7_device_reset(26);
    cpmac_mdio_reset();
    for (i = 0; i < 300; i++) {
        mask = cpmac_read(8);
        if (mask)
            break;
        else
            msleep(10);
    }
    mask &= 2147483647;
    if (mask & (mask - 1)) {
        external_switch = 1;
        mask = 0;
    }
    cpmac_mii_phy_mask = ~(mask | 2147483648U);
    res = platform_driver_register();
    if (nondet_int) {
        int_assume_not(cond_platform_driver_registered);
        goto fail_cpmac;
    }
    ;
    int_assume(cond_platform_driver_registered);
    return 0;
  fail_cpmac:
  fail_mii:
    {
        *(int *)cpmac_mii.priv = nondet_int;
        ioval = nondet_int;
    }
    ;
  fail_alloc:
    return res;
}


void cpmac_exit() {
    platform_driver_unregister();
    {
        *(int *)cpmac_mii.priv = nondet_int;
        ioval = nondet_int;
    }
    ;
}


void thread_init_exit() {
    cpmac_init();
    if (nondet_int) {
        int_assume_not(cond_platform_driver_registered);
        return;
    } else {
        int_assume(cond_platform_driver_registered);
        int_yield();
        int_wait(cond_platform_driver_not_in_use);
        cpmac_exit();
    }
    ;
}


void thread_probe_remove() {
    if (nondet_int) {
        int_assume(cond_platform_driver_registered);
        cpmac_probe();
        if (nondet_int) {
            int_assume(netdev_registered);
            int_yield();
            cpmac_remove();
        } else {
            int_assume_not(netdev_registered);
        }
    }
    ;
    int_notify(cond_platform_driver_not_in_use);
}


void thread_open_close() {
    if (nondet_int) {
        int_lock(rtnl);
        if (nondet_int) {
            int_assume(netdev_registered);
            int_notify(netdev_running);
            if (!cpmac_open()) {
                netif_start_queue();
                int_unlock(rtnl);
                int_yield();
                int_lock(rtnl);
                if (nondet_int) {
                    int_assume(netdev_running);
                    int_reset(netdev_running);
                    int_reset(send_enabled);
                    int_wait_not(send_in_progress);
                    cpmac_stop();
                } else {
                    int_assume_not(netdev_running);
                }
                ;
            }
            ;
        } else {
            int_assume_not(netdev_registered);
        }
        ;
        int_unlock(rtnl);
    }
}


void thread_irq() {
    while (nondet_int)
        {
            int_lock(irq_running_lock);
            int_assume(cond_irq_can_happen);
            int_lock(l1);
            int_assume(cond_irq_enabled);
            cpmac_irq(nondet_int);
            int_unlock(l1);
            int_unlock(irq_running_lock);
            int_yield();
        }
}


void thread_send() {
    while (nondet_int)
        {
            int_yield();
            int_notify(send_in_progress);
            if (nondet_int) {
                int_assume(send_enabled);
                int_assume(netdev_running);
                cpmac_start_xmit((struct sk_buff *)((addr_t)nondet_int));
            }
            ;
            int_reset(send_in_progress);
        }
}


