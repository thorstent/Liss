/*
 * Copyright (C) 2006, 2007 Eugene Konev
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "langinc.h"

#include "types.h"
#include "eth.h"

#include "langinc.h"
#include "types.h"

typedef u64 phys_addr_t;
typedef phys_addr_t resource_size_t;

lock_t synthlock_0;
conditional_t cond_irq_enabled;
lock_t irq_running_lock;

void request_irq () {
    notify(cond_irq_enabled);
}

void free_irq() {
    reset(cond_irq_enabled);
    unlock_s(synthlock_0);
    lock(irq_running_lock);
    unlock(irq_running_lock);
lock_s(synthlock_0);
}

struct resource {
        resource_size_t start;
        resource_size_t end;
        const char *name;
        unsigned long flags;
        //struct resource *parent, *sibling, *child;
};

static inline resource_size_t resource_size(const struct resource *res)
{
        return (resource_size_t) nondet;
}

conditional_t cond_platform_driver_registered;

int platform_driver_register() {
    if (nondet) {
        notify (cond_platform_driver_registered);
        return 0;
    } else {
        return -1;
    }
}

void platform_driver_unregister() {
    reset (cond_platform_driver_registered);
}

int platform_get_irq_byname(const char* name) {
    ioval = nondet;
    return (int)ioval;
}

struct resource * platform_get_resource_byname(unsigned int type, const char * name) {
    return (struct resource *)(addr_t)nondet;
}

struct resource * request_mem_region(resource_size_t start, resource_size_t n, const char *name) {
    return (struct resource *)(addr_t)nondet;
}

void release_mem_region(resource_size_t start, resource_size_t n)
{
    ioval = nondet;
}

#define pr_err(format, ...) {}

conditional_t cond_irq_can_happen;

int ar7_gpio_disable(unsigned gpio) {
    ioval = nondet;
    return nondet;
};
void ar7_device_reset(u32 bit) {
    ioval = nondet;
};

#define AR7_RESET_PERIPHERAL    0x0
#define AR7_RESET_SOFTWARE      0x4
#define AR7_RESET_STATUS        0x8

#define AR7_RESET_BIT_CPMAC_LO  17
#define AR7_RESET_BIT_CPMAC_HI  21
#define AR7_RESET_BIT_MDIO      22
#define AR7_RESET_BIT_EPHY      26

#define AR7_SDRAM_BASE  0x14000000

#define AR7_REGS_BASE   0x08610000

#define AR7_REGS_MAC0   (AR7_REGS_BASE + 0x0000)
#define AR7_REGS_GPIO   (AR7_REGS_BASE + 0x0900)
/* 0x08610A00 - 0x08610BFF (512 bytes, 128 bytes / clock) */
#define AR7_REGS_POWER  (AR7_REGS_BASE + 0x0a00)
#define AR7_REGS_CLOCKS (AR7_REGS_POWER + 0x80)
#define UR8_REGS_CLOCKS (AR7_REGS_POWER + 0x20)
#define AR7_REGS_UART0  (AR7_REGS_BASE + 0x0e00)
#define AR7_REGS_USB    (AR7_REGS_BASE + 0x1200)
#define AR7_REGS_RESET  (AR7_REGS_BASE + 0x1600)
#define AR7_REGS_PINSEL (AR7_REGS_BASE + 0x160C)
#define AR7_REGS_VLYNQ0 (AR7_REGS_BASE + 0x1800)
#define AR7_REGS_DCL    (AR7_REGS_BASE + 0x1a00)
#define AR7_REGS_VLYNQ1 (AR7_REGS_BASE + 0x1c00)
#define AR7_REGS_MDIO   (AR7_REGS_BASE + 0x1e00)
#define AR7_REGS_IRQ    (AR7_REGS_BASE + 0x2400)
#define AR7_REGS_MAC1   (AR7_REGS_BASE + 0x2800)

#define AR7_REGS_WDT    (AR7_REGS_BASE + 0x1f00)
#define UR8_REGS_WDT    (AR7_REGS_BASE + 0x0b00)
#define UR8_REGS_UART1  (AR7_REGS_BASE + 0x0f00)

//#include <linux/module.h>
//#include <linux/interrupt.h>
//#include <linux/moduleparam.h>
//
//#include <linux/sched.h>
//#include <linux/kernel.h>
//#include <linux/slab.h>
//#include <linux/errno.h>
//#include <linux/types.h>
//#include <linux/delay.h>
//
//#include <linux/netdevice.h>
//#include <linux/if_vlan.h>
//#include <linux/etherdevice.h>
//#include <linux/ethtool.h>
//#include <linux/skbuff.h>
//#include <linux/mii.h>
//#include <linux/phy.h>
//#include <linux/phy_fixed.h>
//#include <linux/platform_device.h>
//#include <linux/dma-mapping.h>
//#include <linux/clk.h>
//#include <linux/gpio.h>
//#include <linux/atomic.h>

//MODULE_AUTHOR("Eugene Konev <ejka@imfi.kspu.ru>");
//MODULE_DESCRIPTION("TI AR7 ethernet driver (CPMAC)");
//MODULE_LICENSE("GPL");
//MODULE_ALIAS("platform:cpmac");
//
//static int debug_level = 8;
static int dumb_switch;
//
///* Next 2 are only used in cpmac_probe, so it's pointless to change them */
//module_param(debug_level, int, 0444);
//module_param(dumb_switch, int, 0444);
//
//MODULE_PARM_DESC(debug_level, "Number of NETIF_MSG bits to enable");
//MODULE_PARM_DESC(dumb_switch, "Assume switch is not connected to MDIO bus");
//
//#define CPMAC_VERSION "0.5.2"
///* frame size + 802.1q tag + FCS size */
#define CPMAC_SKB_SIZE		(ETH_FRAME_LEN + ETH_FCS_LEN + VLAN_HLEN)
#define CPMAC_QUEUES	8

/* Ethernet registers */
#define CPMAC_TX_CONTROL		0x0004
#define CPMAC_TX_TEARDOWN		0x0008
#define CPMAC_RX_CONTROL		0x0014
#define CPMAC_RX_TEARDOWN		0x0018
#define CPMAC_MBP			0x0100
#define MBP_RXPASSCRC			0x40000000
#define MBP_RXQOS			0x20000000
#define MBP_RXNOCHAIN			0x10000000
#define MBP_RXCMF			0x01000000
#define MBP_RXSHORT			0x00800000
#define MBP_RXCEF			0x00400000
#define MBP_RXPROMISC			0x00200000
#define MBP_PROMISCCHAN(channel)	(((channel) & 0x7) << 16)
#define MBP_RXBCAST			0x00002000
#define MBP_BCASTCHAN(channel)		(((channel) & 0x7) << 8)
#define MBP_RXMCAST			0x00000020
#define MBP_MCASTCHAN(channel)		((channel) & 0x7)
#define CPMAC_UNICAST_ENABLE		0x0104
#define CPMAC_UNICAST_CLEAR		0x0108
#define CPMAC_MAX_LENGTH		0x010c
#define CPMAC_BUFFER_OFFSET		0x0110
#define CPMAC_MAC_CONTROL		0x0160
#define MAC_TXPTYPE			0x00000200
#define MAC_TXPACE			0x00000040
#define MAC_MII				0x00000020
#define MAC_TXFLOW			0x00000010
#define MAC_RXFLOW			0x00000008
#define MAC_MTEST			0x00000004
#define MAC_LOOPBACK			0x00000002
#define MAC_FDX				0x00000001
#define CPMAC_MAC_STATUS		0x0164
#define MAC_STATUS_QOS			0x00000004
#define MAC_STATUS_RXFLOW		0x00000002
#define MAC_STATUS_TXFLOW		0x00000001
#define CPMAC_TX_INT_ENABLE		0x0178
#define CPMAC_TX_INT_CLEAR		0x017c
#define CPMAC_MAC_INT_VECTOR		0x0180
#define MAC_INT_STATUS			0x00080000
#define MAC_INT_HOST			0x00040000
#define MAC_INT_RX			0x00020000
#define MAC_INT_TX			0x00010000
#define CPMAC_MAC_EOI_VECTOR		0x0184
#define CPMAC_RX_INT_ENABLE		0x0198
#define CPMAC_RX_INT_CLEAR		0x019c
#define CPMAC_MAC_INT_ENABLE		0x01a8
#define CPMAC_MAC_INT_CLEAR		0x01ac
#define CPMAC_MAC_ADDR_LO(channel)	(0x01b0 + (channel) * 4)
#define CPMAC_MAC_ADDR_MID		0x01d0
#define CPMAC_MAC_ADDR_HI		0x01d4
#define CPMAC_MAC_HASH_LO		0x01d8
#define CPMAC_MAC_HASH_HI		0x01dc
#define CPMAC_TX_PTR(channel)		(0x0600 + (channel) * 4)
#define CPMAC_RX_PTR(channel)		(0x0620 + (channel) * 4)
#define CPMAC_TX_ACK(channel)		(0x0640 + (channel) * 4)
#define CPMAC_RX_ACK(channel)		(0x0660 + (channel) * 4)
#define CPMAC_REG_END			0x0680

///* Rx/Tx statistics
// * TODO: use some of them to fill stats in cpmac_stats()
// */
//#define CPMAC_STATS_RX_GOOD		0x0200
//#define CPMAC_STATS_RX_BCAST		0x0204
//#define CPMAC_STATS_RX_MCAST		0x0208
//#define CPMAC_STATS_RX_PAUSE		0x020c
//#define CPMAC_STATS_RX_CRC		0x0210
//#define CPMAC_STATS_RX_ALIGN		0x0214
//#define CPMAC_STATS_RX_OVER		0x0218
//#define CPMAC_STATS_RX_JABBER		0x021c
//#define CPMAC_STATS_RX_UNDER		0x0220
//#define CPMAC_STATS_RX_FRAG		0x0224
//#define CPMAC_STATS_RX_FILTER		0x0228
//#define CPMAC_STATS_RX_QOSFILTER	0x022c
//#define CPMAC_STATS_RX_OCTETS		0x0230
//
//#define CPMAC_STATS_TX_GOOD		0x0234
//#define CPMAC_STATS_TX_BCAST		0x0238
//#define CPMAC_STATS_TX_MCAST		0x023c
//#define CPMAC_STATS_TX_PAUSE		0x0240
//#define CPMAC_STATS_TX_DEFER		0x0244
//#define CPMAC_STATS_TX_COLLISION	0x0248
//#define CPMAC_STATS_TX_SINGLECOLL	0x024c
//#define CPMAC_STATS_TX_MULTICOLL	0x0250
//#define CPMAC_STATS_TX_EXCESSCOLL	0x0254
//#define CPMAC_STATS_TX_LATECOLL		0x0258
//#define CPMAC_STATS_TX_UNDERRUN		0x025c
//#define CPMAC_STATS_TX_CARRIERSENSE	0x0260
//#define CPMAC_STATS_TX_OCTETS		0x0264
//


//#define cpmac_write(base, reg, val)	(writel(val, (void __iomem *)(base) + \
//					       (reg)))

u32 cpmac_mii_phy_mask;

/* MDIO bus */
#define CPMAC_MDIO_VERSION		0x0000
#define CPMAC_MDIO_CONTROL		0x0004
#define MDIOC_IDLE			0x80000000
#define MDIOC_ENABLE			0x40000000
#define MDIOC_PREAMBLE			0x00100000
#define MDIOC_FAULT			0x00080000
#define MDIOC_FAULTDETECT		0x00040000
#define MDIOC_INTTEST			0x00020000
#define MDIOC_CLKDIV(div)		((div) & 0xff)
#define CPMAC_MDIO_ALIVE		0x0008
#define CPMAC_MDIO_LINK			0x000c
#define CPMAC_MDIO_ACCESS(channel)	(0x0080 + (channel) * 8)
#define MDIO_BUSY			0x80000000
#define MDIO_WRITE			0x40000000
#define MDIO_REG(reg)			(((reg) & 0x1f) << 21)
#define MDIO_PHY(phy)			(((phy) & 0x1f) << 16)
#define MDIO_DATA(data)			((data) & 0xffff)
#define CPMAC_MDIO_PHYSEL(channel)	(0x0084 + (channel) * 8)
#define PHYSEL_LINKSEL			0x00000040
#define PHYSEL_LINKINT			0x00000020


struct cpmac_desc {
	u32 hw_next;
	u32 hw_data;
	u16 buflen;
	u16 bufflags;
	u16 datalen;
	u16 dataflags;
#define CPMAC_SOP			0x8000
#define CPMAC_EOP			0x4000
#define CPMAC_OWN			0x2000
#define CPMAC_EOQ			0x1000
	struct sk_buff *skb;
	struct cpmac_desc *next;
	struct cpmac_desc *prev;
	dma_addr_t mapping;
	dma_addr_t data_mapping;
};

//
//struct {
lock_t cplock;
lock_t rx_lock;

struct cpmac_desc *rx_head;
int ring_size;
struct cpmac_desc *desc_ring;
dma_addr_t dma_ring;
void __iomem *regs;
//struct mii_bus *mii_bus;
struct phy_device *phy;
char phy_name[MII_BUS_ID_SIZE + 3];
int oldlink, oldspeed, oldduplex;
u32 msg_enable;
struct net_device dev;
struct work_struct reset_work;
//struct platform_device *pdev;
//struct napi_struct napi;
atomic_t reset_pending;
/*} priv;*/

void cpmac_write(u32 reg, u32 val) {
    *(int*)regs = nondet;
    ioval = nondet;
};

u32 cpmac_read(u32 reg)	{
    return ioval+(*(int*) regs);
}

u32 cpmac_int_vector;

u32 cpmac_read_CPMAC_MAC_INT_VECTOR()	{
    return cpmac_int_vector;
}


u32 cpmac_rx_ptr;

void cpmac_write_CPMAC_RX_PTR(u32 i, u32 val) {
    cpmac_rx_ptr = val;
}

u32 cpmac_read_CPMAC_RX_PTR(u32 i) {
    return cpmac_rx_ptr;
}



struct plat_cpmac_data {
        int reset_bit;
        int power_bit;
        u32 phy_mask;
        char dev_addr[6];
} pdata;

//static irqreturn_t cpmac_irq(int, void *);
static void cpmac_hw_start();
static void cpmac_hw_stop(/*struct net_device *dev*/);
//static int cpmac_open(struct net_device *dev);
//
//static void cpmac_dump_regs(struct net_device *dev)
//{
//	int i;
//	struct cpmac_priv *priv = netdev_priv(dev);
//
//	for (i = 0; i < CPMAC_REG_END; i += 4) {
//		if (i % 16 == 0) {
//			if (i)
//				printk("\n");
//			printk("%s: reg[%p]:", dev->name, priv->regs + i);
//		}
//		printk(" %08x", cpmac_read(priv->regs, i));
//	}
//	printk("\n");
//}
//
//static void cpmac_dump_desc(struct net_device *dev, struct cpmac_desc *desc)
//{
//	int i;
//
//	printk("%s: desc[%p]:", dev->name, desc);
//	for (i = 0; i < sizeof(*desc) / 4; i++)
//		printk(" %08x", ((u32 *)desc)[i]);
//	printk("\n");
//}
//
//static void cpmac_dump_all_desc(struct net_device *dev)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//	struct cpmac_desc *dump = priv->rx_head;
//
//	do {
//		cpmac_dump_desc(dev, dump);
//		dump = dump->next;
//	} while (dump != priv->rx_head);
//}
//
//static void cpmac_dump_skb(struct net_device *dev, struct sk_buff *skb)
//{
//	int i;
//
//	printk("%s: skb 0x%p, len=%d\n", dev->name, skb, skb->len);
//	for (i = 0; i < skb->len; i++) {
//		if (i % 16 == 0) {
//			if (i)
//				printk("\n");
//			printk("%s: data[%p]:", dev->name, skb->data + i);
//		}
//		printk(" %02x", ((u8 *)skb->data)[i]);
//	}
//	printk("\n");
//}
//
//static int cpmac_mdio_read(struct mii_bus *bus, int phy_id, int reg)
//{
//	u32 val;
//
//	while (cpmac_read(bus->priv, CPMAC_MDIO_ACCESS(0)) & MDIO_BUSY)
//		cpu_relax();
//	cpmac_write(bus->priv, CPMAC_MDIO_ACCESS(0), MDIO_BUSY | MDIO_REG(reg) |
//		    MDIO_PHY(phy_id));
//	while ((val = cpmac_read(bus->priv, CPMAC_MDIO_ACCESS(0))) & MDIO_BUSY)
//		cpu_relax();
//
//	return MDIO_DATA(val);
//}
//
//static int cpmac_mdio_write(struct mii_bus *bus, int phy_id,
//			    int reg, u16 val)
//{
//	while (cpmac_read(bus->priv, CPMAC_MDIO_ACCESS(0)) & MDIO_BUSY)
//		cpu_relax();
//	cpmac_write(bus->priv, CPMAC_MDIO_ACCESS(0), MDIO_BUSY | MDIO_WRITE |
//		    MDIO_REG(reg) | MDIO_PHY(phy_id) | MDIO_DATA(val));
//
//	return 0;
//}
//
static int cpmac_mdio_reset()
{
//	struct clk *cpmac_clk;

//	cpmac_clk = clk_get("cpmac");
//	if (IS_ERR(cpmac_clk)) {
//		pr_err("unable to get cpmac clock\n");
//		return -1;
//	}
	ar7_device_reset(AR7_RESET_BIT_MDIO);
	cpmac_write(CPMAC_MDIO_CONTROL, MDIOC_ENABLE |
		    MDIOC_CLKDIV(/*clk_get_rate(cpmac_clk)*/nondet / 2200000 - 1));

	return 0;
}

static int mii_irqs[PHY_MAX_ADDR] = { -1, };

static struct mii_bus cpmac_mii;

//static void cpmac_set_multicast_list(struct net_device *dev)
//{
//	struct netdev_hw_addr *ha;
//	u8 tmp;
//	u32 mbp, bit, hash[2] = { 0, };
//	struct cpmac_priv *priv = netdev_priv(dev);
//
//	mbp = cpmac_read(priv->regs, CPMAC_MBP);
//	if (dev->flags & IFF_PROMISC) {
//		cpmac_write(priv->regs, CPMAC_MBP, (mbp & ~MBP_PROMISCCHAN(0)) |
//			    MBP_RXPROMISC);
//	} else {
//		cpmac_write(priv->regs, CPMAC_MBP, mbp & ~MBP_RXPROMISC);
//		if (dev->flags & IFF_ALLMULTI) {
//			/* enable all multicast mode */
//			cpmac_write(priv->regs, CPMAC_MAC_HASH_LO, 0xffffffff);
//			cpmac_write(priv->regs, CPMAC_MAC_HASH_HI, 0xffffffff);
//		} else {
//			/* cpmac uses some strange mac address hashing
//			 * (not crc32)
//			 */
//			netdev_for_each_mc_addr(ha, dev) {
//				bit = 0;
//				tmp = ha->addr[0];
//				bit  ^= (tmp >> 2) ^ (tmp << 4);
//				tmp = ha->addr[1];
//				bit  ^= (tmp >> 4) ^ (tmp << 2);
//				tmp = ha->addr[2];
//				bit  ^= (tmp >> 6) ^ tmp;
//				tmp = ha->addr[3];
//				bit  ^= (tmp >> 2) ^ (tmp << 4);
//				tmp = ha->addr[4];
//				bit  ^= (tmp >> 4) ^ (tmp << 2);
//				tmp = ha->addr[5];
//				bit  ^= (tmp >> 6) ^ tmp;
//				bit &= 0x3f;
//				hash[bit / 32] |= 1 << (bit % 32);
//			}
//
//			cpmac_write(priv->regs, CPMAC_MAC_HASH_LO, hash[0]);
//			cpmac_write(priv->regs, CPMAC_MAC_HASH_HI, hash[1]);
//		}
//	}
//}
//
//static struct sk_buff *cpmac_rx_one()
//{
//	struct sk_buff *skb, *result = NULL;
//
////	if (unlikely(netif_msg_hw(priv)))
////		cpmac_dump_rx_head(priv->dev, rx_head);
//	cpmac_write(CPMAC_RX_ACK(0), (u32)rx_head->mapping);
//	if (unlikely(!rx_head->datalen)) {
////		if (netif_msg_rx_err(priv) && net_ratelimit())
////			netdev_warn(priv->dev, "rx: spurious interrupt\n");
//
//		return NULL;
//	}
//
//	skb = netdev_alloc_skb_ip_align(CPMAC_SKB_SIZE);
//	if (likely(skb)) {
//		skb_put(rx_head->skb, rx_head->datalen);
//		rx_head->skb->protocol = eth_type_trans(rx_head->skb);
//		skb_checksum_none_assert(rx_head->skb);
//		netdev.stats.rx_packets++;
//		netdev.stats.rx_bytes += rx_head->datalen;
//		result = rx_head->skb;
//		dma_unmap_single(rx_head->data_mapping,
//				 CPMAC_SKB_SIZE, DMA_FROM_DEVICE);
//		rx_head->skb = skb;
//		rx_head->data_mapping = dma_map_single(skb->data,
//						    CPMAC_SKB_SIZE,
//						    DMA_FROM_DEVICE);
//		rx_head->hw_data = (u32)rx_head->data_mapping;
////		if (unlikely(netif_msg_pktdata(priv))) {
////			netdev_dbg(priv->dev, "received packet:\n");
////			cpmac_dump_skb(priv->dev, result);
////		}
//	} else {
////		if (netif_msg_rx_err(priv) && net_ratelimit())
////			netdev_warn(priv->dev,
////				    "low on skbs, dropping packet\n");
//
//		netdev.stats.rx_dropped++;
//	}
//
//	rx_head->buflen = CPMAC_SKB_SIZE;
//	rx_head->dataflags = CPMAC_OWN;
//
//	return result;
//}
//
//static int cpmac_poll(int budget)
//{
//	struct sk_buff *skb;
//	struct cpmac_desc *desc, *restart;
////	struct cpmac_priv *priv = container_of(napi, struct cpmac_priv, napi);
//	int received = 0, processed = 0;
//
//	spin_lock(rx_lock);
//	if (unlikely(!rx_head)) {
////		if (netif_msg_rx_err(priv) && net_ratelimit())
////			netdev_warn(priv->dev, "rx: polling, but no queue\n");
//
//		spin_unlock(rx_lock);
//		//napi_complete();
//		return 0;
//	}
//
////	desc = rx_head;
//	restart = NULL;
////	while (((rx_head->dataflags & CPMAC_OWN) == 0) && (received < budget)) {
//		processed++;
//
////		if ((rx_head->dataflags & CPMAC_EOQ) != 0) {
////			/* The last update to eoq->hw_next didn't happen
////			 * soon enough, and the receiver stopped here.
////			 * Remember this rx_headriptor so we can restart
////			 * the receiver after freeing some space.
////			 */
////			if (unlikely(restart)) {
//////				if (netif_msg_rx_err(priv))
//////					netdev_err(priv->dev, "poll found a"
//////						   " duplicate EOQ: %p and %p\n",
//////						   restart, rx_head);
////				goto fatal_error;
////			}
////
////			restart = rx_head->next;
////		}
//
//		skb = cpmac_rx_one();
//		if (likely(skb)) {
//			netif_receive_skb(skb);
//			received++;
//		}
//		//rx_head = rx_head->next;
////	}
//
//	if (rx_head != rx_head+1) {
//		/* We freed some buffers, but not the whole ring,
//		 * add what we did free to the rx list
//		 */
//		rx_head->prev->hw_next = (u32)0;
//		rx_head->prev->hw_next = rx_head->mapping;
//	}
//
//	/* Optimization: If we did not actually process an EOQ (perhaps because
//	 * of quota limits), check to see if the tail of the queue has EOQ set.
//	 * We should immediately restart in that case so that the receiver can
//	 * restart and run in parallel with more packet processing.
//	 * This lets us handle slightly larger bursts before running
//	 * out of ring space (assuming dev->weight < ring_size)
//	 */
//
//	if (!restart &&
//	     (rx_head->prev->dataflags & (CPMAC_OWN|CPMAC_EOQ))
//		    == CPMAC_EOQ &&
//	     (rx_head->dataflags & CPMAC_OWN) != 0) {
//		/* reset EOQ so the poll loop (above) doesn't try to
//		 * restart this when it eventually gets to this rx_headriptor.
//		 */
//		rx_head->prev->dataflags &= ~CPMAC_EOQ;
//		restart = rx_head;
//	}
//
//	if (restart) {
////		netdev.stats.rx_errors++;
////		netdev.stats.rx_fifo_errors++;
////		if (netif_msg_rx_err(priv) && net_ratelimit())
////			netdev_warn(priv->dev, "rx dma ring overrun\n");
//
////		if (unlikely((restart->dataflags & CPMAC_OWN) == 0)) {
//////			if (netif_msg_drv(priv))
//////				netdev_err(priv->dev, "cpmac_poll is trying "
//////					"to restart rx from a rx_headriptor "
//////					"that's not free: %p\n", restart);
////			goto fatal_error;
////		}
//
//		cpmac_write_CPMAC_RX_PTR(0, restart->mapping);
//	}
//
//	rx_head = rx_head;
//	spin_unlock(rx_lock);
////	if (unlikely(netif_msg_rx_status(priv)))
////		netdev_dbg(priv->dev, "poll processed %d packets\n", received);
//
//	if (processed == 0) {
//		/* we ran out of packets to read,
//		 * revert to interrupt-driven mode
//		 */
//		//napi_complete();
//                // TODO
//		//cpmac_write(CPMAC_RX_INT_ENABLE, 1);
//		return 0;
//	}
//
//	return 1;
//
////fatal_error:
////	/* Something went horribly wrong.
////	 * Reset hardware to try to recover rather than wedging.
////	 */
//////	if (netif_msg_drv(priv)) {
//////		netdev_err(priv->dev, "cpmac_poll is confused. "
//////			   "Resetting hardware\n");
//////		cpmac_dump_all_rx_head(priv->dev);
//////		netdev_dbg(priv->dev, "RX_PTR(0)=0x%08x RX_ACK(0)=0x%08x\n",
//////			   cpmac_read(priv->regs, CPMAC_RX_PTR(0)),
//////			   cpmac_read(priv->regs, CPMAC_RX_ACK(0)));
//////	}
////
////	spin_unlock(rx_lock);
////	napi_complete();
////	netif_tx_stop_all_queues();
////	
////        //napi_disable();
////
//////	atomic_inc(reset_pending);
////	cpmac_hw_stop();
////        
////        // TODO
//////	if (!schedule_work(&priv->reset_work))
//////		atomic_dec(&priv->reset_pending);
////
////	return 0;
//
//}

static int cpmac_start_xmit(struct sk_buff *skb)
{
	int queue, len;
	//struct cpmac_desc *desc;
	//struct cpmac_priv *priv = netdev_priv(dev);

	//if (unlikely(atomic_read(reset_pending)))
	//	return NETDEV_TX_BUSY;

	if (unlikely(skb_padto(skb, ETH_ZLEN)))
		return NETDEV_TX_OK;

	len = max(skb->len, ETH_ZLEN);
	//queue = skb_get_queue_mapping(skb);
	netif_stop_subqueue(/*queue*/);

	//desc = &desc_ring[queue];
	if (unlikely(desc_ring[queue].dataflags & CPMAC_OWN)) {
//		if (netif_msg_tx_err(priv) && net_ratelimit())
//			netdev_warn(dev, "tx dma ring full\n");

		return NETDEV_TX_BUSY;
	}

	spin_lock(cplock);
	spin_unlock(cplock);
	desc_ring[queue].dataflags = CPMAC_SOP | CPMAC_EOP | CPMAC_OWN;
	desc_ring[queue].skb = skb;
	desc_ring[queue].data_mapping = dma_map_single(skb->data, len,
					    DMA_TO_DEVICE);
	desc_ring[queue].hw_data = (u32)desc_ring[queue].data_mapping;
	desc_ring[queue].datalen = len;
	desc_ring[queue].buflen = len;
//	if (unlikely(netif_msg_tx_queued(priv)))
//		netdev_dbg(dev, "sending 0x%p, len=%d\n", skb, skb->len);
//	if (unlikely(netif_msg_hw(priv)))
//		cpmac_dump_desc(dev, &desc_ring[queue]);
//	if (unlikely(netif_msg_pktdata(priv)))
//		cpmac_dump_skb(dev, skb);
	
        //cpmac_write(CPMAC_TX_PTR(queue), (u32)desc_ring[queue].mapping);
        notify(cond_irq_can_happen);

	return NETDEV_TX_OK;
}

static void cpmac_end_xmit(int queue)
{
//	struct cpmac_desc *desc;
	//struct cpmac_priv *priv = netdev_priv(dev);

//	desc = desc_ring[queue];
	cpmac_write(CPMAC_TX_ACK(queue), (u32)desc_ring[queue].mapping);
	if (likely(desc_ring[queue].skb)) {
		unlock_s(synthlock_0);
		spin_lock(cplock);
		netdev.stats.tx_packets++;
		lock_s(synthlock_0);
		netdev.stats.tx_bytes += desc_ring[queue].skb->len;
		spin_unlock(cplock);
		dma_unmap_single(desc_ring[queue].data_mapping, desc_ring[queue].skb->len,
				 DMA_TO_DEVICE);

//		if (unlikely(netif_msg_tx_done(priv)))
//			netdev_dbg(dev, "sent 0x%p, len=%d\n",
//				   desc_ring[queue].skb, desc_ring[queue].skb->len);

		dev_kfree_skb_irq(desc_ring[queue].skb);
		desc_ring[queue].skb = NULL;
		//if (__netif_subqueue_stopped(dev, queue))
			unlock_s(synthlock_0);
			netif_wake_subqueue();
	} else {
//		if (netif_msg_tx_err(priv) && net_ratelimit())
//			netdev_warn(dev, "end_xmit: spurious interrupt\n");
		//if (__netif_subqueue_stopped(dev, queue))
			unlock_s(synthlock_0);
			netif_wake_subqueue();
	}
lock_s(synthlock_0);
}

static void cpmac_hw_stop(/*struct net_device *dev*/)
{
	int i;
	//struct cpmac_priv *priv = netdev_priv(dev);
	//struct plat_cpmac_data *pdata = dev_get_platdata(&priv->pdev->dev);

	ar7_device_reset(pdata.reset_bit);
	cpmac_write(CPMAC_RX_CONTROL,
		    cpmac_read(CPMAC_RX_CONTROL) & ~1);
	cpmac_write(CPMAC_TX_CONTROL,
		    cpmac_read(CPMAC_TX_CONTROL) & ~1);
	//for (i = 0; i < 8; i++) {
		cpmac_write(CPMAC_TX_PTR(i), 0);
		cpmac_write_CPMAC_RX_PTR(i, 0);
	//}
	cpmac_write(CPMAC_UNICAST_CLEAR, 0xff);
	cpmac_write(CPMAC_RX_INT_CLEAR, 0xff);
	cpmac_write(CPMAC_TX_INT_CLEAR, 0xff);
	cpmac_write(CPMAC_MAC_INT_CLEAR, 0xff);
	cpmac_write(CPMAC_MAC_CONTROL,
		    cpmac_read(CPMAC_MAC_CONTROL) & ~MAC_MII);
}

static void cpmac_hw_start()
{
	int i;
	//struct cpmac_priv *priv = netdev_priv(dev);
	//struct plat_cpmac_data *pdata = dev_get_platdata(&priv->pdev->dev);

	ar7_device_reset(pdata.reset_bit);
	//for (i = 0; i < 8; i++) {
		cpmac_write(CPMAC_TX_PTR(i), 0);
		cpmac_write_CPMAC_RX_PTR(i, 0);
	//}
	cpmac_write_CPMAC_RX_PTR(0, rx_head->mapping);

	cpmac_write(CPMAC_MBP, MBP_RXSHORT | MBP_RXBCAST |
		    MBP_RXMCAST);
	cpmac_write(CPMAC_BUFFER_OFFSET, 0);
	//for (i = 0; i < 8; i++)
		cpmac_write(CPMAC_MAC_ADDR_LO(i), netdev.dev_addr[5]);
	cpmac_write(CPMAC_MAC_ADDR_MID, netdev.dev_addr[4]);
	cpmac_write(CPMAC_MAC_ADDR_HI, netdev.dev_addr[0] |
		    (netdev.dev_addr[1] << 8) | (netdev.dev_addr[2] << 16) |
		    (netdev.dev_addr[3] << 24));
	cpmac_write(CPMAC_MAX_LENGTH, CPMAC_SKB_SIZE);
	cpmac_write(CPMAC_UNICAST_CLEAR, 0xff);
	cpmac_write(CPMAC_RX_INT_CLEAR, 0xff);
	cpmac_write(CPMAC_TX_INT_CLEAR, 0xff);
	cpmac_write(CPMAC_MAC_INT_CLEAR, 0xff);
	cpmac_write(CPMAC_UNICAST_ENABLE, 1);
	cpmac_write(CPMAC_RX_INT_ENABLE, 1);
	cpmac_write(CPMAC_TX_INT_ENABLE, 0xff);
	cpmac_write(CPMAC_MAC_INT_ENABLE, 3);

	cpmac_write(CPMAC_RX_CONTROL,
		    cpmac_read(CPMAC_RX_CONTROL) | 1);
	cpmac_write(CPMAC_TX_CONTROL,
		    cpmac_read(CPMAC_TX_CONTROL) | 1);
	cpmac_write(CPMAC_MAC_CONTROL,
		    cpmac_read(CPMAC_MAC_CONTROL) | MAC_MII |
		    MAC_FDX);
}

//static void cpmac_clear_rx(struct net_device *dev)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//	struct cpmac_desc *desc;
//	int i;
//
//	if (unlikely(!priv->rx_head))
//		return;
//	desc = priv->rx_head;
//	for (i = 0; i < priv->ring_size; i++) {
//		if ((desc->dataflags & CPMAC_OWN) == 0) {
//			if (netif_msg_rx_err(priv) && net_ratelimit())
//				netdev_warn(dev, "packet dropped\n");
//			if (unlikely(netif_msg_hw(priv)))
//				cpmac_dump_desc(dev, desc);
//			desc->dataflags = CPMAC_OWN;
//			dev->stats.rx_dropped++;
//		}
//		desc->hw_next = desc->next->mapping;
//		desc = desc->next;
//	}
//	priv->rx_head->prev->hw_next = 0;
//}
//
//static void cpmac_clear_tx(struct net_device *dev)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//	int i;
//
//	if (unlikely(!priv->desc_ring))
//		return;
//	for (i = 0; i < CPMAC_QUEUES; i++) {
//		priv->desc_ring[i].dataflags = 0;
//		if (priv->desc_ring[i].skb) {
//			dev_kfree_skb_any(priv->desc_ring[i].skb);
//			priv->desc_ring[i].skb = NULL;
//		}
//	}
//}
//
//static void cpmac_hw_error(struct work_struct *work)
//{
//	struct cpmac_priv *priv =
//		container_of(work, struct cpmac_priv, reset_work);
//
//	spin_lock(&priv->rx_lock);
//	cpmac_clear_rx(priv->dev);
//	spin_unlock(&priv->rx_lock);
//	cpmac_clear_tx(priv->dev);
//	cpmac_hw_start(priv->dev);
//	barrier();
//	atomic_dec(&priv->reset_pending);
//
//	netif_tx_wake_all_queues(priv->dev);
//	cpmac_write(priv->regs, CPMAC_MAC_INT_ENABLE, 3);
//}
//
//static void cpmac_check_status(struct net_device *dev)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//
//	u32 macstatus = cpmac_read(priv->regs, CPMAC_MAC_STATUS);
//	int rx_channel = (macstatus >> 8) & 7;
//	int rx_code = (macstatus >> 12) & 15;
//	int tx_channel = (macstatus >> 16) & 7;
//	int tx_code = (macstatus >> 20) & 15;
//
//	if (rx_code || tx_code) {
//		if (netif_msg_drv(priv) && net_ratelimit()) {
//			/* Can't find any documentation on what these
//			 * error codes actually are. So just log them and hope..
//			 */
//			if (rx_code)
//				netdev_warn(dev, "host error %d on rx "
//					"channel %d (macstatus %08x), resetting\n",
//					rx_code, rx_channel, macstatus);
//			if (tx_code)
//				netdev_warn(dev, "host error %d on tx "
//					"channel %d (macstatus %08x), resetting\n",
//					tx_code, tx_channel, macstatus);
//		}
//
//		netif_tx_stop_all_queues(dev);
//		cpmac_hw_stop(dev);
//		if (schedule_work(&priv->reset_work))
//			atomic_inc(&priv->reset_pending);
//		if (unlikely(netif_msg_hw(priv)))
//			cpmac_dump_regs(dev);
//	}
//	cpmac_write(priv->regs, CPMAC_MAC_INT_CLEAR, 0xff);
//}

static irqreturn_t cpmac_irq(int irq)
{
//	struct net_device *dev = dev_id;
//	struct cpmac_priv *priv;
	int queue;
	u32 status;

//	priv = netdev_priv(dev);

	status = cpmac_read_CPMAC_MAC_INT_VECTOR();

//	if (unlikely(netif_msg_intr(priv)))
//		netdev_dbg(dev, "interrupt status: 0x%08x\n", status);

	if (status & MAC_INT_TX)
		cpmac_end_xmit((status & 7));

	if (status & MAC_INT_RX) {
		queue = (status >> 8) & 7;
		//if (napi_schedule_prep(&priv->napi)) {
		//	cpmac_write(CPMAC_RX_INT_CLEAR, 1 << queue);
		//	__napi_schedule();
		//}
                //cpmac_poll (nondet);
	}

	//cpmac_write(CPMAC_MAC_EOI_VECTOR, 0);
        unlock_s(synthlock_0);
        reset(cond_irq_can_happen);

        // TODO
//	if (unlikely(status & (MAC_INT_HOST | MAC_INT_STATUS)))
//		cpmac_check_status(dev);

	lock_s(synthlock_0);
	return IRQ_HANDLED;
}

//static void cpmac_tx_timeout(struct net_device *dev)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//
//	spin_lock(&priv->cplock);
//	dev->stats.tx_errors++;
//	spin_unlock(&priv->cplock);
//	if (netif_msg_tx_err(priv) && net_ratelimit())
//		netdev_warn(dev, "transmit timeout\n");
//
//	atomic_inc(&priv->reset_pending);
//	barrier();
//	cpmac_clear_tx(dev);
//	barrier();
//	atomic_dec(&priv->reset_pending);
//
//	netif_tx_wake_all_queues(priv->dev);
//}
//
//static int cpmac_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//
//	if (!(netif_running(dev)))
//		return -EINVAL;
//	if (!priv->phy)
//		return -EINVAL;
//
//	return phy_mii_ioctl(priv->phy, ifr, cmd);
//}
//
//static int cpmac_get_settings(struct net_device *dev, struct ethtool_cmd *cmd)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//
//	if (priv->phy)
//		return phy_ethtool_gset(priv->phy, cmd);
//
//	return -EINVAL;
//}
//
//static int cpmac_set_settings(struct net_device *dev, struct ethtool_cmd *cmd)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//
//	if (!capable(CAP_NET_ADMIN))
//		return -EPERM;
//
//	if (priv->phy)
//		return phy_ethtool_sset(priv->phy, cmd);
//
//	return -EINVAL;
//}
//
//static void cpmac_get_ringparam(struct net_device *dev,
//						struct ethtool_ringparam *ring)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//
//	ring->rx_max_pending = 1024;
//	ring->rx_mini_max_pending = 1;
//	ring->rx_jumbo_max_pending = 1;
//	ring->tx_max_pending = 1;
//
//	ring->rx_pending = priv->ring_size;
//	ring->rx_mini_pending = 1;
//	ring->rx_jumbo_pending = 1;
//	ring->tx_pending = 1;
//}
//
//static int cpmac_set_ringparam(struct net_device *dev,
//						struct ethtool_ringparam *ring)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//
//	if (netif_running(dev))
//		return -EBUSY;
//	priv->ring_size = ring->rx_pending;
//
//	return 0;
//}
//
//static void cpmac_get_drvinfo(struct net_device *dev,
//			      struct ethtool_drvinfo *info)
//{
//	strlcpy(info->driver, "cpmac", sizeof(info->driver));
//	strlcpy(info->version, CPMAC_VERSION, sizeof(info->version));
//	snprintf(info->bus_info, sizeof(info->bus_info), "%s", "cpmac");
//	info->regdump_len = 0;
//}
//
//static const struct ethtool_ops cpmac_ethtool_ops = {
//	.get_settings = cpmac_get_settings,
//	.set_settings = cpmac_set_settings,
//	.get_drvinfo = cpmac_get_drvinfo,
//	.get_link = ethtool_op_get_link,
//	.get_ringparam = cpmac_get_ringparam,
//	.set_ringparam = cpmac_set_ringparam,
//};
//
//static void cpmac_adjust_link(struct net_device *dev)
//{
//	struct cpmac_priv *priv = netdev_priv(dev);
//	int new_state = 0;
//
//	spin_lock(&priv->cplock);
//	if (priv->phy->link) {
//		netif_tx_start_all_queues(dev);
//		if (priv->phy->duplex != priv->oldduplex) {
//			new_state = 1;
//			priv->oldduplex = priv->phy->duplex;
//		}
//
//		if (priv->phy->speed != priv->oldspeed) {
//			new_state = 1;
//			priv->oldspeed = priv->phy->speed;
//		}
//
//		if (!priv->oldlink) {
//			new_state = 1;
//			priv->oldlink = 1;
//		}
//	} else if (priv->oldlink) {
//		new_state = 1;
//		priv->oldlink = 0;
//		priv->oldspeed = 0;
//		priv->oldduplex = -1;
//	}
//
//	if (new_state && netif_msg_link(priv) && net_ratelimit())
//		phy_print_status(priv->phy);
//
//	spin_unlock(&priv->cplock);
//}

static int cpmac_open()
{
	int i, size, res;
	//struct cpmac_priv *priv = netdev_priv(dev);
	struct resource *mem;
	struct cpmac_desc *desc;
	struct sk_buff *skb;

	mem = platform_get_resource_byname(IORESOURCE_MEM, "regs");
	if (!request_mem_region(mem->start, resource_size(mem), netdev.name)) {
//		if (netif_msg_drv(priv))
//			netdev_err(dev, "failed to request registers\n");

		res = -ENXIO;
		goto fail_reserve;
	}

	regs = ioremap(mem->start, resource_size(mem));
	if (!regs) {
//		if (netif_msg_drv(priv))
//			netdev_err(dev, "failed to remap registers\n");

		res = -ENXIO;
		goto fail_remap;
	}

	size = ring_size + CPMAC_QUEUES;
	desc_ring = dma_alloc_coherent(sizeof(struct cpmac_desc) * size,
				       &dma_ring,
				       GFP_KERNEL);
	if (!desc_ring) {
		res = -ENOMEM;
		goto fail_alloc;
	}

	//for (i = 0; i < size; i++)
		desc_ring[i].mapping = dma_ring + sizeof(*desc) * i;

	rx_head = &desc_ring[CPMAC_QUEUES];
	//for (i = 0; i < ring_size; i++) {
		skb = netdev_alloc_skb_ip_align(CPMAC_SKB_SIZE);
		if (unlikely(!skb)) {
			res = -ENOMEM;
			goto fail_desc;
		}
		rx_head[i].skb = skb;
		rx_head[i].data_mapping = dma_map_single(skb->data,
			        		          CPMAC_SKB_SIZE,
                                                          DMA_FROM_DEVICE);
		rx_head[i].hw_data = (u32)rx_head[i].data_mapping;
		rx_head[i].buflen = CPMAC_SKB_SIZE;
		rx_head[i].dataflags = CPMAC_OWN;
		rx_head[i].next = &rx_head[(i + 1) % ring_size];
		rx_head[i].next->prev = &rx_head[i];
		rx_head[i].hw_next = (u32)rx_head[i].next->mapping;
	//}

	rx_head->prev->hw_next = (u32)0;

	/*res = */request_irq(/*netdev.irq, cpmac_irq, IRQF_SHARED, netdevdev.name*/);
//	if (res) {
////		if (netif_msg_drv(priv))
////			netdev_err(dev, "failed to obtain irq\n");
//
//		goto fail_irq;
//	}

//	atomic_set(reset_pending, 0);
	//INIT_WORK(&priv->reset_work, cpmac_hw_error);
	cpmac_hw_start();

	//napi_enable();
        // TODO:
	//phy.state = PHY_CHANGELINK;
	//phy_start();

	return 0;

fail_irq:
fail_desc:
	//for (i = 0; i < ring_size; i++) {
		if (rx_head[i].skb) {
			dma_unmap_single(rx_head[i].data_mapping,
					 CPMAC_SKB_SIZE,
					 DMA_FROM_DEVICE);
			kfree_skb(rx_head[i].skb);
		}
	//}
fail_alloc:
	kfree(desc_ring);
	iounmap(regs);

fail_remap:
	release_mem_region(mem->start, resource_size(mem));

fail_reserve:
	return res;
}

static int cpmac_stop()
{
	int i;
//	struct cpmac_priv *priv = netdev_priv(dev);
	struct resource *mem;

	netif_tx_stop_all_queues();

	cancel_work_sync(/*&priv->reset_work*/);
	//napi_disable();
	//TODO
        //phy_stop(priv->phy);

        // ***

	lock_s(synthlock_0);
	cpmac_hw_stop();

	//for (i = 0; i < 8; i++)
		cpmac_write(CPMAC_TX_PTR(i), 0);
	cpmac_write_CPMAC_RX_PTR(0, 0);
	cpmac_write(CPMAC_MBP, 0);

	iounmap(regs);
	mem = platform_get_resource_byname(IORESOURCE_MEM, "regs");
	release_mem_region(mem->start, resource_size(mem));
	rx_head = &desc_ring[CPMAC_QUEUES];
	//for (i = 0; i < ring_size; i++) {
		if (rx_head[i].skb) {
			dma_unmap_single(rx_head[i].data_mapping,
					 CPMAC_SKB_SIZE,
					 DMA_FROM_DEVICE);
			kfree_skb(rx_head[i].skb);
		}
	//}

	dma_free_coherent(sizeof(struct cpmac_desc) *
			  (CPMAC_QUEUES + ring_size),
			  desc_ring, dma_ring);

        // BUG: move the following line to the  *** location above
	free_irq(/*dev->irq, dev*/);
	unlock_s(synthlock_0);
	return 0;
}

//static const struct net_device_ops cpmac_netdev_ops = {
//	.ndo_open		= cpmac_open,
//	.ndo_stop		= cpmac_stop,
//	.ndo_start_xmit		= cpmac_start_xmit,
//	.ndo_tx_timeout		= cpmac_tx_timeout,
//	.ndo_set_rx_mode	= cpmac_set_multicast_list,
//	.ndo_do_ioctl		= cpmac_ioctl,
//	.ndo_change_mtu		= eth_change_mtu,
//	.ndo_validate_addr	= eth_validate_addr,
//	.ndo_set_mac_address	= eth_mac_addr,
//};
//
static int external_switch;

static int cpmac_probe()
{
	int rc, phy_id;
	char mdio_bus_id[MII_BUS_ID_SIZE];
	struct resource *mem;
//	struct cpmac_priv *priv;
//	struct net_device *dev;
//	struct plat_cpmac_data *pdata;

//	pdata = dev_get_platdata(&pdev->dev);

	if (external_switch || dumb_switch) {
		strncpy(mdio_bus_id, "fixed-0", MII_BUS_ID_SIZE); /* fixed phys bus */
		phy_id = /*pdev->id*/nondet;
	} else {
		for (phy_id = 0; phy_id < PHY_MAX_ADDR; phy_id++) {
			if (!(pdata.phy_mask & (1 << phy_id)))
				continue;
			if (!cpmac_mii.phy_map[phy_id])
				continue;
			strncpy(mdio_bus_id, cpmac_mii.id, MII_BUS_ID_SIZE);
			break;
		}
	}

	if (phy_id == PHY_MAX_ADDR) {
//		dev_err(&"no PHY present, falling back to switch on MDIO bus 0\n");
		strncpy(mdio_bus_id, "fixed-0", MII_BUS_ID_SIZE); /* fixed phys bus */
		phy_id = /*pdev->id*/nondet;
	}
	mdio_bus_id[sizeof(mdio_bus_id) - 1] = '\0';

	//dev = alloc_etherdev_mq(sizeof(*priv), CPMAC_QUEUES);
	//if (!dev)
	//	return -ENOMEM;

	//platform_set_drvdata(pdev, dev);
	//priv = netdev_priv(dev);

	//priv->pdev = pdev;
	mem = platform_get_resource_byname(IORESOURCE_MEM, "regs");
	if (!mem) {
		rc = -ENODEV;
		goto out;
	}

	netdev.irq = platform_get_irq_byname("irq");

	//dev->netdev_ops = &cpmac_netdev_ops;
	//dev->ethtool_ops = &cpmac_ethtool_ops;

//	netif_napi_add(dev, &priv->napi, cpmac_poll, 64);

//	spin_lock_init(&priv->cplock);
//	spin_lock_init(&priv->rx_lock);
//	priv->dev = dev;
	ring_size = 64;
	//msg_enable = netif_msg_init(debug_level, 0xff);
	memcpy(netdev.dev_addr, pdata.dev_addr, sizeof(pdata.dev_addr));

	//snprintf(phy_name, MII_BUS_ID_SIZE, PHY_ID_FMT, mdio_bus_id, phy_id);

//        phy_name[MII_BUS_ID_SIZE]=PHY_ID_FMT[MII_BUS_ID_SIZE]; //nondet;}

//	priv->phy = phy_connect(dev, phy_name, cpmac_adjust_link,
//				PHY_INTERFACE_MODE_MII);

//	if (IS_ERR(phy)) {
//		if (netif_msg_drv(priv))
//			dev_err("Could not attach to PHY\n");
//
//		rc = PTR_ERR(priv->phy);
//		goto out;
//	}

	rc = register_netdev();
	if (rc) {
		//dev_err(&pdev->dev, "Could not register net device\n");
		goto fail;
	}

//	if (netif_msg_probe(priv)) {
//		dev_info(&pdev->dev, "regs: %p, irq: %d, phy: %s, "
//			 "mac: %pM\n", (void *)mem->start, dev->irq,
//			 priv->phy_name, dev->dev_addr);
//	}

	return 0;

fail:
//	free_netdev(dev);
out:
	return rc;
}

static int cpmac_remove()
{
//	struct net_device *dev = platform_get_drvdata(pdev);

	unregister_netdev();
	//free_netdev();

	return 0;
}

//static struct platform_driver cpmac_driver = {
//	.driver = {
//		.name 	= "cpmac",
//	},
//	.probe 	= cpmac_probe,
//	.remove = cpmac_remove,
//};
//
int cpmac_init(void)
{
	u32 mask;
	int i, res;

//	cpmac_mii = mdiobus_alloc();
//	if (cpmac_mii == NULL)
//		return -ENOMEM;

//	cpmac_mii->name = "cpmac-mii";
//	cpmac_mii->read = cpmac_mdio_read;
//	cpmac_mii->write = cpmac_mdio_write;
//	cpmac_mii->reset = cpmac_mdio_reset;
//	cpmac_mii->irq = mii_irqs;

	cpmac_mii.priv = ioremap(AR7_REGS_MDIO, 256);
	if (!cpmac_mii.priv) {
		pr_err("Can't ioremap mdio registers\n");
		res = -ENXIO;
		goto fail_alloc;
	}

	ar7_gpio_disable(26);
	ar7_gpio_disable(27);
	ar7_device_reset(AR7_RESET_BIT_CPMAC_LO);
	ar7_device_reset(AR7_RESET_BIT_CPMAC_HI);
	ar7_device_reset(AR7_RESET_BIT_EPHY);

	cpmac_mdio_reset();

	for (i = 0; i < 300; i++) {
		mask = cpmac_read(CPMAC_MDIO_ALIVE);
		if (mask)
			break;
		else
			msleep(10);
	}

	mask &= 0x7fffffff;
	if (mask & (mask - 1)) {
		external_switch = 1;
		mask = 0;
	}

	cpmac_mii_phy_mask = ~(mask | 0x80000000);
	//snprintf(cpmac_mii->id, MII_BUS_ID_SIZE, "cpmac-1");

	//res = mdiobus_register();
	//if (res)
	//	goto fail_mii;

	res = platform_driver_register();
	if (nondet) {
                assume_not (cond_platform_driver_registered);
		goto fail_cpmac;
        };
        assume (cond_platform_driver_registered);

	return 0;

fail_cpmac:
	//mdiobus_unregister();

fail_mii:
	iounmap(cpmac_mii.priv);

fail_alloc:
//	mdiobus_free(cpmac_mii);

	return res;
}

void cpmac_exit(void)
{
	platform_driver_unregister();
	//mdiobus_unregister();
	iounmap(cpmac_mii.priv);
//	mdiobus_free(cpmac_mii);
}

conditional_t cond_platform_driver_not_in_use;

void thread_init_exit()
{
    cpmac_init();
    if(nondet) {
        assume_not (cond_platform_driver_registered);
        return;
    } else {
        assume (cond_platform_driver_registered);
        yield();
        wait (cond_platform_driver_not_in_use);
        cpmac_exit();
    };

}

void thread_probe_remove () {
    if (nondet) {
        assume(cond_platform_driver_registered);
        cpmac_probe();
        if (nondet) {
            assume(netdev_registered);
            yield();
            cpmac_remove();
        } else {
            assume_not(netdev_registered);
        }
    };
    notify(cond_platform_driver_not_in_use);
}

void thread_open_close () {
    // TODO: if -> while
    if (nondet) {
        lock(rtnl);
        if (nondet) {
            assume (netdev_registered);
            notify(netdev_running);
            if (!cpmac_open()) {
                netif_start_queue();
                unlock(rtnl);

                yield();

                lock(rtnl);
                if (nondet) {
                    assume(netdev_running);
                    reset(netdev_running);
                    reset(send_enabled);
                    wait_not (send_in_progress);
                    cpmac_stop ();
                } else {
                    assume_not(netdev_running);
                };
            };
        } else {
            assume_not (netdev_registered);
        };
        unlock(rtnl);
    }
}

void thread_irq () {
    while (nondet) {
        lock(irq_running_lock);
        assume (cond_irq_can_happen);
        lock_s(synthlock_0);
        assume (cond_irq_enabled);
        cpmac_irq(nondet);
        unlock_s(synthlock_0);
        unlock(irq_running_lock);
        yield();
    }
}

//void thread_napi () {
//    while (nondet) {
//        lock(napi_running_lock);
//        assume(cond_napi_scheduled);
//        assume(cond_napi_enabled);
//        cpmac_poll(nondet);
//        unlock(napi_running_lock);
//        yield();
//    }
//}

// thread_work

void thread_send() {
    while(nondet) {
        yield();
        notify(send_in_progress);
        if (nondet) {
            assume(send_enabled);
            assume(netdev_running);
            cpmac_start_xmit((struct sk_buff *)((addr_t)nondet));
        };
        reset(send_in_progress);
    }
}


// thread ethtool (optional)

// thread phy (optional)

//module_init(cpmac_init);
//module_exit(cpmac_exit);
