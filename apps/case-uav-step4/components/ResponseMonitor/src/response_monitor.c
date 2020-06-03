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


//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
void automation_request_in_event_data_receive(counter_t numDropped, data_t *data) {
    printf("%s: received automation request: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped); fflush(stdout);
    // hexdump("    ", 32, data->payload, sizeof(data->payload));
    
}


recv_queue_t automationRequestInRecvQueue;

// Assumption: only one thread is calling this and/or reading p1_in_recv_counter.
bool automation_request_in_event_data_poll(counter_t *numDropped, data_t *data) {
    return queue_dequeue(&automationRequestInRecvQueue, numDropped, data);
}



//------------------------------------------------------------------------------
// User specified input data receive handler for AADL Input Event Data Port (in) named
// "automation_response_in".
void automation_response_in_event_data_receive(counter_t numDropped, data_t *data) {
    printf("%s: received automation response: numDropped: %" PRIcounter "\n", get_instance_name(), numDropped); fflush(stdout);
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



void run_poll(void) {
    counter_t numDropped;
    data_t data;
    uint32_t invocations = 0;
    uint32_t total_invocations = 0;

    while (true) {

        if (invocations > 0) {
            invocations++;
        }

        bool dataReceived = automation_request_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            automation_request_in_event_data_receive(numDropped, &data);
            invocations = 1;
        }

        dataReceived = automation_response_in_event_data_poll(&numDropped, &data);
        if (dataReceived) {
            automation_response_in_event_data_receive(numDropped, &data);
            total_invocations += invocations;
            printf("total invocations = %u\n", total_invocations);
            invocations = 0;
        }

        if (invocations > 120000) {
            printf("\n************************************\n");
            printf("** Response Monitor:              **\n");
            printf("** Expected a response from UxAS, **\n");
            printf("** but did not receive one!       **\n");
            printf("** Consider aborting mission.     **\n");
            printf("************************************\n\n");
            fflush(stdout);
            total_invocations += invocations;
            invocations = 0;
        }

        seL4_Yield();
    }

}


void post_init(void) {
    recv_queue_init(&automationRequestInRecvQueue, automation_request_in_queue);
    recv_queue_init(&automationResponseInRecvQueue, automation_response_in_queue);
}

int run(void) {

    run_poll();

}

