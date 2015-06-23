#include "langinc.h"

/* framework variables */

lock_t synthlock_0;
int fw_tty_registered;
int fw_tty_initialized;
lock_t fw_tty_lock;
int fw_driver_list_consistent;
int fw_idr_consistent;
lock_t fw_table_lock;
lock_t fw_serial_bus_lock;

/* belkin driver variables */

conditional_t drv_usb_registered;
int drv_usb_initialized;

int drv_registered_with_usb_fw;
conditional_t drv_registered_with_serial_fw;

int drv_id_table;
conditional_t drv_module_ref_cnt;
int drv_device_id_registered;

/* belkin per-device variables */
int dev_usb_serial_initialized;
int dev_autopm;
conditional_t dev_disconnected;
lock_t dev_disc_lock;

/* belkin per-port variables */

conditional_t port_dev_registered;
int port_initialized;

// Language inclusion: had to change this var to conditional_t to avoid a spurious counterexample
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

/* URB variables */
conditional_t write_urb_submitted;


void try_module_get();
void usb_serial_put ();

/*
 * Framework functions
 */
void usb_serial_init () {
    fw_tty_initialized = 1;
    fw_tty_registered = 1;
}


void usb_serial_exit () {
    
    fw_tty_registered = 0;
    
    //put_tty_driver(usb_serial_tty_driver);
    wait_not (port_tty_installed);
    fw_tty_initialized = 0;
}

void lock_table () {
    lock(fw_table_lock);
}

void unlock_table () {
    unlock(fw_table_lock);
}

void lock_serial_bus () {
    lock(fw_serial_bus_lock);
}

void unlock_serial_bus () {
    unlock(fw_serial_bus_lock);
}

void lock_tty () {
    lock(fw_tty_lock);
}

void unlock_tty () {
    unlock(fw_tty_lock);
}

void lock_port () {
    lock (port_lock);
}

void unlock_port () {
    unlock(port_lock);
}

void lock_disc () {
    lock (dev_disc_lock);
}

void unlock_disc () {
    unlock(dev_disc_lock);
}

void usb_serial_probe () {
    int x;
    lock_table();
    
    x = fw_driver_list_consistent;
    if (/*(!drv_id_table) |*/ (!drv_registered_with_usb_fw)) {
        unlock_table();
        return;
    } else {

    try_module_get ();
    
    if (/*drv_module_ref_cnt <= 0*/nondet) {
        assume_not(drv_module_ref_cnt);
        unlock_table ();
        return;
    } else {
        assume(drv_module_ref_cnt);

    //assert (drv_usb_initialized);
    x = drv_usb_initialized;
    //assert (drv_usb_registered);
   
    unlock_table();
    
    dev_usb_serial_initialized = 1;
    //belkin_probe ();
    
    
    //belkin_attach();
    
    /* Avoid race with tty_open and serial_install by setting the
     * disconnected flag and not clearing it until all ports have been
     * registered.
     */
    notify(dev_disconnected);
    
    // allocate_minors ()
    {
        lock_table();
        x = fw_idr_consistent;
        fw_idr_consistent = 0;
        notify(port_idr_registered);
        fw_idr_consistent = 1;
        unlock_table();
    }
    
    port_work_initialized = 1;
    port_initialized = 1;
    notify(port_dev_registered);
    reset(dev_disconnected);
    
    // port_dev_registered = 1 increments module counter
    // module_put();
    }
    }
}


void usb_serial_disconnect () {
    int x;
    lock_disc();
    /* must set a flag, to signal subdrivers */
    notify(dev_disconnected);
    unlock_disc();
    
    //assert (dev_usb_serial_initialized>=0);
    x = dev_usb_serial_initialized;
    x = port_initialized;
    
    if (nondet) {
        assume(port_tty_installed);
        
        assume_not (port_write_in_progress);
        //serial_hangup ();
        //serial_close ();
        lock_tty ();
        //assert (drv_module_ref_cnt > 0);
        //x = drv_module_ref_cnt;
        x = dev_usb_serial_initialized;
        x = port_initialized;

        port_tty_state = 0;
        reset(port_tty_installed);

        //drv_module_ref_cnt--;
        reset(drv_module_ref_cnt);
        usb_serial_put();
        // end: serial_close

        unlock_tty ();
        assume_not(port_work);
        //usb_serial_port_poison_urbs();
        //wake_up_interruptible(&port->port.delta_msr_wait);

        port_work_stop = 1;
        port_work_initialized = 0;

        //if (port_dev_registered) 
        //    port_dev_registered = 0;
    } else {
        assume_not(port_tty_installed);
    };
    
    //belkin_disconnect ();
    
    /* let the last holder of this object cause it to be cleaned up */
    usb_serial_put();
}

void usb_serial_device_probe () {
    int x;
    x = port_initialized;
    //assert (dev_usb_serial_initialized>=0);
    x = dev_usb_serial_initialized;
    dev_autopm++;
    
    //belkin_port_probe ();
    
    notify(port_tty_registered);
    
    dev_autopm--;
}

void usb_serial_device_remove () {
    int x;
    lock_s(synthlock_0);
    x = port_initialized;
    x = dev_usb_serial_initialized;
    //assert (dev_usb_serial_initialized>=0);

    
    /* make sure suspend/resume doesn't race against port_remove */
    unlock_s(synthlock_0);
    dev_autopm++;
    
    reset(port_tty_registered);
    
    //belkin_port_remove();
    
    dev_autopm--;
}

void usb_serial_put () {
    int old;
    int x;
    
    //assert (dev_usb_serial_initialized > 0);
    
    //atomicBegin();
    old = dev_usb_serial_initialized;
    dev_usb_serial_initialized--;
    //atomicEnd();
    
    if (old == 1) {
        //release_minors(serial);
        if (nondet)
        {
            assume (port_idr_registered);
            lock_table();
            x = fw_idr_consistent;
            fw_idr_consistent = 0;
            reset(port_idr_registered);
            fw_idr_consistent = 1;
            unlock_table(); 
        }
        
        //belkin_release ();
        
        /* Now that nothing is using the ports, they can be freed */
        lock_serial_bus();
        lock_s(synthlock_0);
        reset(port_dev_registered);
        unlock_serial_bus();
        assume_not (port_tty_registered);
        dev_usb_serial_initialized = -1;
        unlock_s(synthlock_0);
        port_initialized = 0;
        reset(drv_module_ref_cnt);
        //drv_module_ref_cnt--;
    }
}

/*
 * Driver functions
 */

void try_module_get() {
    //atomicBegin();
//    if (drv_module_ref_cnt >= 0) 
//        drv_module_ref_cnt++;
    notify(drv_module_ref_cnt);
    //atomicEnd();
}

void belkin_init () {
    int x;
    /*
     * udriver must be registered before any of the serial drivers,
     * because the store_new_id() routine for the serial drivers (in
     * bus.c) probes udriver.
     *
     * Performance hack: We don't want udriver to be probed until
     * the serial drivers are registered, because the probe would
     * simply fail for lack of a matching serial driver.
     * So we leave udriver's id_table set to NULL until we are all set.
     *
     * Suspend/resume support is implemented in the usb-serial core,
     * so fill in the PM-related fields in udriver.
     */
    // TODO: implement new_id_store

    lock_table();
    drv_usb_initialized = 1;
    notify(drv_usb_registered);

    // usb_serial_register()
    {
        x = fw_driver_list_consistent;
        fw_driver_list_consistent = 0;
        drv_registered_with_usb_fw = 1;
        fw_driver_list_consistent = 1;
        
        notify(drv_registered_with_serial_fw);
        unlock_table();
    }
 
    drv_id_table = 1;
}

void belkin_exit () {
    int x;
    //assert (drv_usb_initialized);
    x = drv_usb_initialized;
    drv_registered_with_usb_fw = 0;
    
    //usb_serial_deregister
    {
        lock_table ();
        
        x = fw_driver_list_consistent;
        fw_driver_list_consistent = 0;
        drv_registered_with_usb_fw = 0;
        fw_driver_list_consistent = 1;
        
        reset(drv_registered_with_serial_fw);
        unlock_table();
    }
    
    reset(drv_usb_registered);
    drv_usb_initialized = 0;
}

/*
 * Per-port driver functions
 */

void serial_install () {
    int x;

    // port = usb_serial_port_get_by_minor(idx);

    lock_table();
    if (nondet) {
        assume_not(port_idr_registered);
        unlock_table ();
        return;
    } else {
        assume(port_idr_registered);
        lock_disc ();
        if (nondet) {
            assume (dev_disconnected);
            unlock_disc ();
            unlock_table ();
            return;
        } else {
            assume_not(dev_disconnected);
            x = port_initialized;
            //assert (dev_usb_serial_initialized > 0);
            x = dev_usb_serial_initialized;
            dev_usb_serial_initialized++;
            unlock_table ();
            try_module_get ();
            if (/*drv_module_ref_cnt <= 0*/nondet) {
                assume_not(drv_module_ref_cnt);
                usb_serial_put ();
                unlock_disc ();
                return;
            } else {
                assume(drv_module_ref_cnt);
                dev_autopm++;
                port_tty_state=1;
                notify(port_tty_installed);
                unlock_disc ();
            }
        }
    }
}

void serial_hangup() {
    // TODO
    // takes tty lock
}

//void serial_close () {
//    assert (drv_module_ref_cnt > 0);
//    assert (dev_usb_serial_initialized);
//    assert (port_initialized);
//    assume (port_write_in_progress == 0);
//    port_tty_installed = 0;
//
//    drv_module_ref_cnt--;
//    usb_serial_put();
//}

void serial_write () {
    int x;
    x = port_initialized;
    x = dev_usb_serial_initialized;
    x = port_tty_state;
    lock_port ();
    x = port_consistent;
    port_consistent = 0;
    port_consistent = 1;
    unlock_port ();

    if (nondet){ 
        assume(write_urb_submitted);
        return;
    } else {
        assume_not(write_urb_submitted);
        notify(write_urb_submitted);
    }
    // TODO model concurrent operations on the port
}

void serial_write_callback () {
    int x;
    x = port_initialized;
    lock_port ();
    x = port_consistent;
    port_consistent = 0;
    port_consistent = 1;
    unlock_port ();
    x = port_work_initialized;
    notify(port_work);
}

/*
 * Threads
 */
void thread_fw_module () {
    usb_serial_init();
    
    belkin_init();

// Language inclusion: this requires reference counting    
//    atomicBegin();
//    assume(drv_module_ref_cnt == 0);
//    drv_module_ref_cnt--;
//    atomicEnd();
//    
//    belkin_exit();
//    
//    usb_serial_exit();
    // TODO: everything must be uninitialised
    
}

void thread_usb_bus () {
    assume (drv_usb_registered /*| drv_device_id_registered*/);
    yield();
    usb_serial_probe ();
    yield();
    // TODO
    
    // hack to avoid checking return value of usb_serial_probe
    if (nondet) {
        assume(port_dev_registered);
        usb_serial_disconnect ();
    } else {
        assume_not(port_dev_registered);
    }
}

void thread_usb_cb () {
    int x;
    //while (drv_usb_registered != 0) {
        assume (write_urb_submitted/* | (drv_usb_registered == 0)*/);
        //assert (drv_usb_initialized);
        x = drv_usb_initialized;
        reset(write_urb_submitted);
        serial_write_callback();
    //}
}

void thread_port_work () {
    int x;
    //while (port_work_active != 0) {
        assume (port_work /*| (port_work_stop == 1)*/);
        //x = port_initialized;
        //x = port_tty_state;
        reset(port_write_in_progress);
        reset(port_work);
    //};
}

void thread_serial_bus () {
    lock_serial_bus();
    assume (port_dev_registered);
    usb_serial_device_probe ();
    unlock_serial_bus();
    lock_s(synthlock_0);
    
    assume_not (port_dev_registered);
    unlock_s(synthlock_0);
    lock_serial_bus();
    usb_serial_device_remove ();
    unlock_serial_bus();
}

void thread_tty () {
    int x;
    assume (drv_registered_with_serial_fw);
    serial_install ();
    yield();
    //while (port_tty_installed != 0) {
        lock_tty ();
        if (nondet) {
            assume(port_tty_installed);
            notify(port_write_in_progress);
            //serial_write();
            x = port_initialized;
            x = dev_usb_serial_initialized;
            x = port_tty_state;
            lock_port ();
            x = port_consistent;
            port_consistent = 0;
            port_consistent = 1;
            unlock_port ();

            if (nondet) {
                assume_not(write_urb_submitted);
                notify(write_urb_submitted);
            } else {
                assume(write_urb_submitted);
            }
            // end serial_write
        } else {
            assume_not(port_tty_installed);
        }
        unlock_tty ();
    //}
    // TODO
}

void thread_attribute () {
    try_module_get();

    if (/*drv_module_ref_cnt <= 0*/nondet) {
        assume_not(drv_module_ref_cnt);
        return;
    } else {
        assume(drv_module_ref_cnt);
        assume (drv_registered_with_serial_fw);
        drv_device_id_registered = 1;
        //drv_module_ref_cnt--;
        reset(drv_module_ref_cnt);
    }
}

//void main () {
//    /* framework variables */
//    
//    fw_tty_registered = 0;
//    fw_tty_initialized = 0;
//    fw_tty_lock = 0;
//    fw_driver_list_consistent = 1;
//    fw_idr_consistent = 1;
//    fw_table_lock = 0;
//    fw_serial_bus_lock = 0;
//    
//    /* belkin driver variables */
//    
//    drv_usb_registered = 0;
//    drv_usb_initialized = 0;
//    
//    drv_registered_with_usb_fw = 0;
//    drv_registered_with_serial_fw = 0;
//    
//    drv_id_table = 0;
//    drv_module_ref_cnt = 0;
//    drv_device_id_registered = 0;
//    
//    /* belkin per-device variables */
//    dev_usb_serial_initialized = -1;
//    dev_autopm = 0;
//    dev_disconnected = 0;
//    dev_disc_lock = 0;
//    
//    /* belkin per-port variables */
//    
//    port_dev_registered = 0;
//    port_initialized = 0;
//    port_tty_registered = 0;
//    port_tty_installed = 0;
//    port_idr_registered = 0;
//    port_work = 0;
//    port_work_initialized = 0;
//    port_work_stop = 0;
//    port_consistent = 1;
//    port_lock = 0;
//    port_write_in_progress = 0;
//
//    write_urb_submitted = URB_IDLE;
//    
//    /*******************/
//    fw_module_thread ();
//    usb_bus_thread ();
//    serial_bus_thread ();
//    tty_thread ();
//    attribute_thread (); 
//    usb_cb_thread ();
//    port_work_thread ();
//}
