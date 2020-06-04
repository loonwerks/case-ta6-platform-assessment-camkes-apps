/*
 * Copyright 2020, Collins Aerospace
 */

#include <camkes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <am_counter.h>
#include <am_data.h>
#include <am_queue.h>

#include <stdint.h>
#include <sys/types.h>

#include "hexdump.h"

// Forward declarations
// void operating_region_out_event_data_send(data_t *data);
// void line_search_task_out_event_data_send(data_t *data);
// void automation_request_out_event_data_send(data_t *data);



//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "p1_in".
void trusted_ids_in_event_data_receive(counter_t numDropped, data_t *data) {
    printf("%s received trusted id list:", get_instance_name());
    for(int i = 0; i < 3; i++) {
        printf(" ");
        for(int j = 0; j < 4; j++)
            printf("%02X", data->payload[4*i+j]);
    }
    printf("; numDropped: %" PRIcounter "\n", numDropped);
    // printf("%s: received trusted ids: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped);
    // hexdump("    ", 32, data->payload, sizeof(data->payload));
}

recv_queue_t trustedIdsInRecvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool trusted_ids_in_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&trustedIdsInRecvQueue, numDropped, data);
}


//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "p1_in".
// void operating_region_in_event_data_receive(counter_t numDropped, data_t *data) {
//     printf("%s: received operating region: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped);
//     // hexdump("    ", 32, data->payload, sizeof(data->payload));
//     operating_region_out_event_data_send(data);
// }
//
// recv_queue_t operatingRegionInRecvQueue;
//
// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
// bool operating_region_in_event_data_poll(counter_t *numDropped, data_t *data) {
//     return queue_dequeue(&operatingRegionInRecvQueue, numDropped, data);
// }


//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
// void line_search_task_in_event_data_receive(counter_t numDropped, data_t *data) {
//     printf("%s: received line search task: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped);
//     // hexdump("    ", 32, data->payload, sizeof(data->payload));
//     line_search_task_out_event_data_send(data);
// }
//
//
// recv_queue_t lineSearchTaskInRecvQueue;
//
// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
// bool line_search_task_in_event_data_poll(counter_t *numDropped, data_t *data) {
//     return queue_dequeue(&lineSearchTaskInRecvQueue, numDropped, data);
// }


//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
// void automation_request_in_event_data_receive(counter_t numDropped, data_t *data) {
//     printf("%s: received automation request: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped);
//     // hexdump("    ", 32, data->payload, sizeof(data->payload));
//     automation_request_out_event_data_send(data);
// }
//
//
// recv_queue_t automationRequestInRecvQueue;
//
// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
// bool automation_request_in_event_data_poll(counter_t *numDropped, data_t *data) {
//     return queue_dequeue(&automationRequestInRecvQueue, numDropped, data);
// }


void done_emit_underlying(void) WEAK;
static void done_emit(void) {
  /* If the interface is not connected, the 'underlying' function will
   * not exist.
   */
  if (done_emit_underlying) {
    done_emit_underlying();
  }
}



// void operating_region_out_event_data_send(data_t *data) {
//     queue_enqueue(operating_region_out_queue, data);
//     operating_region_out_SendEvent_emit();
//     done_emit();
// }

// void line_search_task_out_event_data_send(data_t *data) {
//     queue_enqueue(line_search_task_out_queue, data);
//     line_search_task_out_SendEvent_emit();
//     done_emit();
// }

// void automation_request_out_event_data_send(data_t *data) {
//     queue_enqueue(automation_request_out_queue, data);
//     automation_request_out_SendEvent_emit();
//     done_emit();
// }


void run_poll(void) {
    counter_t numDropped;
    data_t data;

    while (true) {

        bool dataReceived = trusted_ids_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            trusted_ids_in_event_data_receive(numDropped, &data);
        }

        // bool dataReceived = operating_region_in_event_data_poll(&numDropped, &data);
        // if (dataReceived) {
        //     operating_region_in_event_data_receive(numDropped, &data);
        // }

        // dataReceived = line_search_task_in_event_data_poll(&numDropped, &data);
        // if (dataReceived) {
        //     line_search_task_in_event_data_receive(numDropped, &data);
        // }

        // dataReceived = automation_request_in_event_data_poll(&numDropped, &data);
        // if (dataReceived) {
        //     automation_request_in_event_data_receive(numDropped, &data);
        // }

        seL4_Yield();
    }

}


void post_init(void) {
    recv_queue_init(&trustedIdsInRecvQueue, trusted_ids_in_queue);
    // recv_queue_init(&operatingRegionInRecvQueue, operating_region_in_queue);
    // recv_queue_init(&lineSearchTaskInRecvQueue, line_search_task_in_queue);
    // recv_queue_init(&automationRequestInRecvQueue, automation_request_in_queue);
    // queue_init(operating_region_out_queue);
    // queue_init(line_search_task_out_queue);
    // queue_init(automation_request_out_queue);
}

int run(void) {

    run_poll();
}
