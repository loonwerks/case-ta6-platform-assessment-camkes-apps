/*
 * Copyright 2020, Collins Aerospace
 */

#include <camkes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <counter.h>
#include <data.h>
#include <queue.h>

#include <stdint.h>
#include <sys/types.h>

#include "hexdump.h"

// Forward declarations
void alert_out_event_data_send(data_t *data);




//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
void automation_response_in_event_data_receive(counter_t numDropped, data_t *data) {
    printf("%s: received automation response: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped);
    // hexdump("    ", 32, data->payload, sizeof(data->payload));
    
}


recv_queue_t automationResponseInRecvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool automation_response_in_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&automationResponseInRecvQueue, numDropped, data);
}



void done_emit_underlying(void) WEAK;
static void done_emit(void) {
  /* If the interface is not connected, the 'underlying' function will
   * not exist.
   */
  if (done_emit_underlying) {
    done_emit_underlying();
  }
}



void alert_out_event_data_send(data_t *data) {
    queue_enqueue(alert_out_queue, data);
    alert_out_SendEvent_emit();
    done_emit();
}


void initializeFlyZones() {


}


void run_poll(void) {
    counter_t numDropped;
    data_t data;

    initializeFlyZones();

    while (true) {

        bool dataReceived = automation_response_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            automation_response_in_event_data_receive(numDropped, &data);
        }

        seL4_Yield();
    }

}


void post_init(void) {
    recv_queue_init(&automationResponseInRecvQueue, automation_response_in_queue);
    queue_init(alert_out_queue);
}

int run(void) {

    run_poll();

}

