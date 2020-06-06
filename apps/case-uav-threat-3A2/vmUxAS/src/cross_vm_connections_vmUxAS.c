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
//    dataport queue_t operating_region_in_crossvm_dp;
//    maybe consumes SendEvent operating_region_in_done;
//
//    dataport queue_t line_search_task_in_crossvm_dp;
//    maybe consumes SendEvent line_search_task_in_done;
//
//    dataport queue_t automation_request_in_crossvm_dp;
//    maybe consumes SendEvent automation_request_in_done;
//
//    dataport queue_t automation_response_out_crossvm_dp;
//    emits SendEvent wpm_ready;
//
//    dataport queue_t crossvm_dp_apss;
//    consumes SendEvent apss_done;
//
//    dataport queue_t uxas_log_out_crossvm_dp;
//    emits SendEvent uxas_log_out_ready;


#define NUM_CONNECTIONS 7
static struct camkes_crossvm_connection connections[NUM_CONNECTIONS];

// these are defined in the dataport's glue code
extern dataport_caps_handle_t operating_region_in_crossvm_dp_handle;
seL4_Word operating_region_in_done_notification_badge(void);

extern dataport_caps_handle_t line_search_task_in_crossvm_dp_handle;
seL4_Word line_search_task_in_done_notification_badge(void);

extern dataport_caps_handle_t automation_request_in_crossvm_dp_handle;
seL4_Word automation_request_in_done_notification_badge(void);

extern dataport_caps_handle_t automation_response_out_1_crossvm_dp_handle;
void automation_response_out_1_ready_emit_underlying(void); 

extern dataport_caps_handle_t automation_response_out_2_crossvm_dp_handle;
void automation_response_out_2_ready_emit_underlying(void); 

// extern dataport_caps_handle_t automation_response_out_3_crossvm_dp_handle;
// void automation_response_out_3_ready_emit_underlying(void); 
 
extern dataport_caps_handle_t air_vehicle_state_in_crossvm_dp_handle;
seL4_Word air_vehicle_state_in_done_notification_badge(void);

extern dataport_caps_handle_t uxas_log_out_crossvm_dp_handle;
void uxas_log_out_ready_emit_underlying(void); 


static int consume_callback(vm_t *vm, void *cookie)
{
    consume_connection_event(vm, (seL4_Word) cookie, true);
    return 0;
}


void init_cross_vm_connections(vm_t *vm, void *cookie)
{
    connections[0] = (struct camkes_crossvm_connection) {
        .handle = &operating_region_in_crossvm_dp_handle,
        .emit_fn = NULL,
        .consume_badge = operating_region_in_done_notification_badge()
    };

    connections[1] = (struct camkes_crossvm_connection) {
        .handle = &line_search_task_in_crossvm_dp_handle,
        .emit_fn = NULL,
        .consume_badge = line_search_task_in_done_notification_badge()
    };

    connections[2] = (struct camkes_crossvm_connection) {
        .handle = &automation_request_in_crossvm_dp_handle,
        .emit_fn = NULL,
        .consume_badge = automation_request_in_done_notification_badge()
    };

    connections[3] = (struct camkes_crossvm_connection) {
        .handle = &automation_response_out_1_crossvm_dp_handle,
        .emit_fn = automation_response_out_1_ready_emit_underlying,
        .consume_badge = -1
    };

    connections[4] = (struct camkes_crossvm_connection) {
        .handle = &automation_response_out_2_crossvm_dp_handle,
        .emit_fn = automation_response_out_2_ready_emit_underlying,
        .consume_badge = -1
    };

//     connections[5] = (struct camkes_crossvm_connection) {
//         .handle = &automation_response_out_3_crossvm_dp_handle,
//         .emit_fn = automation_response_out_3_ready_emit_underlying,
//         .consume_badge = -1
//     };
 
    connections[5] = (struct camkes_crossvm_connection) {
        .handle = &air_vehicle_state_in_crossvm_dp_handle,
        .emit_fn = NULL,
        .consume_badge = air_vehicle_state_in_done_notification_badge()
    };

    connections[6] = (struct camkes_crossvm_connection) {
        .handle = &uxas_log_out_crossvm_dp_handle,
        .emit_fn = uxas_log_out_ready_emit_underlying,
        .consume_badge = -1
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
