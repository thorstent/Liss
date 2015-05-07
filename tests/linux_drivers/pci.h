#ifndef _PCI_H_
#define _PCI_H_

#include "types.h"
#include "langinc.h"

#define PCI_VENDOR_ID           0x00    /* 16 bits */
#define PCI_DEVICE_ID           0x02    /* 16 bits */
#define PCI_COMMAND             0x04    /* 16 bits */
#define  PCI_COMMAND_IO         0x1     /* Enable response in I/O space */
#define  PCI_COMMAND_MEMORY     0x2     /* Enable response in Memory space */
#define  PCI_COMMAND_MASTER     0x4     /* Enable bus mastering */
#define  PCI_COMMAND_SPECIAL    0x8     /* Enable response to special cycles */
#define  PCI_COMMAND_INVALIDATE 0x10    /* Use memory write and invalidate */
#define  PCI_COMMAND_VGA_PALETTE 0x20   /* Enable palette snooping */
#define  PCI_COMMAND_PARITY     0x40    /* Enable parity checking */
#define  PCI_COMMAND_WAIT       0x80    /* Enable address/data stepping */
#define  PCI_COMMAND_SERR       0x100   /* Enable SERR */
#define  PCI_COMMAND_FAST_BACK  0x200   /* Enable back-to-back writes */
#define  PCI_COMMAND_INTX_DISABLE 0x400 /* INTx Emulation Disable */

#define PCI_STATUS              0x06    /* 16 bits */
#define  PCI_STATUS_INTERRUPT   0x08    /* Interrupt status */
#define  PCI_STATUS_CAP_LIST    0x10    /* Support Capability List */
#define  PCI_STATUS_66MHZ       0x20    /* Support 66 Mhz PCI 2.1 bus */
#define  PCI_STATUS_UDF         0x40    /* Support User Definable Features [obsolete] */
#define  PCI_STATUS_FAST_BACK   0x80    /* Accept fast-back to back */
#define  PCI_STATUS_PARITY      0x100   /* Detected parity error */
#define  PCI_STATUS_DEVSEL_MASK 0x600   /* DEVSEL timing */
#define  PCI_STATUS_DEVSEL_FAST         0x000
#define  PCI_STATUS_DEVSEL_MEDIUM       0x200
#define  PCI_STATUS_DEVSEL_SLOW         0x400
#define  PCI_STATUS_SIG_TARGET_ABORT    0x800 /* Set on target abort */
#define  PCI_STATUS_REC_TARGET_ABORT    0x1000 /* Master ack of " */
#define  PCI_STATUS_REC_MASTER_ABORT    0x2000 /* Set on master abort */
#define  PCI_STATUS_SIG_SYSTEM_ERROR    0x4000 /* Set when we drive SERR */
#define  PCI_STATUS_DETECTED_PARITY     0x8000 /* Set on parity error */

#define PCI_CLASS_REVISION      0x08    /* High 24 bits are class, low 8 revision */
#define PCI_REVISION_ID         0x08    /* Revision ID */
#define PCI_CLASS_PROG          0x09    /* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE        0x0a    /* Device class */

#define PCI_CACHE_LINE_SIZE     0x0c    /* 8 bits */
#define PCI_LATENCY_TIMER       0x0d    /* 8 bits */
#define PCI_HEADER_TYPE         0x0e    /* 8 bits */
#define  PCI_HEADER_TYPE_NORMAL         0
#define  PCI_HEADER_TYPE_BRIDGE         1
#define  PCI_HEADER_TYPE_CARDBUS        2

#define PCI_BIST                0x0f    /* 8 bits */
#define  PCI_BIST_CODE_MASK     0x0f    /* Return result */
#define  PCI_BIST_START         0x40    /* 1 to start BIST, 2 secs or less */
#define  PCI_BIST_CAPABLE       0x80    /* 1 if BIST capable */


int pci_write_config_byte(int where, u8 val)
{
    ioval = where+val;
    return ioval;
}

int pci_enable_msi() {
    ioval = true;
    return (int) ioval;
};

void pci_disable_msi() {
    ioval = true;
}


#define PCIE_LINK_STATE_L0S     1
#define PCIE_LINK_STATE_L1      2
#define PCIE_LINK_STATE_CLKPM   4

void pci_disable_link_state(int state) {
    ioval = state;
}

int pci_enable_device() 
{
    ioval = 1;
    return ioval;
}

void pci_disable_device()
{
    ioval = 1;
}

int pci_resource_flags(int flags) {
    return ioval | flags;
}

#define pci_resource_start(bar) (ioval|bar)
#define pci_resource_len(bar)   (ioval|bar)

int pci_request_regions(const char *res_name)
{
    ioval = (u64) res_name;
    return ioval;
}

void pci_release_regions()
{
    ioval = 1;
}

int pci_set_dma_mask(u64 mask)
{
    ioval = mask;
    return ioval;
}

bool pci_is_pcie() 
{
    return ioval;
}    

void pci_set_master() 
{
    ioval = true;
};


bool pci_dev_run_wake() 
{
    return (bool)ioval;
};

conditional_t pm_up_request;
conditional_t pm_up;
conditional_t pm_request;

void pm_runtime_put_noidle()
{
    reset(pm_up_request);
    notify(pm_request);
    wait_not(pm_request);
};

int pm_runtime_get_sync() 
{
    notify(pm_up_request);
    notify(pm_request);
    wait_not(pm_request);
    wait(pm_up);
    // TODO: deal with errors
    return 0;
}

int pm_runtime_get_noresume() 
{
    notify(pm_up_request);
    notify(pm_request);
    wait_not(pm_request);
    wait(pm_up);
    return 0;
}

void pm_runtime_put_sync() 
{
    reset(pm_up_request);
    notify(pm_request);
    wait_not(pm_request);
}

int pm_request_resume()
{
    notify(pm_up_request);
    notify(pm_request);
    wait_not(pm_request);
    return 0;
}

int pm_schedule_suspend(unsigned int delay)
{
    reset(pm_up_request);
    notify(pm_request);
    wait_not(pm_request);
    return 0;
}

int pci_set_mwi() 
{
    ioval = 1;
    return ioval;
}

void pci_clear_mwi()
{
    ioval = 1;
}


#endif // _PCI_H_
