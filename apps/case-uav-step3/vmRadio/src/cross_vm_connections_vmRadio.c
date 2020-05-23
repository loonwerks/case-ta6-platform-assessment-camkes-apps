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
//
//    dataport queue_t operating_region_out_crossvm_dp;
//    emits SendEvent operating_region_out_ready;
//
//    dataport queue_t line_search_task_out_crossvm_dp;
//    emits SendEvent line_search_task_out_ready;
//
//    dataport queue_t automation_request_out_crossvm_dp;
//    emits SendEvent automation_request_out_ready;
//
//    dataport queue_t uxas_log_in_crossvm_dp;
//    maybe consumes SendEvent uxas_log_in_done;


#define NUM_CONNECTIONS 4
static struct camkes_crossvm_connection connections[NUM_CONNECTIONS];

// these are defined in the dataport's glue code
extern dataport_caps_handle_t operating_region_out_crossvm_dp_handle;
void operating_region_out_ready_emit_underlying(void); 

extern dataport_caps_handle_t line_search_task_out_crossvm_dp_handle;
void line_search_task_out_ready_emit_underlying(void); 

extern dataport_caps_handle_t automation_request_out_crossvm_dp_handle;
void automation_request_out_ready_emit_underlying(void); 

extern dataport_caps_handle_t uxas_log_in_crossvm_dp_handle;
seL4_Word uxas_log_in_done_notification_badge(void);


static int consume_callback(vm_t *vm, void *cookie)
{
    consume_connection_event(vm, (seL4_Word) cookie, true);
    return 0;
}


void init_cross_vm_connections(vm_t *vm, void *cookie)
{
    connections[0] = (struct camkes_crossvm_connection) {
        .handle = &operating_region_out_crossvm_dp_handle,
        .emit_fn = operating_region_out_ready_emit_underlying,
        .consume_badge = -1
    };

    connections[1] = (struct camkes_crossvm_connection) {
        .handle = &line_search_task_out_crossvm_dp_handle,
        .emit_fn = line_search_task_out_ready_emit_underlying,
        .consume_badge = -1
    };

    connections[2] = (struct camkes_crossvm_connection) {
        .handle = &automation_request_out_crossvm_dp_handle,
        .emit_fn = automation_request_out_ready_emit_underlying,
        .consume_badge = -1
    };

    connections[3] = (struct camkes_crossvm_connection) {
        .handle = &uxas_log_in_crossvm_dp_handle,
        .emit_fn = NULL,
        .consume_badge = uxas_log_in_done_notification_badge()
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
