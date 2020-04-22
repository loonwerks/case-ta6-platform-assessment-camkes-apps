/*
 * Copyright 2020, Collins Aerospace.
 */

#include <autoconf.h>
#include <camkes.h>
#include <vmlinux.h>
#include <sel4vm/guest_vm.h>

#include <sel4vmmplatsupport/drivers/cross_vm_connection.h>
#include <sel4vmmplatsupport/drivers/pci_helper.h>
#include <pci/helper.h>

#ifdef CONFIG_PLAT_QEMU_ARM_VIRT
#define CONNECTION_BASE_ADDRESS 0xDF000000
#else
#define CONNECTION_BASE_ADDRESS 0x3F000000
#endif

// The following device definitions correspond to the following camkes interfaces.

// dataport queue_t crossvm_dp_wpm;
// emits SendEvent wpm_ready;

// dataport queue_t crossvm_dp_apss;
// consumes SendEvent apss_done;


#define NUM_CONNECTIONS 2
static struct camkes_crossvm_connection connections[NUM_CONNECTIONS];

// these are defined in the dataport's glue code
extern dataport_caps_handle_t crossvm_dp_wpm_handle;
void wpm_ready_emit_underlying(void); 

extern dataport_caps_handle_t crossvm_dp_apss_handle;
seL4_Word apss_done_notification_badge(void);


static int consume_callback(vm_t *vm, void *cookie)
{
    consume_connection_event(vm, (seL4_Word) cookie, true);
    return 0;
}


void init_cross_vm_connections(vm_t *vm, void *cookie)
{
    connections[0] = (struct camkes_crossvm_connection) {
        .handle = &crossvm_dp_wpm_handle,
        .emit_fn = wpm_ready_emit_underlying,
        .consume_badge = -1
    };

    connections[1] = (struct camkes_crossvm_connection) {
        .handle = &crossvm_dp_apss_handle,
        .emit_fn = NULL,
        .consume_badge = apss_done_notification_badge()
    };

    for (int i = 0; i < NUM_CONNECTIONS; i++) {
        if (connections[i].consume_badge != -1) {
            int err = register_async_event_handler(connections[i].consume_badge, consume_callback, (void *)connections[i].consume_badge);
            ZF_LOGF_IF(err, "Failed to register_async_event_handler for init_cross_vm_connections.");

        }   
    }


    cross_vm_connections_init(vm, CONNECTION_BASE_ADDRESS, connections, ARRAY_SIZE(connections));
}

DEFINE_MODULE(cross_vm_connections, NULL, init_cross_vm_connections)
