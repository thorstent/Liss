#ifndef _ETH_H_
#define _ETH_H_

#include "langinc.h"
#include "types.h"

#define ETH_DATA_LEN    1500            /* Max. octets in payload        */
#define ETH_HLEN        14              /* Total octets in header.       */
#define ETH_ALEN        6               /* Octets in one ethernet addr   */

#define ETH_ZLEN        60              /* Min. octets in frame sans FCS */
#define ETH_FRAME_LEN   1514            /* Max. octets in frame sans FCS */
#define ETH_FCS_LEN     4               /* Octets in the FCS             */

#define VLAN_HLEN       4               /* The additional bytes required by VLAN
                                         * (in addition to the Ethernet header)
                                         */
#define VLAN_ETH_HLEN   18              /* Total octets in header.       */
#define VLAN_ETH_ZLEN   64              /* Min. octets in frame sans FCS */

/*
 * According to 802.3ac, the packet can be 4 bytes longer. --Klika Jan
 */
#define VLAN_ETH_DATA_LEN       1500    /* Max. octets in payload        */
#define VLAN_ETH_FRAME_LEN      1518    /* Max. octets in frame sans FCS */

// models data received by the OS from the driver
int rx_val;

struct sk_buff {
    unsigned int len;
    u16       protocol;
};

static inline struct sk_buff *netdev_alloc_skb_ip_align(unsigned int length)
{
         return (struct sk_buff*) (addr_t) nondet;
}


u16 eth_type_trans(struct sk_buff *skb){
    return nondet;
};

static inline void skb_checksum_none_assert(const struct sk_buff *skb)
{}

int netif_receive_skb(const struct sk_buff *skb){
    rx_val = nondet;
    return nondet;
};


#define skb_padto(_skb,_pad) ({_skb = (struct sk_buff*)(addr_t)nondet; nondet;})

#define dev_kfree_skb_any(_s) {_s=(struct sk_buff*)(addr_t)nondet;}
#define dev_kfree_skb_irq(_s) {_s=(struct sk_buff*)(addr_t)nondet;}
#define kfree_skb(_s) {_s=(struct sk_buff*)(addr_t)nondet;}
#define skb_put(_s,_l) {_s=(struct sk_buff*)(addr_t)(nondet+(_l));}

#define net_ratelimit() true
#define netif_level(level, type, fmt, args...)


#define netif_emerg(type, fmt, args...)              \
        netif_level(emerg, type, fmt, ##args)
#define netif_alert(type, fmt, args...)              \
        netif_level(alert, priv, type, dev, fmt, ##args)
#define netif_crit(type, fmt, args...)               \
        netif_level(crit, type, fmt, ##args)
#define netif_err(type, fmt, args...)                \
        netif_level(err, type, fmt, ##args)
#define netif_warn(type, fmt, args...)               \
        netif_level(warn, type, fmt, ##args)
#define netif_notice(type, fmt, args...)             \
        netif_level(notice, type, fmt, ##args)
#define netif_info(type, fmt, args...)               \
        netif_level(info, type, fmt, ##args)

#define IFNAMSIZ        16


struct net_device_stats {
        unsigned long   rx_packets;
        unsigned long   tx_packets;
        unsigned long   rx_bytes;
        unsigned long   tx_bytes;
        unsigned long   rx_errors;
        unsigned long   tx_errors;
        unsigned long   rx_dropped;
        unsigned long   tx_dropped;
        unsigned long   multicast;
        unsigned long   collisions;
        unsigned long   rx_length_errors;
        unsigned long   rx_over_errors;
        unsigned long   rx_crc_errors;
        unsigned long   rx_frame_errors;
        unsigned long   rx_fifo_errors;
        unsigned long   rx_missed_errors;
        unsigned long   tx_aborted_errors;
        unsigned long   tx_carrier_errors;
        unsigned long   tx_fifo_errors;
        unsigned long   tx_heartbeat_errors;
        unsigned long   tx_window_errors;
        unsigned long   rx_compressed;
        unsigned long   tx_compressed;
};


struct net_device {
    char name[IFNAMSIZ];
    int irq;
    char dev_addr[6];
    struct net_device_stats stats;
} netdev;

struct mii_if_info {
        int phy_id;
        int advertising;
        int phy_id_mask;
        int reg_num_mask;

        unsigned int full_duplex : 1;   /* is full duplex? */
        unsigned int force_media : 1;   /* is autoneg. disabled? */
        unsigned int supports_gmii : 1; /* are GMII registers supported? */

//        struct net_device *dev;
//        int (*mdio_read) (struct net_device *dev, int phy_id, int location);
//        void (*mdio_write) (struct net_device *dev, int phy_id, int location, int val);
};

#define MII_BUS_ID_SIZE (20 - 3)
#define PHY_MAX_ADDR    32
#define PHY_ID_FMT "%s:%02x"

struct phy_device {};

struct mii_bus {
        const char *name;
        char id[MII_BUS_ID_SIZE];
        void *priv;
        //int (*read)(struct mii_bus *bus, int phy_id, int regnum);
        //int (*write)(struct mii_bus *bus, int phy_id, int regnum, u16 val);
        //int (*reset)(struct mii_bus *bus);

        /*
         * A lock to ensure that only one thing can read/write
         * the MDIO bus at a time
         */
        //struct mutex mdio_lock;

        //struct device *parent;
        enum {
                MDIOBUS_ALLOCATED = 1,
                MDIOBUS_REGISTERED,
                MDIOBUS_UNREGISTERED,
                MDIOBUS_RELEASED,
        } state;
        //struct device dev;

        /* list of all PHYs on bus */
        struct phy_device *phy_map[PHY_MAX_ADDR];

        /* PHY addresses to be ignored when probing */
        u32 phy_mask;

        /*
         * Pointer to an array of interrupts, each PHY's
         * interrupt at the index matching its address
         */
        int *irq;
};

#define MAX_ADDR_LEN    32              /* Largest hardware address length */

struct netdev_hw_addr {
        unsigned char           addr[MAX_ADDR_LEN];
        unsigned char           type;
#define NETDEV_HW_ADDR_T_LAN            1
#define NETDEV_HW_ADDR_T_SAN            2
#define NETDEV_HW_ADDR_T_SLAVE          3
#define NETDEV_HW_ADDR_T_UNICAST        4
#define NETDEV_HW_ADDR_T_MULTICAST      5
        bool                    global_use;
        int                     sync_cnt;
        int                     refcount;
        int                     synced;
};

struct netdev_hw_addr_list {
         int                     count;
         struct netdev_hw_addr   *addr;
};



typedef u64 netdev_features_t;

netdev_features_t               netdev_features;
netdev_features_t               netdev_hw_features;
netdev_features_t               netdev_vlan_features;
unsigned int                    netdev_flags;
unsigned char                   *netdev_addr;
int                             netdev_watchdog_timeo;
struct netdev_hw_addr_list      netdev_mc;

void netdev_reset_queue() 
{
        ioval = true;
}

int netdev_mc_count() 
{
        return netdev_mc.count;
}

#define netdev_for_each_mc_addr(ha) for(ha=netdev_mc.addr; ha!=NULL; ha++)

enum {
        NETIF_F_SG_BIT,                 /* Scatter/gather IO. */
        NETIF_F_IP_CSUM_BIT,            /* Can checksum TCP/UDP over IPv4. */
        __UNUSED_NETIF_F_1,
        NETIF_F_HW_CSUM_BIT,            /* Can checksum all the packets. */
        NETIF_F_IPV6_CSUM_BIT,          /* Can checksum TCP/UDP over IPV6 */
        NETIF_F_HIGHDMA_BIT,            /* Can DMA to high memory. */
        NETIF_F_FRAGLIST_BIT,           /* Scatter/gather IO. */
        NETIF_F_HW_VLAN_CTAG_TX_BIT,    /* Transmit VLAN CTAG HW acceleration */
        NETIF_F_HW_VLAN_CTAG_RX_BIT,    /* Receive VLAN CTAG HW acceleration */
        NETIF_F_HW_VLAN_CTAG_FILTER_BIT,/* Receive filtering on VLAN CTAGs */
        NETIF_F_VLAN_CHALLENGED_BIT,    /* Device cannot handle VLAN packets */
        NETIF_F_GSO_BIT,                /* Enable software GSO. */
        NETIF_F_LLTX_BIT,               /* LockLess TX - deprecated. Please */
                                        /* do not use LLTX in new drivers */
        NETIF_F_NETNS_LOCAL_BIT,        /* Does not change network namespaces */
        NETIF_F_GRO_BIT,                /* Generic receive offload */
        NETIF_F_LRO_BIT,                /* large receive offload */

        /**/NETIF_F_GSO_SHIFT,          /* keep the order of SKB_GSO_* bits */
        NETIF_F_TSO_BIT                 /* ... TCPv4 segmentation */
                = NETIF_F_GSO_SHIFT,
        NETIF_F_UFO_BIT,                /* ... UDPv4 fragmentation */
        NETIF_F_GSO_ROBUST_BIT,         /* ... ->SKB_GSO_DODGY */
        NETIF_F_TSO_ECN_BIT,            /* ... TCP ECN support */
        NETIF_F_TSO6_BIT,               /* ... TCPv6 segmentation */
        NETIF_F_FSO_BIT,                /* ... FCoE segmentation */
        NETIF_F_GSO_GRE_BIT,            /* ... GRE with TSO */
        NETIF_F_GSO_GRE_CSUM_BIT,       /* ... GRE with csum with TSO */
        NETIF_F_GSO_IPIP_BIT,           /* ... IPIP tunnel with TSO */
        NETIF_F_GSO_SIT_BIT,            /* ... SIT tunnel with TSO */
        NETIF_F_GSO_UDP_TUNNEL_BIT,     /* ... UDP TUNNEL with TSO */
        NETIF_F_GSO_UDP_TUNNEL_CSUM_BIT,/* ... UDP TUNNEL with TSO & CSUM */
        NETIF_F_GSO_MPLS_BIT,           /* ... MPLS segmentation */
        /**/NETIF_F_GSO_LAST =          /* last bit, see GSO_MASK */
                NETIF_F_GSO_MPLS_BIT,

        NETIF_F_FCOE_CRC_BIT,           /* FCoE CRC32 */
        NETIF_F_SCTP_CSUM_BIT,          /* SCTP checksum offload */
        NETIF_F_FCOE_MTU_BIT,           /* Supports max FCoE MTU, 2158 bytes*/
        NETIF_F_NTUPLE_BIT,             /* N-tuple filters supported */
        NETIF_F_RXHASH_BIT,             /* Receive hashing offload */
        NETIF_F_RXCSUM_BIT,             /* Receive checksumming offload */
        NETIF_F_NOCACHE_COPY_BIT,       /* Use no-cache copyfromuser */
        NETIF_F_LOOPBACK_BIT,           /* Enable loopback */
        NETIF_F_RXFCS_BIT,              /* Append FCS to skb pkt data */
        NETIF_F_RXALL_BIT,              /* Receive errored frames too */
        NETIF_F_HW_VLAN_STAG_TX_BIT,    /* Transmit VLAN STAG HW acceleration */
        NETIF_F_HW_VLAN_STAG_RX_BIT,    /* Receive VLAN STAG HW acceleration */
        NETIF_F_HW_VLAN_STAG_FILTER_BIT,/* Receive filtering on VLAN STAGs */
        NETIF_F_HW_L2FW_DOFFLOAD_BIT,   /* Allow L2 Forwarding in Hardware */
        NETIF_F_BUSY_POLL_BIT,          /* Busy poll */

        /*
         * Add your fresh new feature above and remember to update
         * netdev_features_strings[] in net/core/ethtool.c and maybe
         * some feature mask #defines below. Please also describe it
         * in Documentation/networking/netdev-features.txt.
         */

        /**/NETDEV_FEATURE_COUNT
};

/* copy'n'paste compression ;) */
#define __NETIF_F_BIT(bit)      ((netdev_features_t)1 << (bit))
#define __NETIF_F(name)         __NETIF_F_BIT(NETIF_F_##name##_BIT)

#define NETIF_F_FCOE_CRC        __NETIF_F(FCOE_CRC)
#define NETIF_F_FCOE_MTU        __NETIF_F(FCOE_MTU)
#define NETIF_F_FRAGLIST        __NETIF_F(FRAGLIST)
#define NETIF_F_FSO             __NETIF_F(FSO)
#define NETIF_F_GRO             __NETIF_F(GRO)
#define NETIF_F_GSO             __NETIF_F(GSO)
#define NETIF_F_GSO_ROBUST      __NETIF_F(GSO_ROBUST)
#define NETIF_F_HIGHDMA         __NETIF_F(HIGHDMA)
#define NETIF_F_HW_CSUM         __NETIF_F(HW_CSUM)
#define NETIF_F_HW_VLAN_CTAG_FILTER __NETIF_F(HW_VLAN_CTAG_FILTER)
#define NETIF_F_HW_VLAN_CTAG_RX __NETIF_F(HW_VLAN_CTAG_RX)
#define NETIF_F_HW_VLAN_CTAG_TX __NETIF_F(HW_VLAN_CTAG_TX)
#define NETIF_F_IP_CSUM         __NETIF_F(IP_CSUM)
#define NETIF_F_IPV6_CSUM       __NETIF_F(IPV6_CSUM)
#define NETIF_F_LLTX            __NETIF_F(LLTX)
#define NETIF_F_LOOPBACK        __NETIF_F(LOOPBACK)
#define NETIF_F_LRO             __NETIF_F(LRO)
#define NETIF_F_NETNS_LOCAL     __NETIF_F(NETNS_LOCAL)
#define NETIF_F_NOCACHE_COPY    __NETIF_F(NOCACHE_COPY)
#define NETIF_F_NTUPLE          __NETIF_F(NTUPLE)
#define NETIF_F_RXCSUM          __NETIF_F(RXCSUM)
#define NETIF_F_RXHASH          __NETIF_F(RXHASH)
#define NETIF_F_SCTP_CSUM       __NETIF_F(SCTP_CSUM)
#define NETIF_F_SG              __NETIF_F(SG)
#define NETIF_F_TSO6            __NETIF_F(TSO6)
#define NETIF_F_TSO_ECN         __NETIF_F(TSO_ECN)
#define NETIF_F_TSO             __NETIF_F(TSO)
#define NETIF_F_UFO             __NETIF_F(UFO)
#define NETIF_F_VLAN_CHALLENGED __NETIF_F(VLAN_CHALLENGED)
#define NETIF_F_RXFCS           __NETIF_F(RXFCS)
#define NETIF_F_RXALL           __NETIF_F(RXALL)
#define NETIF_F_GSO_GRE         __NETIF_F(GSO_GRE)
#define NETIF_F_GSO_GRE_CSUM    __NETIF_F(GSO_GRE_CSUM)
#define NETIF_F_GSO_IPIP        __NETIF_F(GSO_IPIP)
#define NETIF_F_GSO_SIT         __NETIF_F(GSO_SIT)
#define NETIF_F_GSO_UDP_TUNNEL  __NETIF_F(GSO_UDP_TUNNEL)
#define NETIF_F_GSO_UDP_TUNNEL_CSUM __NETIF_F(GSO_UDP_TUNNEL_CSUM)
#define NETIF_F_GSO_MPLS        __NETIF_F(GSO_MPLS)
#define NETIF_F_HW_VLAN_STAG_FILTER __NETIF_F(HW_VLAN_STAG_FILTER)
#define NETIF_F_HW_VLAN_STAG_RX __NETIF_F(HW_VLAN_STAG_RX)
#define NETIF_F_HW_VLAN_STAG_TX __NETIF_F(HW_VLAN_STAG_TX)
#define NETIF_F_HW_L2FW_DOFFLOAD        __NETIF_F(HW_L2FW_DOFFLOAD)
#define NETIF_F_BUSY_POLL       __NETIF_F(BUSY_POLL)

enum net_device_flags {
        IFF_UP                          = 1<<0,  /* sysfs */
        IFF_BROADCAST                   = 1<<1,  /* volatile */
        IFF_DEBUG                       = 1<<2,  /* sysfs */
        IFF_LOOPBACK                    = 1<<3,  /* volatile */
        IFF_POINTOPOINT                 = 1<<4,  /* volatile */
        IFF_NOTRAILERS                  = 1<<5,  /* sysfs */
        IFF_RUNNING                     = 1<<6,  /* volatile */
        IFF_NOARP                       = 1<<7,  /* sysfs */
        IFF_PROMISC                     = 1<<8,  /* sysfs */
        IFF_ALLMULTI                    = 1<<9,  /* sysfs */
        IFF_MASTER                      = 1<<10, /* volatile */
        IFF_SLAVE                       = 1<<11, /* volatile */
        IFF_MULTICAST                   = 1<<12, /* sysfs */
        IFF_PORTSEL                     = 1<<13, /* sysfs */
        IFF_AUTOMEDIA                   = 1<<14, /* sysfs */
        IFF_DYNAMIC                     = 1<<15, /* sysfs */
        IFF_LOWER_UP                    = 1<<16, /* volatile */
        IFF_DORMANT                     = 1<<17, /* volatile */
        IFF_ECHO                        = 1<<18, /* volatile */
};

enum netdev_tx {
        __NETDEV_TX_MIN  = INT_MIN,     /* make sure enum is signed */
        NETDEV_TX_OK     = 0x00,        /* driver took care of packet */
        NETDEV_TX_BUSY   = 0x10,        /* driver tx path was busy*/
        NETDEV_TX_LOCKED = 0x20,        /* driver tx lock was already taken */
};

static inline bool is_zero_ether_addr(const u8 *addr)
{
        return ((*(const u32 *)addr) | (*(const u16 *)(addr + 4))) == 0;
}


bool is_multicast_ether_addr(const u8 *addr)
{
        return 0x01 & addr[0];
}


bool is_valid_ether_addr(const u8 *addr)
{
    /* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
     * explicitly check for it here. */
    return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}

lock_t rtnl;
conditional_t netdev_registered;  // after register_netdev
conditional_t netdev_running;     // after open()
conditional_t send_enabled;       // after netif_start_queue
conditional_t send_in_progress;   // locked when calling start_xmit


int register_netdev()
{
//    int ret;

    //lock(rtnl);
    if (nondet) {
        notify(netdev_registered);
        //unlock(rtnl);
        return 0;
    } else {
        //unlock(rtnl);
        return -1;
    };
//    return ret;
}


static int cpmac_stop();

void unregister_netdev()
{
//    int ret;

    lock(rtnl);
//    wait_not(netdev_running);

    reset (netdev_registered);
    if (nondet) {
        assume(netdev_running);
        reset(netdev_running);
        reset(send_enabled);
        wait_not (send_in_progress);
        cpmac_stop ();
    } else {
        assume_not(netdev_running);
    };
    unlock(rtnl);
//    return ret;
}


//#define netif_running()  
//    // TODO: return netdev_running

void netif_carrier_off() 
{
    //ioval = 1;
}

void netif_carrier_on() 
{
    ioval = 1;
}

void netif_start_queue()
{
    notify(send_enabled);
};

void netif_stop_queue()
{
//    lock(send_in_progress);
    reset(send_enabled);
    //wait_not (send_in_progress);
//    unlock(send_in_progress);
};

void netif_stop_subqueue(/*queue*/)
{
    netif_stop_queue();
};

void netif_wake_subqueue(/*queue*/)
{
    notify(send_enabled);
};


void netif_tx_stop_all_queues()
{
    netif_stop_queue();
};

void netif_device_detach()
{
    // TODO
};

void netif_device_attach()
{
    // TODO
}

struct napi_struct{};

lock_t napi_running_lock;
conditional_t cond_napi_enabled;
conditional_t cond_napi_scheduled;

void napi_enable()
{
    notify(cond_napi_enabled);
}

void napi_disable()
{
    reset (cond_napi_enabled);
    lock(napi_running_lock);
    unlock(napi_running_lock);
}

void napi_complete () 
{
    reset (cond_napi_scheduled);
}

void __napi_schedule()
{
    notify(cond_napi_scheduled);
};


#define MII_BMCR            0x00        /* Basic mode control register */
#define MII_BMSR            0x01        /* Basic mode status register  */
#define MII_PHYSID1         0x02        /* PHYS ID 1                   */
#define MII_PHYSID2         0x03        /* PHYS ID 2                   */
#define MII_ADVERTISE       0x04        /* Advertisement control reg   */
#define MII_LPA             0x05        /* Link partner ability reg    */
#define MII_EXPANSION       0x06        /* Expansion register          */
#define MII_CTRL1000        0x09        /* 1000BASE-T control          */
#define MII_STAT1000        0x0a        /* 1000BASE-T status           */
#define MII_ESTATUS         0x0f        /* Extended Status */
#define MII_DCOUNTER        0x12        /* Disconnect counter          */
#define MII_FCSCOUNTER      0x13        /* False carrier counter       */
#define MII_NWAYTEST        0x14        /* N-way auto-neg test reg     */
#define MII_RERRCOUNTER     0x15        /* Receive error counter       */
#define MII_SREVISION       0x16        /* Silicon revision            */
#define MII_RESV1           0x17        /* Reserved...                 */
#define MII_LBRERROR        0x18        /* Lpback, rx, bypass error    */
#define MII_PHYADDR         0x19        /* PHY address                 */
#define MII_RESV2           0x1a        /* Reserved...                 */
#define MII_TPISTATUS       0x1b        /* TPI status for 10mbps       */
#define MII_NCONFIG         0x1c        /* Network interface config    */

/* Basic mode control register. */
#define BMCR_RESV               0x003f  /* Unused...                   */
#define BMCR_SPEED1000          0x0040  /* MSB of Speed (1000)         */
#define BMCR_CTST               0x0080  /* Collision test              */
#define BMCR_FULLDPLX           0x0100  /* Full duplex                 */
#define BMCR_ANRESTART          0x0200  /* Auto negotiation restart    */
#define BMCR_ISOLATE            0x0400  /* Disconnect DP83840 from MII */
#define BMCR_PDOWN              0x0800  /* Powerdown the DP83840       */
#define BMCR_ANENABLE           0x1000  /* Enable auto negotiation     */
#define BMCR_SPEED100           0x2000  /* Select 100Mbps              */
#define BMCR_LOOPBACK           0x4000  /* TXD loopback bits           */
#define BMCR_RESET              0x8000  /* Reset the DP83840           */

/* Basic mode status register. */
#define BMSR_ERCAP              0x0001  /* Ext-reg capability          */
#define BMSR_JCD                0x0002  /* Jabber detected             */
#define BMSR_LSTATUS            0x0004  /* Link status                 */
#define BMSR_ANEGCAPABLE        0x0008  /* Able to do auto-negotiation */
#define BMSR_RFAULT             0x0010  /* Remote fault detected       */
#define BMSR_ANEGCOMPLETE       0x0020  /* Auto-negotiation complete   */
#define BMSR_RESV               0x00c0  /* Unused...                   */
#define BMSR_ESTATEN            0x0100  /* Extended Status in R15 */
#define BMSR_100HALF2           0x0200  /* Can do 100BASE-T2 HDX */
#define BMSR_100FULL2           0x0400  /* Can do 100BASE-T2 FDX */
#define BMSR_10HALF             0x0800  /* Can do 10mbps, half-duplex  */
#define BMSR_10FULL             0x1000  /* Can do 10mbps, full-duplex  */
#define BMSR_100HALF            0x2000  /* Can do 100mbps, half-duplex */
#define BMSR_100FULL            0x4000  /* Can do 100mbps, full-duplex */
#define BMSR_100BASE4           0x8000  /* Can do 100mbps, 4k packets  */

/* Enable or disable autonegotiation. */
#define AUTONEG_DISABLE         0x00
#define AUTONEG_ENABLE          0x01

/* Advertisement control register. */
#define ADVERTISE_SLCT          0x001f  /* Selector bits               */
#define ADVERTISE_CSMA          0x0001  /* Only selector supported     */
#define ADVERTISE_10HALF        0x0020  /* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL     0x0020  /* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL        0x0040  /* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF     0x0040  /* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF       0x0080  /* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE    0x0080  /* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL       0x0100  /* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM 0x0100  /* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4      0x0200  /* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP     0x0400  /* Try for pause               */
#define ADVERTISE_PAUSE_ASYM    0x0800  /* Try for asymetric pause     */
#define ADVERTISE_RESV          0x1000  /* Unused...                   */
#define ADVERTISE_RFAULT        0x2000  /* Say we can detect faults    */
#define ADVERTISE_LPACK         0x4000  /* Ack link partners response  */
#define ADVERTISE_NPAGE         0x8000  /* Next page bit               */

#define ADVERTISE_FULL          (ADVERTISE_100FULL | ADVERTISE_10FULL | \
                                  ADVERTISE_CSMA)
#define ADVERTISE_ALL           (ADVERTISE_10HALF | ADVERTISE_10FULL | \
                                  ADVERTISE_100HALF | ADVERTISE_100FULL)

#define ADVERTISE_1000FULL      0x0200  /* Advertise 1000BASE-T full duplex */
#define ADVERTISE_1000HALF      0x0100  /* Advertise 1000BASE-T half duplex */

#define ADVERTISED_10baseT_Half         (1 << 0)
#define ADVERTISED_10baseT_Full         (1 << 1)
#define ADVERTISED_100baseT_Half        (1 << 2)
#define ADVERTISED_100baseT_Full        (1 << 3)
#define ADVERTISED_1000baseT_Half       (1 << 4)
#define ADVERTISED_1000baseT_Full       (1 << 5)
#define ADVERTISED_Autoneg              (1 << 6)
#define ADVERTISED_TP                   (1 << 7)
#define ADVERTISED_AUI                  (1 << 8)
#define ADVERTISED_MII                  (1 << 9)
#define ADVERTISED_FIBRE                (1 << 10)
#define ADVERTISED_BNC                  (1 << 11)
#define ADVERTISED_10000baseT_Full      (1 << 12)
#define ADVERTISED_Pause                (1 << 13)
#define ADVERTISED_Asym_Pause           (1 << 14)
#define ADVERTISED_2500baseX_Full       (1 << 15)
#define ADVERTISED_Backplane            (1 << 16)
#define ADVERTISED_1000baseKX_Full      (1 << 17)
#define ADVERTISED_10000baseKX4_Full    (1 << 18)
#define ADVERTISED_10000baseKR_Full     (1 << 19)
#define ADVERTISED_10000baseR_FEC       (1 << 20)
#define ADVERTISED_20000baseMLD2_Full   (1 << 21)
#define ADVERTISED_20000baseKR2_Full    (1 << 22)
#define ADVERTISED_40000baseKR4_Full    (1 << 23)
#define ADVERTISED_40000baseCR4_Full    (1 << 24)
#define ADVERTISED_40000baseSR4_Full    (1 << 25)
#define ADVERTISED_40000baseLR4_Full    (1 << 26)

#define SPEED_10                10
#define SPEED_100               100
#define SPEED_1000              1000
#define SPEED_2500              2500
#define SPEED_10000             10000
#define SPEED_UNKNOWN           -1

#define DUPLEX_HALF             0x00
#define DUPLEX_FULL             0x01
#define DUPLEX_UNKNOWN          0xff

#define ether_crc(length, data)    (length + *(int*)data)

#endif // _ETH_H_
