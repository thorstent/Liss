#include "langinc.h"

int fw_tty_registered;
int fw_tty_initialized;
lock_t fw_tty_lock;
int fw_driver_list_consistent;
int fw_idr_consistent;
lock_t fw_table_lock;
lock_t fw_serial_bus_lock;
conditional_t drv_usb_registered;
int drv_usb_initialized;
int drv_registered_with_usb_fw;
conditional_t drv_registered_with_serial_fw;
int drv_id_table;
conditional_t drv_module_ref_cnt;
int drv_device_id_registered;
int dev_usb_serial_initialized;
int dev_autopm;
conditional_t dev_disconnected;
lock_t dev_disc_lock;
conditional_t port_dev_registered;
int port_initialized;
conditional_t port_idr_registered;
conditional_t port_tty_installed;
int port_tty_state;
conditional_t port_work;
int port_work_initialized;
int port_work_stop;
conditional_t port_tty_registered;
int port_consistent;
lock_t port_lock;
conditional_t port_write_in_progress;
conditional_t write_urb_submitted;

void try_module_get()
void usb_serial_put()
void usb_serial_init() {
    fw_tty_initialized = 1;
    fw_tty_registered = 1;
}


void usb_serial_exit() {
    fw_tty_registered = 0;
    int_wait_not(port_tty_installed);
    fw_tty_initialized = 0;
}


void lock_table() {
    int_lock(fw_table_lock);
}


void unlock_table() {
    int_unlock(fw_table_lock);
}


void lock_serial_bus() {
    int_lock(fw_serial_bus_lock);
}


void unlock_serial_bus() {
    int_unlock(fw_serial_bus_lock);
}


void lock_tty() {
    int_lock(fw_tty_lock);
}


void unlock_tty() {
    int_unlock(fw_tty_lock);
}


void lock_port() {
    int_lock(port_lock);
}


void unlock_port() {
    int_unlock(port_lock);
}


void lock_disc() {
    int_lock(dev_disc_lock);
}


void unlock_disc() {
    int_unlock(dev_disc_lock);
}


void usb_serial_probe() {
    int x;
    lock_table();
    x = fw_driver_list_consistent;
    if ((!drv_registered_with_usb_fw)) {
        unlock_table();
        return;
    } else {
        try_module_get();
        if (nondet_int) {
            int_assume_not(drv_module_ref_cnt);
            unlock_table();
            return;
        } else {
            int_assume(drv_module_ref_cnt);
            x = drv_usb_initialized;
            unlock_table();
            dev_usb_serial_initialized = 1;
            int_notify(dev_disconnected);
            {
                lock_table();
                x = fw_idr_consistent;
                fw_idr_consistent = 0;
                int_notify(port_idr_registered);
                fw_idr_consistent = 1;
                unlock_table();
            }
            port_work_initialized = 1;
            port_initialized = 1;
            int_notify(port_dev_registered);
            int_reset(dev_disconnected);
        }
    }
}


void usb_serial_disconnect() {
    int x;
    lock_disc();
    int_notify(dev_disconnected);
    unlock_disc();
    x = dev_usb_serial_initialized;
    x = port_initialized;
    if (nondet_int) {
        int_assume(port_tty_installed);
        lock_tty();
        x = dev_usb_serial_initialized;
        x = port_initialized;
        int_assume_not(port_write_in_progress);
        port_tty_state = 0;
        int_reset(port_tty_installed);
        int_reset(drv_module_ref_cnt);
        usb_serial_put();
        port_work_stop = 1;
        port_work_initialized = 0;
        int_assume_not(port_work);
        unlock_tty();
    } else {
        int_assume_not(port_tty_installed);
    }
    ;
    usb_serial_put();
}


void usb_serial_device_probe() {
    int x;
    x = port_initialized;
    x = dev_usb_serial_initialized;
    dev_autopm++;
    int_notify(port_tty_registered);
    dev_autopm--;
}


void usb_serial_device_remove() {
    int x;
    x = port_initialized;
    x = dev_usb_serial_initialized;
    dev_autopm++;
    int_reset(port_tty_registered);
    dev_autopm--;
}


void usb_serial_put() {
    int old;
    int x;
    old = dev_usb_serial_initialized;
    dev_usb_serial_initialized--;
    if (old == 1) {
        if (nondet_int) {
            int_assume(port_idr_registered);
            lock_table();
            x = fw_idr_consistent;
            fw_idr_consistent = 0;
            int_reset(port_idr_registered);
            fw_idr_consistent = 1;
            unlock_table();
        }
        lock_serial_bus();
        int_reset(port_dev_registered);
        unlock_serial_bus();
        int_assume_not(port_tty_registered);
        dev_usb_serial_initialized = -1;
        port_initialized = 0;
        int_reset(drv_module_ref_cnt);
    }
}


void try_module_get() {
    int_notify(drv_module_ref_cnt);
}


void belkin_init() {
    int x;
    lock_table();
    drv_usb_initialized = 1;
    int_notify(drv_usb_registered);
    {
        x = fw_driver_list_consistent;
        fw_driver_list_consistent = 0;
        drv_registered_with_usb_fw = 1;
        fw_driver_list_consistent = 1;
        int_notify(drv_registered_with_serial_fw);
        unlock_table();
    }
    drv_id_table = 1;
}


void belkin_exit() {
    int x;
    x = drv_usb_initialized;
    drv_registered_with_usb_fw = 0;
    {
        lock_table();
        x = fw_driver_list_consistent;
        fw_driver_list_consistent = 0;
        drv_registered_with_usb_fw = 0;
        fw_driver_list_consistent = 1;
        int_reset(drv_registered_with_serial_fw);
        unlock_table();
    }
    int_reset(drv_usb_registered);
    drv_usb_initialized = 0;
}


void serial_install() {
    int x;
    lock_table();
    if (nondet_int) {
        int_assume_not(port_idr_registered);
        unlock_table();
        return;
    } else {
        int_assume(port_idr_registered);
        lock_disc();
        if (nondet_int) {
            int_assume(dev_disconnected);
            unlock_disc();
            unlock_table();
            return;
        } else {
            int_assume_not(dev_disconnected);
            x = port_initialized;
            x = dev_usb_serial_initialized;
            dev_usb_serial_initialized++;
            unlock_table();
            try_module_get();
            if (nondet_int) {
                int_assume_not(drv_module_ref_cnt);
                usb_serial_put();
                unlock_disc();
                return;
            } else {
                int_assume(drv_module_ref_cnt);
                dev_autopm++;
                port_tty_state = 1;
                int_notify(port_tty_installed);
                unlock_disc();
            }
        }
    }
}


void serial_hangup() {
}


void serial_write() {
    int x;
    x = port_initialized;
    x = dev_usb_serial_initialized;
    x = port_tty_state;
    lock_port();
    x = port_consistent;
    port_consistent = 0;
    port_consistent = 1;
    unlock_port();
    if (nondet_int) {
        int_assume(write_urb_submitted);
        return;
    } else {
        int_assume_not(write_urb_submitted);
        int_notify(write_urb_submitted);
    }
}


void serial_write_callback() {
    int x;
    x = port_initialized;
    lock_port();
    x = port_consistent;
    port_consistent = 0;
    port_consistent = 1;
    unlock_port();
    x = port_work_initialized;
    int_notify(port_work);
}


void thread_fw_module() {
    usb_serial_init();
    belkin_init();
}


void thread_usb_bus() {
    int_assume(drv_usb_registered);
    int_yield();
    usb_serial_probe();
    int_yield();
    if (nondet_int) {
        int_assume(port_dev_registered);
        usb_serial_disconnect();
    } else {
        int_assume_not(port_dev_registered);
    }
}


void thread_usb_cb() {
    int x;
    int_assume(write_urb_submitted);
    x = drv_usb_initialized;
    int_reset(write_urb_submitted);
    serial_write_callback();
}


void thread_port_work() {
    int x;
    int_assume(port_work);
    x = port_initialized;
    x = port_tty_state;
    int_reset(port_write_in_progress);
    int_reset(port_work);
}


void thread_serial_bus() {
    lock_serial_bus();
    int_assume(port_dev_registered);
    usb_serial_device_probe();
    unlock_serial_bus();
    int_assume_not(port_dev_registered);
    lock_serial_bus();
    usb_serial_device_remove();
    unlock_serial_bus();
}


void thread_tty() {
    int x;
    int_assume(drv_registered_with_serial_fw);
    serial_install();
    int_yield();
    lock_tty();
    if (nondet_int) {
        int_assume(port_tty_installed);
        int_notify(port_write_in_progress);
        x = port_initialized;
        x = dev_usb_serial_initialized;
        x = port_tty_state;
        lock_port();
        x = port_consistent;
        port_consistent = 0;
        port_consistent = 1;
        unlock_port();
        if (nondet_int) {
            int_assume_not(write_urb_submitted);
            int_notify(write_urb_submitted);
        } else {
            int_assume(write_urb_submitted);
        }
    } else {
        int_assume_not(port_tty_installed);
    }
    unlock_tty();
}


void thread_attribute() {
    try_module_get();
    if (nondet_int) {
        int_assume_not(drv_module_ref_cnt);
        return;
    } else {
        int_assume(drv_module_ref_cnt);
        int_assume(drv_registered_with_serial_fw);
        drv_device_id_registered = 1;
        int_reset(drv_module_ref_cnt);
    }
}


